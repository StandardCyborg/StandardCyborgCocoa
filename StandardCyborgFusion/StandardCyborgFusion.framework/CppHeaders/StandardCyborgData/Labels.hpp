//
//  Labels.hpp
//  StandardCyborgSDK
//
//  Created by eric on 2019-06-06.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <string>
#include <vector>

#include <StandardCyborgData/VertexSelection.hpp>

namespace StandardCyborg {

class LabeledRegion {
public:
    VertexSelection indices;
    std::string name;
};

class Labels {
private:
    std::vector<LabeledRegion> labels;
    
public:
    Labels() {}
    
    Labels(const std::vector<LabeledRegion>& labels_)
    {
        labels = labels_;
    }
    
    std::vector<LabeledRegion>& getLabels()
    {
        return labels;
    }
    
    const std::vector<LabeledRegion>& getLabels() const
    {
        return labels;
    }
};

}
