//
//  BaseTestViewController.h
//  StandardCyborgGeometryTestbed
//
//  Created by Aaron Thompson on 4/5/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import <SceneKit/SceneKit.h>
#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

/**
 This class was made to be subclassed and overridden at the exposed methods below
 */
@interface BaseTestViewController : UIViewController

- (SCNNode *)loadTestCaseNamed:(NSString *)testCaseName meshed:(BOOL)meshed;
- (void)addNode:(SCNNode *)node;
- (void)removeNode:(SCNNode *)node;

// MARK: - Required Subclass Overrides

- (SCNView *)sceneView;

// MARK: - Optional Subclass Overrides

- (void)viewDidLoad NS_REQUIRES_SUPER;

- (void)reloadScene;

- (void)didTapNodeWithResult:(SCNHitTestResult *)result;


@end

NS_ASSUME_NONNULL_END
