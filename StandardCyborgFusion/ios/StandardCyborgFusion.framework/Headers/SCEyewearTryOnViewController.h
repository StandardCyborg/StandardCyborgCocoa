//
//  SCEyewearTryOnViewController.h
//  StandardCyborgFusion
//
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#include <TargetConditionals.h>
#if !TARGET_OS_OSX

#import <UIKit/UIKit.h>
@class ARSCNView;
@class SCNNode;
@class SCEyewearNode;

NS_ASSUME_NONNULL_BEGIN

/** Standard Cyborg’s Virtual Try-on tools. Features including:
 - Easily swap assets live
 - Interactive positioning
 */
@interface SCEyewearTryOnViewController : UIViewController

@property (nonatomic, readonly) ARSCNView *sceneView;

/** The contents of this node will be overlaid on the user's face.
    To adjust the position of this eyewear relative to the face, adjust the position,
    fadeStartZ, and fadeDistance properties of this node directly. */
@property (nonatomic, retain) SCEyewearNode *eyewearNode;

/** When YES, double tapping with two fingers toggles a HUD with controls to adjust
    properties of the eyewearNode, such as its position relative to the face */
@property (nonatomic) BOOL controlsHUDEnabled;

/** To conveniently add an environment map (e.g. to show a reflected scene in eyewear lenses,
    pass in a URL to an HDR image to be used as the map. */
- (void)loadEnvironmentMapAtURL:(NSURL *)mapURL;

@end

NS_ASSUME_NONNULL_END

#endif // !TARGET_OS_OSX
