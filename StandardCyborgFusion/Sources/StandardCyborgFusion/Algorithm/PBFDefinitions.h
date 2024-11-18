//
//  PBFDefinitions.h
//  StandardCyborgFusion
//
//  Created by Ricky Reusser on 5/2/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

// Equal to 1 << 31, which is just a bit below __UINT32_MAX__ (1<<32 - 1),
// but __UINT32_MAX__ seems to cause it to fail to assimilate.
#define EMPTY_SURFEL_INDEX (2147483648u)
