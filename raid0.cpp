#include "raid0.h"
#include <fstream>
#include <iostream>
#include <vector>

RAID0::RAID0(size_t stripeSize) : stripe(stripeSize) {}

bool RAID0::diskExists(const std::string& disk) {
    std::ifstream f(diskDir + disk, std::ios::binary);
    return f.good();
}

RaidStatus RAID0::checkArray() {
    int existing = 0;
    for (const auto& d : disks)
        if (diskExists(d))
            existing++;

    if (existing == 0) {
        std::cout << "NO RAID DISKS FOUND\nACTION REQUIRED: init\n";
        return RAID_NEEDS_INIT;
    }

    if (existing != (int)disks.size()) {
        std::cout << "RAID ARRAY CORRUPTED: missing disk(s)\nACTION REQUIRED: init\n";
        return RAID_CORRUPTED;
    }

    std::cout << "RAID array OK\n";
    return RAID_OK;
}

void RAID0::init() {
    // assume disks/ exists
    for (const auto& d : disks) {
        std::ofstream out(diskDir + d, std::ios::binary | std::ios::trunc);
        if (!out) {
            std::cout << "ERROR: Cannot create " << d << "\n";
            return;
        }
    }
    std::cout << "RAID array initialized (all disks empty)\n";
}

RaidStatus RAID0::writeUserData(const std::string& data) {
    if (checkArray() != RAID_OK)
        return RAID_NEEDS_INIT;

    // 1. Determine current sizes of all disks
    std::vector<size_t> diskSizes(disks.size(), 0);
    for (size_t i = 0; i < disks.size(); i++) {
        std::ifstream in(diskDir + disks[i], std::ios::binary | std::ios::ate);
            diskSizes[i] = in.tellg();
    }

    // 2. Determine starting disk index for next write
    size_t diskIndex = 0;
    // If all disks are equal size, start from first disk
    // Otherwise, start from the next disk in sequence (mod disks.size())
    size_t minSize = diskSizes[0];
    for (size_t i = 1; i < disks.size(); i++)
        if (diskSizes[i] < minSize)
            minSize = diskSizes[i];

    // Offset all disks to same logical start for striping
    for (size_t i = 0; i < disks.size(); i++)
        diskSizes[i] -= minSize;

    // Find disk to start writing (next in stripe)
    for (size_t i = 0; i < disks.size(); i++) {
        if (diskSizes[i] == 0) {
            diskIndex = i;
            break;
        }
    }

    // 3. Append new data in stripe pattern
    for (char c : data) {
        std::ofstream out(diskDir + disks[diskIndex], std::ios::binary | std::ios::app);
        if (!out) {
            std::cout << "ERROR: Cannot write to " << disks[diskIndex] << "\n";
            return RAID_CORRUPTED;
        }
        out << c;
        diskIndex = (diskIndex + 1) % disks.size();
    }

    std::cout << "Write completed\n";
    return RAID_OK;
}


RaidStatus RAID0::readData() {
    if (checkArray() != RAID_OK)
        return RAID_NEEDS_INIT;

    std::vector<std::ifstream> ins;
    for (const auto& d : disks)
        ins.emplace_back(diskDir + d, std::ios::binary);

    std::cout << "Reconstructed Data:\n";

    bool readSomething = true;
    while (readSomething) {
        readSomething = false;
        for (auto& in : ins) {
            char c;
            if (in.get(c)) {
                std::cout << c;
                readSomething = true;
            }
        }
    }

    std::cout << "\n";
    return RAID_OK;
}

void RAID0::inspectDisks() {
    for (size_t i = 0; i < disks.size(); i++) {
        std::cout << "disk" << (i + 1) << ": ";
        std::ifstream in(diskDir + disks[i], std::ios::binary);
        if (!in) {
            std::cout << "[MISSING]\n";
            continue;
        }
        std::string content((std::istreambuf_iterator<char>(in)),
                             std::istreambuf_iterator<char>());
        if (content.empty())
            std::cout << "(empty)";
        else
            std::cout << content;
        std::cout << "\n";
    }
}
