//
//  crc32.cpp
//  StandardCyborgFusion
//
//  Created by Ricky Reusser on 12/12/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#include "crc32.hpp"

// Compute a CRC-32 checksum of raw byte data
uint32_t crc32(const void *data, size_t n_bytes) {
    uint32_t crc = 0;
    static uint32_t table[0x100];
    if (!*table) {
        for (size_t i = 0; i < 0x100; ++i) {
            uint32_t r = (uint32_t)i;
            for (int j = 0; j < 8; ++j) {
                r = (r & 1? 0: (uint32_t)0xEDB88320L) ^ r >> 1;
            }
            table[i] = r ^ (uint32_t)0xFF000000L;
        }
    }
    for (size_t i = 0; i < n_bytes; ++i) {
        crc = table[(uint8_t)crc ^ ((uint8_t*)data)[i]] ^ crc >> 8;
    }
    
    return crc;
}
