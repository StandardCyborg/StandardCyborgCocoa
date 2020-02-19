//
//  SCEyewearNode.h
//  StandardCyborgFusion
//
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <SceneKit/SceneKit.h>

NS_ASSUME_NONNULL_BEGIN

/** Draws its contents faded out toward the back of the head */
@interface SCEyewearNode : SCNNode

/** The direction within the asset where the fade should go.
    Defaults to (0, 0, 1), meaning it will fade out toward +z. */
@property (nonatomic) SCNVector3 fadeDirection;

/** Fades out the eyewear toward the back of the head, starting at this z position, in meters */
@property (nonatomic) float fadeStart;

/** Controls the distance over which the eyewear fades out, in meters */
@property (nonatomic) float fadeDistance;

@end

NS_ASSUME_NONNULL_END
