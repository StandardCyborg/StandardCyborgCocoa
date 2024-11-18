//
//  ViewController.mm
//  VisualTesterMac
//
//  Created by Aaron Thompson on 7/12/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <SceneKit/SceneKit.h>
#import <standard_cyborg/sc3d/Geometry.hpp>
#import <standard_cyborg/math/Mat3x4.hpp>
#import <standard_cyborg/math/Mat3x3.hpp>
#import <standard_cyborg/scene_graph/SceneGraph.hpp>
#import <StandardCyborgFusion/GeometryHelpers.hpp>
#import <StandardCyborgFusion/PointCloudIO.hpp>
#import <StandardCyborgFusion/SCOfflineReconstructionManager.h>
#import <StandardCyborgFusion/SCOfflineReconstructionManager_Private.h>
#import <StandardCyborgFusion/SCPointCloud+Geometry.h>
#import <StandardCyborgFusion/StandardCyborgFusion.h>
#import <StandardCyborgFusion/Surfel.hpp>
#import <standard_cyborg/io/gltf/SceneGraphFileIO_GLTF.hpp>
#import <stdlib.h>
#import <fstream>

#import "AppDelegate.h"
#import "CameraControl.h"
#import "ClearPass.hpp"
#import "DrawAxes.hpp"
#import "DrawCorrespondences.hpp"
#import "DrawPointCloud.hpp"
#import "DrawRawDepths.hpp"
#import "DrawSurfelIndexMap.hpp"
#import "EigenSceneKitHelpers.hpp"
#import "ViewController.h"
#import <json.hpp>

NS_ASSUME_NONNULL_BEGIN

using namespace standard_cyborg;

static SCNVector3 SurfelsBoundingBoxCenter(const Surfels& surfels);

@interface ViewController () <CameraControlDelegate, SCOfflineReconstructionManagerDelegate>
@end

@implementation ViewController
{
    IBOutlet __weak NSButton *assimilateNextButton;
    IBOutlet __weak NSButton *assimilateAllButton;
    IBOutlet __weak NSButton *resetButton;
    IBOutlet __weak NSButton *openDirectoryButton;
    IBOutlet __weak NSTextField *frameIndexField;
    IBOutlet __weak NSTextField *icpDownsampleField;
    IBOutlet __weak NSTextField *pbfMinDepthField;
    IBOutlet __weak NSTextField *pbfMaxDepthField;
    IBOutlet __weak NSButton *drawCorrespondencesCheckbox;
    IBOutlet __weak NSButton *colorByNormalsCheckbox;
    
    NSMutableDictionary *_textureDebugWindowsByTitle;
    MetalVisualizationEngine *_visualizationEngine;
    DrawPointCloud *_drawPointCloud;
    DrawCorrespondences *_drawCorrespondences;
    DrawRawDepths *_drawRawDepths;
    DrawSurfelIndexMap *_drawSurfelIndexMap;
    CameraControl *_cameraControl;
    
    id<MTLDevice> _metalDevice;
    id<MTLCommandQueue> _algorithmCommandQueue;
    id<MTLCommandQueue> _visualizationCommandQueue;
    SCOfflineReconstructionManager *_reconstructionManager;
    
    dispatch_queue_t _processingQueue;
    BOOL _isProcessing;
    NSString *_dataDirectoryPath;
    NSInteger _nextFrameIndex;
    BOOL _assimilatingAll;
    BOOL _hasCenter;
    ICPResult _lastICPResult;
}

// MARK: - IBActions

- (IBAction)openDirectory:(NSButton *)sender {
    NSOpenPanel *openDialog = [NSOpenPanel openPanel];
    [openDialog setCanChooseFiles:NO];
    [openDialog setAllowsMultipleSelection:NO];
    [openDialog setCanChooseDirectories:YES];
    
    if ([openDialog runModal] == NSModalResponseOK) {
        NSArray* dirs = [openDialog URLs];
        NSAssert([dirs count] == 1, @"You must select one directory");
        _dataDirectoryPath = [[dirs objectAtIndex:0] path];
        [[NSUserDefaults standardUserDefaults] setObject:_dataDirectoryPath forKey:@"LastDataPath"];
        [self reset:nil];
    }
}

- (IBAction)assimilateNext:(nullable id)sender
{
    _assimilatingAll = NO;
    [self _assimilateNextFrame];
}

- (IBAction)assimilateAll:(nullable id)sender
{
    if (_assimilatingAll) {
        // This is a pause button
        _assimilatingAll = NO;
    } else {
        _assimilatingAll = YES;
    }
    
    [self _assimilateNextFrame];
}

- (IBAction)reset:(nullable id)sender
{
    [_reconstructionManager reset];
    
    _nextFrameIndex = 0;
    _assimilatingAll = NO;
    _hasCenter = NO;
    _lastICPResult = ICPResult();
    
    [self _loadMotionData];
    
    // Assimilate the first frame
    [self _assimilateNextFrame];
}

- (IBAction)exportUSDA:(id)sender
{
    NSSavePanel *panel = [NSSavePanel savePanel];
    [panel setTitle:@"Export USDA"];
    [panel setNameFieldStringValue:@"Scan.usda"];
    [panel setAllowedFileTypes:@[@"usda"]];
    [panel setExtensionHidden:NO];
    [panel setShowsTagField:NO];
    [panel beginWithCompletionHandler:^(NSModalResponse result) {
        if (result != NSModalResponseOK) { return; }
        
        NSString *USDAPath = [[panel URL] path];
        
        [_reconstructionManager writePointCloudToUSDAFile:USDAPath];
    }];
}

- (IBAction)exportPLY:(id)sender
{
    NSSavePanel *panel = [NSSavePanel savePanel];
    [panel setTitle:@"Export PLY"];
    [panel setNameFieldStringValue:@"Scan.ply"];
    [panel setAllowedFileTypes:@[@"ply"]];
    [panel setExtensionHidden:NO];
    [panel setShowsTagField:NO];
    [panel beginWithCompletionHandler:^(NSModalResponse result) {
        if (result != NSModalResponseOK) { return; }
        
        NSString *PLYPath = [[panel URL] path];
        
        [_reconstructionManager writePointCloudToPLYFile:PLYPath];
    }];
}

- (IBAction)exportPosesToJSON:(id)sender
{
    NSSavePanel *panel = [NSSavePanel savePanel];
    [panel setTitle:@"Export Poses to JSON"];
    [panel setNameFieldStringValue:@"poses.json"];
    [panel setAllowedFileTypes:@[@"json"]];
    [panel setExtensionHidden:NO];
    [panel setShowsTagField:NO];
    [panel beginWithCompletionHandler:^(NSModalResponse result) {
        if (result != NSModalResponseOK) { return; }
        
        NSString *JSONPath = [[panel URL] path];
        
        const std::vector<PBFAssimilatedFrameMetadata> metadata = [_reconstructionManager assimilatedFrameMetadata];
        nlohmann::json poses;
        for (const PBFAssimilatedFrameMetadata& datum : metadata) {
            nlohmann::json pose;
            pose["merged"] = datum.isMerged;
            pose["timestamp"] = datum.timestamp;

            std::vector<std::vector<float>> rows;
            for (int i = 0; i < 3; i++) {
                std::vector<float> row;
                for (int j = 0; j < 4; j++) {
                    row.push_back(datum.viewMatrix(i, j));
                }
                rows.push_back(row);
            }
            
            pose["surfel_count"] = datum.surfelCount;
            pose["extrinsic_matrix"] = rows;
            
            poses["poses"].push_back(pose);
        }
        
        std::string path ([JSONPath UTF8String]);
        std::ofstream poseOutput(path);
        poseOutput << poses.dump(2);
        poseOutput.close();
    }];
}

- (IBAction)exportSceneGraph:(id)sender
{
    NSSavePanel *panel = [NSSavePanel savePanel];
    [panel setTitle:@"Export Scenegraph"];
    [panel setNameFieldStringValue:@"scan.gltf"];
    [panel setAllowedFileTypes:@[@"gltf"]];
    [panel setExtensionHidden:NO];
    [panel setShowsTagField:NO];
    [panel beginWithCompletionHandler:^(NSModalResponse result) {
        if (result != NSModalResponseOK) { return; }
        
        NSString *GLTFPath = [[panel URL] path];
        
        sc3d::Geometry geometry {};
        SCPointCloud* pointCloud = [_reconstructionManager buildPointCloud];
        
        [pointCloud toGeometry:geometry];
        
        using namespace scene_graph;
        std::shared_ptr<Node> rootNode = std::make_shared<Node>("Root");
        std::shared_ptr<GeometryNode> geometryNode(new GeometryNode("Point cloud reconstruction"));
        std::shared_ptr<CoordinateFrameNode> axesNode (new CoordinateFrameNode("axes"));
        geometryNode->getGeometry().copy(geometry);
        rootNode->setTransform(toMat3x4([_reconstructionManager gravityAlignedAxes]).inverse());
        
        rootNode->appendChildren({geometryNode, axesNode});

        io::gltf::WriteSceneGraphToGltf({rootNode}, std::string([GLTFPath UTF8String]));
    }];
}

- (IBAction)uiControlDidChange:(id)sender
{
    [self _updateValuesFromUI];
    [self _updateUI];
    [self _redraw];
}

// MARK: - NSViewController

- (void)viewDidLoad
{
    _processingQueue = dispatch_queue_create("ICP", NULL);
    _textureDebugWindowsByTitle = [[NSMutableDictionary alloc] init];
    
    NSArray<id<MTLDevice>>* allDevices = MTLCopyAllDevices();
    _metalDevice = [allDevices lastObject];
    NSLog(@"GPU: %@", [_metalDevice name]);
    
    _algorithmCommandQueue = [_metalDevice newCommandQueue];
    _algorithmCommandQueue.label = @"ViewController._algorithmCommandQueue";
    
    _visualizationCommandQueue = [_metalDevice newCommandQueue];
    _visualizationCommandQueue.label = @"ViewController._visualizationCommandQueue";
    
    _reconstructionManager = [[SCOfflineReconstructionManager alloc] initWithDevice:_metalDevice
                                                                       commandQueue:_algorithmCommandQueue
                                                                     maxThreadCount:(int)[[NSProcessInfo processInfo] processorCount]];
  
    
    
    _reconstructionManager.delegate = self;
    
    id<MTLLibrary> library = [_metalDevice newDefaultLibrary];
    
    ClearPass *clearPass = [[ClearPass alloc] initWithDevice:_metalDevice library:library];
    _drawPointCloud = [[DrawPointCloud alloc] initWithDevice:_metalDevice library:library];
    _drawCorrespondences = [[DrawCorrespondences alloc] initWithDevice:_metalDevice library:library];
    DrawAxes *drawAxes = [[DrawAxes alloc] initWithDevice:_metalDevice library:library];
    NSArray *visualizations = @[clearPass, _drawPointCloud, drawAxes, _drawCorrespondences];
    
    _visualizationEngine = [[MetalVisualizationEngine alloc] initWithDevice:_metalDevice
                                                               commandQueue:_visualizationCommandQueue
                                                                    library:library
                                                             visualizations:visualizations];
    _drawRawDepths = [[DrawRawDepths alloc] initWithDevice:_metalDevice
                                              commandQueue:_visualizationCommandQueue
                                                   library:library];
    _drawSurfelIndexMap = [[DrawSurfelIndexMap alloc] initWithDevice:_metalDevice
                                                        commandQueue:_visualizationCommandQueue
                                                             library:library];
    
    _cameraControl = [[CameraControl alloc] init];
    _cameraControl.delegate = self;
    
    NSString *lastDataPath = [[NSUserDefaults standardUserDefaults] stringForKey:@"LastDataPath"];
    if ([lastDataPath length] > 0) {
        _dataDirectoryPath = lastDataPath;
        NSLog(@"Using previously opened data directory \"%@\"\n", lastDataPath);
        [self reset:nil];
    }
    
    [self _updateUI];
}

// MARK: - CameraControlDelegate

- (void)cameraDidMove:(CameraControl *)control
{
    [self _redraw];
}

// MARK: - Internal

- (void)_loadMotionData
{
    if ([[NSFileManager defaultManager] fileExistsAtPath:_dataDirectoryPath] == NO) {
        NSLog(@"No directory found at \"%@\"\n", _dataDirectoryPath);
        return;
    }
    
    NSString *filePath = [_dataDirectoryPath stringByAppendingFormat:@"/motion-data.json"];
    
    if ([[NSFileManager defaultManager] fileExistsAtPath:filePath] == NO) {
        NSLog(@"No file exists at path: \"%@\"\n", filePath);
        return;
    }
    [_reconstructionManager setMotionDataPath:filePath];
}

- (NSString * _Nullable)_loadNextFramePath
{
    if ([[NSFileManager defaultManager] fileExistsAtPath:_dataDirectoryPath] == NO) {
        NSLog(@"No directory found at \"%@\"\n", _dataDirectoryPath);
        return nil;
    }
    
    ++_nextFrameIndex;
    
    NSString *filePath = [_dataDirectoryPath stringByAppendingFormat:@"/frame-%03d.ply", (int)(_nextFrameIndex)];
    
    if ([[NSFileManager defaultManager] fileExistsAtPath:filePath] == NO) {
        NSLog(@"No file exists at path: \"%@\"\n", filePath);
        return nil;
    }
    
    frameIndexField.integerValue = _nextFrameIndex;
    
    return filePath;
}

- (void)_assimilateNextFrame
{
    // Load the next cloud from the queue
    NSString *nextFramePath = [self _loadNextFramePath];
    
    if (nextFramePath == nil) {
        [_reconstructionManager finalize];
        _assimilatingAll = NO;
        [self _updateUI];
        [self _redraw];
        return;
    }
    
    _isProcessing = true;
    [self _updateUI];
    
    SCOfflineReconstructionManager *reconstructionManager = _reconstructionManager;
    dispatch_async(_processingQueue, ^{
        [reconstructionManager accumulateFromBPLYWithPath:nextFramePath];

        dispatch_async(dispatch_get_main_queue(), ^{
            _isProcessing = false;
            [self _updateUI];
            [self _redraw];
            
            if (_assimilatingAll) {
                [self _assimilateNextFrame];
            }
        });
    });
}

- (void)_updateValuesFromUI
{
    [_reconstructionManager setICPDownsampleFraction:icpDownsampleField.floatValue];
    [_reconstructionManager setMinDepth:pbfMinDepthField.floatValue];
    [_reconstructionManager setMaxDepth:pbfMaxDepthField.floatValue];
    [_drawCorrespondences setEnabled:(BOOL)drawCorrespondencesCheckbox.integerValue];
    [_drawPointCloud setColorByNormals:(BOOL)colorByNormalsCheckbox.integerValue];
}

- (void)_updateUI
{
    [pbfMinDepthField setFloatValue:_reconstructionManager.minDepth];
    [pbfMaxDepthField setFloatValue:_reconstructionManager.maxDepth];
    [icpDownsampleField setFloatValue:_reconstructionManager.icpDownsampleFraction];
    [drawCorrespondencesCheckbox setIntegerValue:_drawCorrespondences.enabled];
    [colorByNormalsCheckbox setIntegerValue:_drawPointCloud.colorByNormals];
    [frameIndexField setIntegerValue:_nextFrameIndex];
    
    [assimilateNextButton setEnabled:!_isProcessing];
    [assimilateAllButton setEnabled:!_isProcessing || _assimilatingAll];
    [assimilateAllButton setTitle:_isProcessing && _assimilatingAll ? @"Pause" : @"Assimilate All"];
    [resetButton setEnabled:!_isProcessing];
}

- (id<CAMetalDrawable>)_nextDrawableForWindowWithTitle:(NSString *)title
                                                  size:(CGSize)size
                                         configuration:(void (^_Nullable)(NSWindow *))configuration
{
    NSWindow *window = _textureDebugWindowsByTitle[title];
    
    if (window == nil) {
        NSView *view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, size.width, size.height)];
        [view setWantsLayer:YES];
        
        CAMetalLayer *metalLayer = [[CAMetalLayer alloc] init];
        [metalLayer setBounds:view.bounds];
        [metalLayer setDevice:_metalDevice];
        [metalLayer setFramebufferOnly:NO];
        [view setLayer:metalLayer];
        
        window = [[NSWindow alloc] initWithContentRect:NSMakeRect(_textureDebugWindowsByTitle.count * view.bounds.size.width + 250,
                                                                  _textureDebugWindowsByTitle.count * view.bounds.size.height + 100,
                                                                  view.bounds.size.width,
                                                                  view.bounds.size.height)
                                             styleMask:NSWindowStyleMaskTitled|NSWindowStyleMaskFullSizeContentView
                                               backing:NSBackingStoreBuffered
                                                 defer:NO];
        [window setTitle:title];
        [window setContentView:view];
        [window orderBack:nil];
        
        _textureDebugWindowsByTitle[title] = window;
        
        if (configuration != nil) { configuration(window); }
    } else {
        NSRect frame = window.frame;
        frame.size = size;
        [window setFrame:frame display:NO];
    }
    
    CAMetalLayer *metalLayer = (CAMetalLayer *)[[window contentView] layer];
    metalLayer.drawableSize = size;
    
    return [metalLayer nextDrawable];
}

- (void)_redraw
{
    const Surfels& surfels = [_reconstructionManager surfels];
    auto rawFrame = [_reconstructionManager lastRawFrame];
    CGSize rawFrameSize = rawFrame == nullptr ? CGSizeZero : CGSizeMake(rawFrame->width, rawFrame->height);
    const std::vector<uint32_t>& surfelIndexMap = [_reconstructionManager surfelIndexMap];
    id<MTLTexture> surfelIndexMapTexture = [_reconstructionManager surfelIndexMapTexture];
    CGSize surfelIndexMapSize = surfelIndexMapTexture == nil ? CGSizeMake(360, 240) : CGSizeMake(surfelIndexMapTexture.width, surfelIndexMapTexture.height);
    
    if (!_hasCenter && surfels.size() > 0) {
        SCNVector3 center = SurfelsBoundingBoxCenter(surfels);
        [_cameraControl setCenterX:center.x centerY:center.y centerZ:center.z];
        _hasCenter = YES;
    }
    
    id<CAMetalDrawable> drawable = [self _nextDrawableForWindowWithTitle:@"Point Cloud Debug"
                                                                    size:CGSizeMake(900, 900)
                                                           configuration:^(NSWindow *window) {
                                                               [_cameraControl installInView:window.contentView];
                                                           }];
    
    [_visualizationEngine renderSurfels:surfels
                              icpResult:_lastICPResult
                             viewMatrix:[_cameraControl viewMatrix]
                       projectionMatrix:[_cameraControl projectionMatrix]
                           intoDrawable:drawable];
    
    id<CAMetalDrawable> rawDepthsDrawable = [self _nextDrawableForWindowWithTitle:@"Raw Depths"
                                                                             size:rawFrameSize
                                                                    configuration:^(NSWindow *window){}];
    [_drawRawDepths draw:rawFrame into:rawDepthsDrawable];

    id<CAMetalDrawable> mapDrawable = [self _nextDrawableForWindowWithTitle:@"Surfel Index Map"
                                                                       size:surfelIndexMapSize
                                                              configuration:^(NSWindow *window){}];
    [_drawSurfelIndexMap draw:surfelIndexMap into:mapDrawable];
}

// MARK: - SCOfflineReconstructionManagerDelegate

- (void)reconstructionManager:(SCOfflineReconstructionManager *)manager didIterateICPWithResult:(ICPResult)result
{
    dispatch_async(dispatch_get_main_queue(), ^{
        _lastICPResult = result;

        if (!_drawCorrespondences.enabled) { return; }
        
        [self _redraw];
    });
}

@end

static SCNVector3 SurfelsBoundingBoxCenter(const Surfels& surfels)
{
    float minX = FLT_MAX, maxX = -FLT_MAX;
    float minY = FLT_MAX, maxY = -FLT_MAX;
    float minZ = FLT_MAX, maxZ = -FLT_MAX;
    
    for (auto& surfel : surfels) {
        minX = MIN(surfel.position.x(), minX);
        maxX = MAX(surfel.position.x(), maxX);
        minY = MIN(surfel.position.y(), minY);
        maxY = MAX(surfel.position.y(), maxY);
        minZ = MIN(surfel.position.z(), minZ);
        maxZ = MAX(surfel.position.z(), maxZ);
    }
    
    float centerX = 0.5f * (maxX - minX) + minX;
    float centerY = 0.5f * (maxY - minY) + minY;
    float centerZ = 0.5f * (maxZ - minZ) + minZ;
    
    return SCNVector3Make(centerX, centerY, centerZ);
}

NS_ASSUME_NONNULL_END
