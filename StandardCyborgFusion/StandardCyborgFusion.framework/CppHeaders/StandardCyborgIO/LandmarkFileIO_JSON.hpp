//
//  LandmarkFileIO_JSON.hpp
//  StandardCyborgIO
//
//  Created by Ricky Reusser on 7/12/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <istream>
#include <ostream>

namespace StandardCyborg {

class Landmark;

/** Read a landmark from a stream of JSON data */
extern bool ReadLandmarkFromJSONStream(Landmark& landmarkOut, std::istream& input);

/** Read a landmark from a JSON file */
extern bool ReadLandmarkFromJSONFile(Landmark& landmarkOut, std::string filename);

/** Serialize a landmark as JSON to an output stream */
extern bool WriteLandmarkToJSONStream(std::ostream& output, const Landmark& landmark);
    
/** Write a landmark to a JSON file */
extern bool WriteLandmarkToJSONFile(std::string filename, const Landmark& landmark);

} // namespace StandardCyborg

