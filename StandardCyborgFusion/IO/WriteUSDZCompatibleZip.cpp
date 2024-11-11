//
//  WriteUSDZCompatibleZip.cpp
//  StandardCyborgFusion
//
//  Created by Ricky Reusser on 12/2/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#include "WriteUSDZCompatibleZip.hpp"

#include <fstream>
#include <ctime>
#include <sys/stat.h>
#include "crc32.hpp"

// A sample driver program. It's very simple. It just takes a zip file name and a
// list of file names and creates a usdz-compatible zip archive.
/*
int main (int argc, char** argv) {
    std::vector<std::string> filenameList;

    for (int i = 2; i < argc; i++) {
        filenameList.push_back(std::string(argv[i]));
    }

    std::string outputFilename (argv[1]);

    WriteUSDZCompatibleZip(outputFilename, filenameList);

    return 0;
};
*/

// WARNING: This is very close but might be off by a day and requires a bit
// more debugging (maybe. honestly, it's not critical).
struct MSDOSTimestamp {
    uint16_t date = 0x0000;
    uint16_t time = 0x0000;
    
    MSDOSTimestamp() { }
    
    MSDOSTimestamp(const time_t inputTime) {
        fromTime(inputTime);
    }
    
    void fromTime(const time_t inputTime) {
        struct tm *modLocaltime = localtime(&inputTime);
        
        time = ((modLocaltime->tm_sec / 2) << 0) |
        ((modLocaltime->tm_min) << 4) |
        ((modLocaltime->tm_hour) << 11);
        
        date = ((modLocaltime->tm_mday) << 0) |
        ((modLocaltime->tm_mon + 1) << 5) |
        ((modLocaltime->tm_year - 80) << 9);
    }
};

// The data portions of USDZ files must be 64-byte aligned. We use the
// vendor-custom Extra Field portion of the zip files to add the necessary
// padding. Note that the headers are *not* 64-byte aligned; only the file
// data is.
static off_t offsetOfNextAlignedChunk(off_t offset, unsigned log2Alignment) {
    return (((offset - 1) >> log2Alignment) + 1) << log2Alignment;
}

// The local header directly preceeding each file in the zip archive
struct __attribute__((__packed__)) ZipFileLocalFileHeader {
    // magic number. alwasy: 0x04034b50
    uint32_t signature = 0x04034b50L;
    
    // Generally 0x000a for us
    uint16_t versionToExtract = 0x000a;
    
    // Zero is fine here.
    uint16_t generalPurposeBitFlag = 0x0000;
    
    // Compression method. since uncompressed, just 0x0000
    uint16_t compressionMethod = 0x0000;
    
    uint16_t lastModificationTime = 0x0000;
    uint16_t lastModificationDate = 0x0000;
    
    // CRC-32 hash of file contents
    uint32_t crc32checksum = 0x00000000L;
    
    // In bytes, compressed and uncompressed identical since not compressed
    uint32_t compressedSize = 0x00000000L;
    uint32_t uncompressedSize = 0x00000000L;
    
    // Number of bytes in file name
    uint16_t filenameLength = 0x0000;
    
    // Number of bytes in extra field. Not entirely clear what this should
    // be since it's all zeros. Possibly padded for the sake of alignment?
    // The only nonzero part appears to be 0x00151986
    uint16_t extraFieldLength = 0x0000;
    
    // Followed by
    // - <compressedSize> bytes of data
    // - <extraFieldLength> bytes of extra field data
};

// The file header at the end of the zip archive
struct __attribute__((__packed__)) ZipCentralDirectoryFileHeader {
    // magic number. alwasy: 0x01024b50
    uint32_t signature = 0x02014b50L;
    
    // seems to be 0x00
    uint16_t versionMadeBy = 0x0000;
    
    // 0x0a
    uint16_t versionNeededToExtract = 0x000a;
    
    // Again, just 0x00
    uint16_t generalPurposeBitFlag = 0x0000;
    
    uint16_t compressionMethod = 0x0000;
    
    uint16_t lastModificationTime = 0x0000;
    uint16_t lastModificationDate = 0x0000;
    
    uint32_t crc32checksum = 0x00000000L;
    
    uint32_t compressedSize = 0x00000000L;
    uint32_t uncompressedSize = 0x00000000L;
    
    uint16_t filenameLength = 0x0000;
    uint16_t extraFieldLength = 0x0000;
    uint16_t firstCommentLength = 0x0000;
    
    uint16_t diskNumberWhereStarts = 0x0000;
    
    uint16_t internalFileAttributes = 0x0000;
    uint32_t externalFileAttributes = 0x00000000L;
    
    uint32_t relativeOffsetOfLocalFileHeader = 0x00000000L;
    
    // Followed by:
    // - <filenameLength> bytes of file name
    // - <extraFieldLength> bytes of extra field data
    //     for Scan_Albedo.png, 0x391986. For the usdc, 0x151986
    // - <firstCommentLenght> bytes of comment data
};

// The final record which closes everything off
struct __attribute__((__packed__)) ZipEndOfCentralDirectoryRecord {
    // magic #: 0x06054b50
    uint32_t signature = 0x06054b50L;
    
    uint16_t diskNumber = 0x0000;
    uint16_t diskOfCentralDirectoryStart = 0x0000;
    uint16_t diskCentralDirectoryRecordCount = 0x0000;
    uint16_t centralDirectoryRecordCount = 0x0000;
    
    // In bytes
    uint32_t centralDirectorySize = 0x00000000L;
    
    // In bytes, relative to the start of the archive
    uint32_t centralDirectoryOffset = 0x00000000L;
    
    uint16_t commentLength = 0x0000;
};

int WriteUSDZCompatibleZip(const std::string outputFilename, const std::vector<std::string> filenameList) {
    std::ofstream zip;
    zip.open(outputFilename);
    
    // A vector to store central directory data and its extra parts so that
    // we can write them at the end of the file
    std::vector<ZipCentralDirectoryFileHeader> centralDirectoryHeaders;
    std::vector<std::vector<char>> extraFields;
    
    // track the offset within the resulting zip file
    off_t offset = 0;
    
    // Loop over files, reading each one, computing the checksum, and writing it back out.
    // We could presumably optimize this by not reading the whole thing into memory, but
    // then we'd have to jump around in the file in order to write the offset;
    for (off_t i = 0; i < filenameList.size(); i++) {
        std::string filename = filenameList[i];
        
        // Get time and date from file stats
        MSDOSTimestamp modifiedAtDosTimestamp;
        struct stat fileStats;
        if (stat(filename.c_str(), &fileStats) == 0) {
            time_t modifiedTime = time(&fileStats.st_mtime);
            modifiedAtDosTimestamp.fromTime(modifiedTime);
        }
        
        // Create an input stream
        std::ifstream file (filename, std::ios::binary | std::ios::ate);
        std::streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        
        // Create a buffer in which to store the file
        std::vector<char> buffer (fileSize);
        
        // Read the whole file into memory
        if (file.read(buffer.data(), fileSize)) {
            void *data = &buffer.front();
            
            uint32_t checksum = crc32(data, buffer.size());
            off_t filenameOffset = offset + sizeof(ZipFileLocalFileHeader);
            off_t extraFieldOffset = filenameOffset + filename.size();
            
            // Compute the next 64-byte alignment
            size_t requiredExtraBytes = 4;
            off_t paddedOffset = offsetOfNextAlignedChunk(extraFieldOffset + requiredExtraBytes, 6);
            off_t extraFieldLength = paddedOffset - extraFieldOffset;
            
            ZipFileLocalFileHeader localHeader {};
            
            localHeader.crc32checksum = checksum;
            localHeader.compressedSize = (uint32_t)buffer.size();
            localHeader.uncompressedSize = (uint32_t)buffer.size();
            localHeader.filenameLength = filename.size();
            localHeader.extraFieldLength = extraFieldLength;
            localHeader.lastModificationDate = modifiedAtDosTimestamp.date;
            localHeader.lastModificationTime = modifiedAtDosTimestamp.time;
            
            std::vector<char> extraField (extraFieldLength);
            
            // These are hard-coded attributes. I don't know what they mean, but they
            // seem to work even though they're not strictly identical to the file I
            // was aiming to recreate
            extraField[0] = 0x86;
            extraField[1] = 0x19;
            extraField[2] = 0x15;
            
            zip.write((char*)&localHeader, sizeof(ZipFileLocalFileHeader));
            zip << filename;
            zip.write(&extraField.front(), extraFieldLength);
            zip.write(&buffer.front(), buffer.size());
            
            // Store a central directory header, for writing at the end of the file
            ZipCentralDirectoryFileHeader centralDirectoryHeader;
            
            centralDirectoryHeader.lastModificationDate = modifiedAtDosTimestamp.date;
            centralDirectoryHeader.lastModificationTime = modifiedAtDosTimestamp.time;
            centralDirectoryHeader.crc32checksum = checksum;
            centralDirectoryHeader.compressedSize = (uint32_t)buffer.size();
            centralDirectoryHeader.uncompressedSize = (uint32_t)buffer.size();
            centralDirectoryHeader.filenameLength = filename.size();
            centralDirectoryHeader.extraFieldLength = extraField.size();
            centralDirectoryHeader.relativeOffsetOfLocalFileHeader = (uint32_t)offset;
            
            // Store this so we can duplicate it in the central directory
            extraFields.push_back(extraField);
            
            // Before incrementing, store the offset of this header
            centralDirectoryHeaders.push_back(centralDirectoryHeader);
            
            // The offset with padding becomes the new total offset
            offset = paddedOffset + buffer.size();
        }
    }
    
    // Keep track of the central directory size and offset
    off_t centralDirectoryOffset = offset;
    size_t centralDirectorySize = 0;
    
    // Write the central directory records
    for (off_t i = 0; i < filenameList.size(); i++) {
        std::string filename = filenameList[i];
        
        centralDirectorySize += sizeof(ZipCentralDirectoryFileHeader);
        centralDirectorySize += filenameList[i].size();
        centralDirectorySize += extraFields[i].size();
        
        zip.write((char*)&centralDirectoryHeaders[i], sizeof(ZipCentralDirectoryFileHeader));
        zip << filenameList[i];
        zip.write(&extraFields[i].front(), extraFields[i].size());
    }
    
    // Write the end of central directory record
    ZipEndOfCentralDirectoryRecord endOfCentralDirectory;
    
    endOfCentralDirectory.centralDirectorySize = (uint32_t)centralDirectorySize;
    endOfCentralDirectory.diskCentralDirectoryRecordCount = filenameList.size();
    endOfCentralDirectory.centralDirectoryRecordCount = filenameList.size();
    endOfCentralDirectory.centralDirectoryOffset = (uint32_t)centralDirectoryOffset;
    
    zip.write((char*)&endOfCentralDirectory, sizeof(ZipEndOfCentralDirectoryRecord));
    
    zip.close();
    
    return 0;
}
