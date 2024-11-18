//
//  ScreenSpaceLandmark.hpp
//  StandardCyborgFusion
//
//  Created by Ricky Reusser on 4/19/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

struct ScreenSpaceLandmark {
    // Landmark position relative to the incoming frame with (x, y) in [0, 1] x [0, 1]
    float x;
    float y;
    
    int landmarkId;
};
