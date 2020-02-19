//
//  PolylineFileIO_JSON.hpp
//  StandardCyborgIO
//
//  Created by Ricky Reusser on 2019-07-10.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <istream>
#include <ostream>

namespace StandardCyborg {

class Polyline;

/** Read a polyline from an stream of JSON data */
extern bool ReadPolylineFromJSONStream(Polyline& polylineOut, std::istream& input);

/** Read a polyline from a JSON file */
extern bool ReadPolylineFromJSONFile(Polyline& polylineOut, std::string filename);

/** Serialize a polyline as JSON to an output stream */
extern bool WritePolylineToJSONStream(std::ostream& output, const Polyline& polyline);
    
/** Write a polyline to a JSON file */
extern bool WritePolylineToFile(std::string filename, const Polyline& polyline);

} // namespace StandardCyborg
