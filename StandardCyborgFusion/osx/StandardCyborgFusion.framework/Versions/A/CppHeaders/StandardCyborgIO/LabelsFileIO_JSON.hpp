//
//  LabelsFileIO.hpp
//  StandardCyborgIO
//
//  Created by eric on 2019-06-06.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#pragma once

#include <istream>
#include <ostream>

namespace StandardCyborg {

class Labels;

/** Read labels from JSON into an existing Labels instance */
extern bool ReadLabelsFromJSONFile(Labels& labelsOut, std::string filename);

/** Read labels from a stream of JSON into an existing Labels instance */
extern bool ReadLabelsFromJSONStream(Labels& labelsOut, std::istream& input);

/** Serialize labels into an output stream */
extern bool WriteLabelsToJSONStream(std::ostream& output, const Labels& labels);

/** Serialize Labels to a JSON file */
extern bool WriteLabelsToJSONFile(std::string filename, const Labels& labels);

} // namespace StandardCyborg
