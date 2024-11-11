//
//  SCMeshTexturing.mm
//  StandardCyborgFusion
//
//  Created by Eric Arnebäck on 5/27/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <CoreImage/CoreImage.h>
#import <CoreServices/CoreServices.h>
#import <Metal/Metal.h>

//#import <StandardCyborgFusion/MeshUvMap.hpp>
#import "../../Algorithm/MeshUvMap.hpp"

#import <standard_cyborg/sc3d/Geometry.hpp>
#import <standard_cyborg/math/Vec4.hpp>
#import <StandardCyborgFusion/PerspectiveCamera+AVFoundation.hpp>
#import <StandardCyborgFusion/SCMesh+Geometry.h>
#import <StandardCyborgFusion/SCMesh_Private.h>
#import <StandardCyborgFusion/SCPointCloud+FileIO.h>
#import <StandardCyborgFusion/SCPointCloud_Private.h>
#import <StandardCyborgFusion/PointCloudIO.hpp>
#import <standard_cyborg/io/imgfile/ColorImageFileIO.hpp>
#import <standard_cyborg/io/ply/GeometryFileIO_PLY.hpp>
#import <fstream>
#import <memory>

//#define SAVE_DIAGNOSTICS

#import "MetalTextureProjection.hpp"
#import "SCMeshTexturing.h"

using namespace standard_cyborg;

// clang-format off
NSString * const SCMeshTexturingAPIErrorDomain = @"SCMeshTexturingAPIErrorDomain";
static NSString * const _ContainerFolderNamePrefix = @"SCMeshTexturing";
static NSString * const _MetadataJSONFilename = @"Metadata.json";
// clang-format on

@interface _RGBFrameMetadata : NSObject
@property (nonatomic) simd_float4x4 viewMatrix;
@property (nonatomic) simd_float4x4 projectionMatrix;
@property (nonatomic) int frameIndex;
@property (nonatomic) NSURL *imageURL;
@end

@implementation _RGBFrameMetadata
@end


@implementation SCMeshTexturing {
    CIContext *_context;
    id<MTLDevice> _metalDevice;
    NSString *_containerPath;

    dispatch_queue_t _reconstructionQueue;
    int _queue_nextFrameIndex;
    NSMutableArray *_metadatas;
    sc3d::PerspectiveCamera _camera;
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        NSString *containerFolderName = [NSString stringWithFormat:@"%@-%@", _ContainerFolderNamePrefix, [[NSUUID UUID] UUIDString]];
        _containerPath = [NSTemporaryDirectory() stringByAppendingPathComponent:containerFolderName];
        [self _ensureContainerDirectory];
        
        dispatch_queue_attr_t attributes = dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL, QOS_CLASS_USER_INITIATED, -1);
        _reconstructionQueue = dispatch_queue_create("SCMeshTexturing._reconstructionQueue", attributes);
        
        _metadatas = [[NSMutableArray alloc] init];
        _metalDevice = MTLCreateSystemDefaultDevice();
        _context = [CIContext contextWithMTLDevice:_metalDevice];
        
        // At the first creation of SCMeshTexturing, make sure we clean up
        // all previous garbage created in previous runs
        static dispatch_once_t onceToken;
        dispatch_once(&onceToken, ^{
            [self _removeDataFromPreviousRuns];
        });
    }
    return self;
}

- (void)dealloc
{
    if ([_containerPath containsString:NSTemporaryDirectory()]) {
        [self _removeContainerDirectory];
    }
}

- (instancetype)initWithReconstructionSessionDataDirectory:(NSString *)path
{
    NSString *jsonPath = [path stringByAppendingPathComponent:_MetadataJSONFilename];
    if (![[NSFileManager defaultManager] fileExistsAtPath:jsonPath]) { return nil; }
    
    self = [self init];
    if (self) {
        _containerPath = path;
        
        std::ifstream fileStream([jsonPath UTF8String]);
        nlohmann::json json;
        fileStream >> json;
        
        [self _loadMetadataAndCalibrationDataFromJSON:json];
    }
    return self;
}

- (void)saveReconstructionSessionDataToDirectory:(NSString *)path
{
    NSFileManager *fileManager = [NSFileManager defaultManager];
    
    if (![fileManager fileExistsAtPath:path]) {
        [fileManager createDirectoryAtPath:path withIntermediateDirectories:YES attributes:nil error:NULL];
    }
    
    // Copy all the images
    for (NSString *filename in [fileManager contentsOfDirectoryAtPath:_containerPath error:NULL]) {
        if ([[filename pathExtension] isEqualToString:@"jpeg"]) {
            NSString *sourcePath = [_containerPath stringByAppendingPathComponent:filename];
            NSString *targetPath = [path stringByAppendingPathComponent:filename];
            [fileManager copyItemAtPath:sourcePath toPath:targetPath error:NULL];
        }
    }
    
    nlohmann::json output;
    [self _writeMetadataAndCalibrationDataToJSON:output];
    
    NSString *JSONPath = [path stringByAppendingPathComponent:_MetadataJSONFilename];
    NSString *JSONString = [NSString stringWithUTF8String:output.dump().c_str()];
    [JSONString writeToFile:JSONPath atomically:YES encoding:NSUTF8StringEncoding error:nil];
}

- (void)saveColorBufferForReconstruction:(CVPixelBufferRef)colorBuffer
                          withViewMatrix:(simd_float4x4)viewMatrix
                        projectionMatrix:(simd_float4x4)projectionMatrix
{
    CIImage *image = [CIImage imageWithCVPixelBuffer:colorBuffer];
    
    if (_flipsInputHorizontally) {
        // Remember that these come in as landscape left, so actually
        // flip vertically to achieve an apparent horizontal flip
        image = [image imageByApplyingTransform:CGAffineTransformTranslate(CGAffineTransformMakeScale(1, -1),
                                                                           image.extent.size.width, 0)];
    }
    
    dispatch_async(_reconstructionQueue, ^{
        int frameIndex = _queue_nextFrameIndex++;
        NSURL *URL = [self _URLForFrameIndex:frameIndex];
        
        _RGBFrameMetadata *metadata = [[_RGBFrameMetadata alloc] init];
        metadata.viewMatrix = viewMatrix;
        metadata.projectionMatrix = projectionMatrix;
        metadata.frameIndex = frameIndex;
        metadata.imageURL = URL;
        
        [_metadatas addObject:metadata];
        
        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
        
        NSError *error = nil;
        BOOL success = [_context writeJPEGRepresentationOfImage:image toURL:URL colorSpace:colorSpace options:@{} error:&error];
        if (!success) {
            NSLog(@"Error writing file to %@: %@", URL, error);
        }
        
        CGColorSpaceRelease(colorSpace);
    });
}

// clang-format off
- (void)reconstructMeshWithWithPointCloud:(SCPointCloud *)pointCloud
                        textureResolution:(NSInteger)textureResolution
                        meshingParameters:(SCMeshingParameters *)meshingParameters
                         coloringStrategy:(SCMeshColoringStrategy)coloringStrategy
                                 progress:(void (^)(float progress, BOOL *))progressHandler
                               completion:(void (^)(NSError * _Nullable, SCMesh * _Nullable))completion
{
    if (textureResolution < 1) {
        NSError *error = [self _buildAPIError:SCMeshTexturingAPIErrorArgument
                                  description:@"Invalid texture resolution: %d", textureResolution];
        dispatch_async(dispatch_get_main_queue(), ^{
            completion(error, nil);
        });
        return;
    }
    
    if ([self _camera].getLegacyImageSize().width <= 0 || _cameraCalibrationFrameWidth <= 0 || _cameraCalibrationFrameHeight <= 0) {
        NSError *error = [self _buildAPIError:SCMeshTexturingAPIErrorArgument
                                  description:@"Camera calibration parameters were not set; you must assign cameraCalibrationData, cameraCalibrationFrameWidth, and cameraCalibrationFrameHeight before invoking reconstructMesh"];
        dispatch_async(dispatch_get_main_queue(), ^{
            completion(error, nil);
        });
        return;
    }
    
    // Returns NO if it should stop
    BOOL (^reportProgress)(float) = ^BOOL(float progress) {
        BOOL shouldStop = NO;
        progressHandler(progress, &shouldStop);
        return !shouldStop;
    };
    
    dispatch_async(_reconstructionQueue, ^{
        NSError *error = nil;
        sc3d::Geometry meshGeometry;
        SCMesh *result = nil;
        
        // Step 1: Build a mesh
        {
            BOOL meshingSuccess = [self _meshPointCloud:pointCloud
                                             parameters:meshingParameters
                                           intoGeometry:meshGeometry
                                                  error:&error
                                        progressHandler:^(float progress, BOOL *shouldStop) {
                *shouldStop = !reportProgress(progress * 1.0 / 3.0);
            }];
            
            if (!meshingSuccess) {
                completion(error, nil);
                return;
            }
        }
        
        if (!reportProgress(1.0 / 3.0)) {
            completion(nil, nil);
            return;
        }
        
        if (coloringStrategy == SCMeshColoringStrategyVertex) {
            reportProgress(2.0 / 3.0);
           
            // calculate the color of the vertices by finding the closest point in the point cloud.
            sc3d::Geometry cloudGeometry;
            [pointCloud toGeometry:cloudGeometry];
            
            std::vector<math::Vec3> newColors;
            
            for (int iv = 0; iv < meshGeometry.vertexCount(); ++iv) {
                math::Vec3 pos = meshGeometry.getPositions()[iv];
                
                int foundIndex = cloudGeometry.getClosestVertexIndex(pos);
                
                if (0 <= foundIndex && foundIndex < cloudGeometry.vertexCount()) {
                    math::Vec3 closestColor = cloudGeometry.getColors()[foundIndex];
                    newColors.push_back(closestColor);
                } else {
                
                    if(cloudGeometry.getColors().size() > 0) {
                        // okay, if we can't find a closest point for some reason, just pick one color from the point cloud as fallback.
                        math::Vec3 col = cloudGeometry.getColors()[0];
                        newColors.push_back(col);
                    } else {
                        // just to cover all bases.
                        math::Vec3 col(1.0, 0.0, 0.0);
                        newColors.push_back(col);
                    }
                    
                }
            }
            meshGeometry.setColors(newColors);
            
            reportProgress(1.0);
            
            result = [SCMesh meshWithVertexColorsFromGeometry:meshGeometry];
        } else {
            std::vector<float> textureData;
            
            // Step 2: UV map the mesh
            {
                BOOL uvMapSuccess = algorithms::uvmapMesh(meshGeometry);
                
                if (!uvMapSuccess) {
                    error = [self _buildAPIError:SCMeshTexturingAPIErrorArgument
                                     description:@"UV unwrapping failed: %s", algorithms::getUvmapMeshErrorMessage().c_str()];
                    completion(error, nil);
                    return;
                }
            }
            
            if (!reportProgress(2.0 / 3.0)) {
                completion(nil, nil);
                return;
            }
            
            // Step 3: Project onto the UV-mapped mesh
            {
                BOOL projectionSuccess = [self _doProjectionWithUvMappedMesh:meshGeometry
                                                            outputTextureRes:textureResolution
                                                                      result:textureData
                                                                       error:&error
                                                             progressHandler:^(float progress) {
                    // TODO: Support cancelling projection mid-operation
                    reportProgress(2.0 / 3.0 + progress * 1.0 / 3.0);
                }];
                
                if (projectionSuccess == NO || textureData.size() == 0) {
                    error = [self _buildAPIError:SCMeshTexturingAPIErrorArgument
                                     description:@"textureProjection.finishProjecting failed"];
                    completion(error, nil);
                    return;
                }
            }
            
            // Step 4: Convert from sc3d::Geometry to SCMesh
            {
                result = [SCMesh meshFromGeometry:meshGeometry
                                      textureData:textureData
                                textureResolution:textureResolution];
            }
        }
        
                
        if (result == nil) {
            completion(error, nil);
            return;
        }

        if (!reportProgress(1.0)) {
            completion(nil, nil);
            return;
        }
        
#ifdef SAVE_DIAGNOSTICS
        
        /*
         - (void)_writeDiagnosticsToDisk:(const std::vector<float> &)textureData
         textureResolution:(int)textureResolution
                  geometry:(const Geometry &)geometry
         */
        [self _writeDiagnosticsToDisk:textureData
                           textureResolution:textureResolution
                             geometry:meshGeometry];
#endif
        
        completion(nil, result);
    });
}

// clang-format on
- (void)reset
{
    [self _removeContainerDirectory];
    [_metadatas removeAllObjects];
    _queue_nextFrameIndex = 0;
}

// MARK: - Private

- (NSError *)_buildAPIError:(SCMeshTexturingAPIError)errorCode
                description:(NSString *)description, ...
{
    va_list args;
    va_start(args, description);
    NSString *message = [[NSString alloc] initWithFormat:description arguments:args];
    va_end(args);
    
    return [NSError errorWithDomain:SCMeshTexturingAPIErrorDomain
                               code:errorCode
                           userInfo:@{NSDebugDescriptionErrorKey: message}];
}

- (void)_removeDataFromPreviousRuns
{
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSError *error = nil;
    
    for (NSString *path in [fileManager contentsOfDirectoryAtPath:NSTemporaryDirectory() error:&error]) {
        if ([path hasPrefix:_ContainerFolderNamePrefix]) {
            NSString *absolutePath = [NSTemporaryDirectory() stringByAppendingPathComponent:path];
            [fileManager removeItemAtPath:absolutePath error:&error];
        }
    }
    
    if (error != nil) {
        NSLog(@"Error removing data from previous runs in %@: %@", _containerPath, error);
    }
}

- (void)_ensureContainerDirectory
{
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSError *error = nil;
    
    if (![fileManager fileExistsAtPath:_containerPath]) {
        if (![fileManager createDirectoryAtPath:_containerPath withIntermediateDirectories:NO attributes:nil error:&error]) {
            NSLog(@"Error creating container directory at %@: %@", _containerPath, error);
        }
    }
}

- (void)_removeContainerDirectory
{
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSError *error = nil;
    
    if ([fileManager fileExistsAtPath:_containerPath]) {
        if (![fileManager removeItemAtPath:_containerPath error:&error]) {
            NSLog(@"Error removing container directory at %@: %@", _containerPath, error);
        } else {
            NSLog(@"Removed container path %@", _containerPath);
        }
    }
}

- (sc3d::PerspectiveCamera)_camera
{
    if (_camera.getLegacyImageSize().width == 0) {
        _camera = PerspectiveCameraFromAVCameraCalibrationData(_cameraCalibrationData,
                                                               _cameraCalibrationFrameWidth,
                                                               _cameraCalibrationFrameHeight);
    }
    
    return _camera;
}

- (NSURL *)_URLForFrameIndex:(int)frameIndex
{
    NSAssert(frameIndex <= 999, @"At most 999 frames are supported for analysis");
    
    [self _ensureContainerDirectory];
    
    NSString *filename = [NSString stringWithFormat:@"Frame-%03d.jpeg", frameIndex];
    NSString *path = [_containerPath stringByAppendingPathComponent:filename];
    
    return [NSURL fileURLWithPath:path];
}

- (void)_loadImageFromFileAtURL:(NSURL *)URL
                     intoVector:(std::vector<math::Vec4> &)buffer
                          width:(int *)outWidth
                         height:(int *)outHeight
                       channels:(int *)outChannels
                               
{
    CIImage *image = [CIImage imageWithContentsOfURL:URL];
    if (image == nil) { return; }
    
    CGColorSpaceRef linearColorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGBLinear);
    int imageWidth = image.extent.size.width;
    int imageHeight = image.extent.size.height;
    
    buffer.resize(imageWidth * imageHeight);
    
    [_context render:image
            toBitmap:buffer.data()
            rowBytes:4 * imageWidth * sizeof(float)
              bounds:image.extent
              format:kCIFormatRGBAf
          colorSpace:linearColorSpace];
    
    CGColorSpaceRelease(linearColorSpace);
    
    *outWidth = imageWidth;
    *outHeight = imageHeight;
    *outChannels = 4;
}

- (void)_writeMetadataAndCalibrationDataToJSON:(nlohmann::json &)json
{
    nlohmann::json metadatas = nlohmann::json::array();
    
    for (_RGBFrameMetadata *metadata in _metadatas) {
        simd_float4x4 viewMatrix = metadata.viewMatrix;
        simd_float4x4 projectionMatrix = metadata.projectionMatrix;
        int frameIndex = metadata.frameIndex;
        NSString *imageFilename = [metadata.imageURL lastPathComponent];
        
        nlohmann::json metadataJSON;
        metadataJSON["view_matrix"] = std::array<float, 16>{
            viewMatrix.columns[0][0], viewMatrix.columns[0][1], viewMatrix.columns[0][2], viewMatrix.columns[0][3],
            viewMatrix.columns[1][0], viewMatrix.columns[1][1], viewMatrix.columns[1][2], viewMatrix.columns[1][3],
            viewMatrix.columns[2][0], viewMatrix.columns[2][1], viewMatrix.columns[2][2], viewMatrix.columns[2][3],
            viewMatrix.columns[3][0], viewMatrix.columns[3][1], viewMatrix.columns[3][2], viewMatrix.columns[3][3] //
        };
        metadataJSON["projection_matrix"] = std::array<float, 16>{
            projectionMatrix.columns[0][0], projectionMatrix.columns[0][1], projectionMatrix.columns[0][2], projectionMatrix.columns[0][3],
            projectionMatrix.columns[1][0], projectionMatrix.columns[1][1], projectionMatrix.columns[1][2], projectionMatrix.columns[1][3],
            projectionMatrix.columns[2][0], projectionMatrix.columns[2][1], projectionMatrix.columns[2][2], projectionMatrix.columns[2][3],
            projectionMatrix.columns[3][0], projectionMatrix.columns[3][1], projectionMatrix.columns[3][2], projectionMatrix.columns[3][3] //
        };
        metadataJSON["frame_index"] = frameIndex;
        metadataJSON["image_filename"] = std::string([imageFilename UTF8String]);
        
        metadatas.push_back(metadataJSON);
    }
    
    json["metadatas"] = metadatas;
    json["camera_intrinsics"] = PointCloudIO::JSONFromPerspectiveCamera([self _camera]);
    json["camera_calibration_frame_width"] = _cameraCalibrationFrameWidth;
    json["camera_calibration_frame_height"] = _cameraCalibrationFrameHeight;
}

- (void)_loadMetadataAndCalibrationDataFromJSON:(nlohmann::json &)json
{
    _camera = PointCloudIO::PerspectiveCameraFromJSON(json);
    _cameraCalibrationFrameWidth = json["camera_calibration_frame_width"];
    _cameraCalibrationFrameHeight = json["camera_calibration_frame_height"];
    
    for (auto &metadataJSON : json["metadatas"]) {
        std::array<float, 16> viewMatrix = metadataJSON["view_matrix"];
        std::array<float, 16> projectionMatrix = metadataJSON["projection_matrix"];
        int frameIndex = metadataJSON["frame_index"];
        std::string imageFilename = metadataJSON["image_filename"];
        
        simd_float4x4 viewMatrixSimd, projectionMatrixSimd;
        memcpy(&viewMatrixSimd, viewMatrix.data(), sizeof(simd_float4x4));
        memcpy(&projectionMatrixSimd, projectionMatrix.data(), sizeof(simd_float4x4));
        NSString *imageFilenameNS = [NSString stringWithUTF8String:imageFilename.c_str()];
        
        _RGBFrameMetadata *metadata = [[_RGBFrameMetadata alloc] init];
        metadata.viewMatrix = viewMatrixSimd;
        metadata.projectionMatrix = projectionMatrixSimd;
        metadata.frameIndex = frameIndex;
        metadata.imageURL = [NSURL fileURLWithPath:[_containerPath stringByAppendingPathComponent:imageFilenameNS]];
        
        [_metadatas addObject:metadata];
    }
}

- (BOOL)_doProjectionWithUvMappedMesh:(const sc3d::Geometry &)meshGeo
                     outputTextureRes:(NSInteger)outputTextureRes
                               result:(std::vector<float> &)resultOut
                                error:(NSError **)errorOut
                      progressHandler:(void (^)(float progress))progressHandler
{
    if ([_metadatas count] == 0) {
        if (errorOut != NULL) {
            *errorOut = [self _buildAPIError:SCMeshTexturingAPIErrorArgument
                                 description:@"-saveColorBufferForReconstruction:withViewMatrix:projectionMatrix: was not called before reconstructing"];
        }
        return NO;
    }
    
    MetalTextureProjection textureProjection(_metalDevice, (int)outputTextureRes);
    
    // initialize for texture projection.
    std::vector<math::Vec4> vecFrameData;
    {
        _RGBFrameMetadata *metadata = _metadatas[0];
        NSURL *imageURL = metadata.imageURL;
        std::string jpegPath([imageURL.path UTF8String]);

        sc3d::ColorImage imageFrame;
        io::imgfile::ReadColorImageFromFile(imageFrame, jpegPath);

        if (imageFrame.getWidth() == 0 || imageFrame.getHeight() == 0) {
            if (errorOut != NULL) {
                *errorOut = [self _buildAPIError:SCMeshTexturingAPIErrorInternal
                                     description:@"ReadColorImageFromFile failed for path: %s", jpegPath.c_str()];
            }
            return NO;
        }
        
        sc3d::PerspectiveCamera camera = [self _camera];

        if (!textureProjection.startProjecting(imageFrame.getWidth(), imageFrame.getHeight(), meshGeo, camera)) {
            if (errorOut != NULL) {
                *errorOut = [self _buildAPIError:SCMeshTexturingAPIErrorInternal
                                     description:@"textureProjection.startProjecting failed"];
            }
            return NO;
        }
        
        vecFrameData.resize(imageFrame.getWidth() * imageFrame.getHeight(), math::Vec4());
    }
    
    sc3d::ColorImage imageFrame;
    int frameIndex = 0;
    
    for (_RGBFrameMetadata *metadata in _metadatas) {
        NSURL *imageURL = metadata.imageURL;
        
        // load the jpeg to be projected.
        {
            int width = 0;
            int height = 0;
            int channels = 0;
            
            [self _loadImageFromFileAtURL:imageURL
                               intoVector:vecFrameData
                                    width:&width
                                   height:&height
                                 channels:&channels];
            
            if (width == 0 || height == 0 || channels != 4) {
                *errorOut = [self _buildAPIError:SCMeshTexturingAPIErrorArgument
                                     description:@"LoadImageFromFile failed for path: %@", imageURL];
                return NO;
            }
            
            imageFrame.reset(width, height, vecFrameData);
        }
        
        // we need to invert the view matrix.
        simd_float4x4 viewMatrix = simd_inverse(metadata.viewMatrix);
        
        simd_float4x4 projectionMatrix = metadata.projectionMatrix;
        
        if (!textureProjection.projectSingleTexture(viewMatrix, projectionMatrix, imageFrame, meshGeo)) {
            *errorOut = [self _buildAPIError:SCMeshTexturingAPIErrorArgument
                                 description:@"textureProjection.projectTexture failed"];
            return NO;
        }
        
        progressHandler((float)frameIndex / (float)[_metadatas count]);
        ++frameIndex;
    }
    
    resultOut = textureProjection.finishProjecting(meshGeo);
    
    return YES;
}

- (BOOL)_meshPointCloud:(SCPointCloud *)pointCloud
             parameters:(SCMeshingParameters *)parameters
           intoGeometry:(sc3d::Geometry &)geometryOut
                  error:(NSError **)errorOut
        progressHandler:(void (^)(float, BOOL *))progressHandler
{
    // Write the point cloud to a .ply file, so we can use SCMeshingOperation
    NSString *plyFilename = @"temp-point-cloud.ply";
    NSString *pointCloudPlyPath = [_containerPath stringByAppendingPathComponent:plyFilename];
    NSString *outputPath = [pointCloudPlyPath stringByReplacingOccurrencesOfString:@".ply" withString:@"-mesh.ply"];
    
    [self _ensureContainerDirectory];
    
    BOOL success = [pointCloud writeToPLYAtPath:pointCloudPlyPath];
    if (!success) {
        if (errorOut != NULL) {
            *errorOut = [self _buildAPIError:SCMeshTexturingAPIErrorInternal
                                 description:@"Error writing to %@", pointCloudPlyPath];
        }
        return NO;
    }
    
    __block BOOL shouldStop = NO;
    
    SCMeshingOperation *operation = [[SCMeshingOperation alloc] initWithInputPLYPath:pointCloudPlyPath outputPLYPath:outputPath];
    operation.parameters = parameters;
    
    __weak SCMeshingOperation *weakOperation = operation;
    operation.progressHandler = ^(float progress) {
        // Adapt the progress handler to allow cancellation
        progressHandler(progress, &shouldStop);
        
        if (shouldStop) {
            NSLog(@"Cancelling meshing operation");
            [weakOperation cancel];
        }
    };
    
    [operation start];
    
    if ([operation isCancelled]) {
        return NO;
    } else {
        io::ply::ReadGeometryFromPLYFile(geometryOut, std::string([outputPath UTF8String]));
        
        // color is no longer needed beyond this point. discard.
        std::vector<math::Vec3> newColors(geometryOut.vertexCount(), math::Vec3{1, 1, 1});
        
        if (!geometryOut.setColors(newColors)) {
            return NO;
        }
        
        return YES;
    }
}

#ifdef SAVE_DIAGNOSTICS
- (void)_writeDiagnosticsToDisk:(const std::vector<float> &)textureData
              textureResolution:(int)textureResolution
                       geometry:(const Geometry &)geometry
{
    // Output the resulting texture and mesh to disk for debugging purposes.
    // This code will be removed later.
    NSString *debugDirectory = [NSTemporaryDirectory() stringByAppendingPathComponent:@"/DebugMeshTexturing"];
    NSString *texturePath = [debugDirectory stringByAppendingPathComponent:@"mesh-000.png"];
    NSString *geoPath = [debugDirectory stringByAppendingPathComponent:@"mesh-000.ply"];
    NSString *gltfPath = [debugDirectory stringByAppendingPathComponent:@"out.glb"];
    
    // Re-create the debug directory
    if ([[NSFileManager defaultManager] fileExistsAtPath:debugDirectory]) {
        [[NSFileManager defaultManager] removeItemAtPath:debugDirectory error:NULL];
    }
    [[NSFileManager defaultManager] createDirectoryAtPath:debugDirectory withIntermediateDirectories:NO attributes:nil error:NULL];
    
    std::vector<Vec4> rgbaData;
    for (int i = 0; i < textureData.size(); i += 4) {
        rgbaData.push_back(Vec4(textureData[i + 0],
                                textureData[i + 1],
                                textureData[i + 2],
                                1));
    }
    
    ColorImage colorImage(textureResolution, textureResolution, rgbaData);
    
    SCMesh *scMesh = [SCMesh meshFromGeometry:geometry
                                  textureData:textureData
                            textureResolution:textureResolution];
    
    WriteColorImageToFile([texturePath UTF8String], colorImage);
    WriteGeometryToPLYFile([geoPath UTF8String], geometry);
    [scMesh writeToGLBAtPath:gltfPath];
}
#endif

@end
