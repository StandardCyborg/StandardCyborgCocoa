//
//  SCReconstructionManager_Private.h
//  StandardCyborgFusion
//
//  Created by Ricky Reusser on 12/13/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#import <StandardCyborgFusion/SCReconstructionManager.h>

@protocol SCReconstructionManagerDelegatePrivate <SCReconstructionManagerDelegate>

- (void)metalReconstructionManager:(SCReconstructionManager *)manager
didAssimilateFrameWithExtendedMetadata:(SCAssimilatedFrameMetadata)metadata
                reconstructedModel:(SCPointCloud *)model;

@end
