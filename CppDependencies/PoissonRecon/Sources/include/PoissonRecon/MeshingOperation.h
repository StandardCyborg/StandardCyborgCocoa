//
//  MeshingOperation.h
//  Meshing
//
//  Created by Aaron Thompson on 4/26/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <Foundation/Foundation.h>

/** Reconstructs a mesh from a point cloud and trims the edges on the result. */
@interface MeshingOperation : NSOperation

- (instancetype)init NS_UNAVAILABLE;

- (instancetype)initWithInputFilePath:(NSString *)inputPath
                       outputFilePath:(NSString *)outputPath;

/** Set this to be informed about the progress of the meshing operation.
    @param progress From 0.0-1.0
 */
@property (nonatomic, copy) void (^progressHandler)(float progress);

/** The resolution of the reconstructed mesh vertices.
    Higher values will result in more vertices per meshes,
    and also take longer to reconstruct.
    Range is 1-10, default is 5.
 */
@property (nonatomic) int resolution;

/** The smoothness of the reconstructed mesh vertex positions.
    Range is 1-10, default is 2.
 */
@property (nonatomic) int smoothness;

/** The amount of surface trimming for low-density mesh regions.
    Range is 0-10, defaults to 5; higher numbers trim more away; 0 = don't trim.
 */
@property (nonatomic) int surfaceTrimmingAmount;

/** If YES, attempts to build a closed mesh.
 */
@property (nonatomic) BOOL closed;

@end
