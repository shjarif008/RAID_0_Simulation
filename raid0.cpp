#include "raid0.h"
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

RAID0::RAID0(size_t stripeSize) : stripe(stripeSize) {}

bool RAID0::diskExists(const std::string& disk) {
    ifstream f(diskDir + disk, ios::binary);
    return f.good();
}

RaidStatus RAID0::checkArray() {
    int existing = 0;
    for (const auto& d : disks)
        if (diskExists(d))
            existing++;

    if (existing == 0) {
        cout << "NO RAID DISKS FOUND\nACTION REQUIRED: init\n";
        return RAID_NEEDS_INIT;
    }

    if (existing != (int)disks.size()) {
        cout << "RAID ARRAY CORRUPTED: missing disk(s)\nACTION REQUIRED: init\n";
        return RAID_CORRUPTED;
    }

    cout << "RAID array OK\n";
    return RAID_OK;
}

void RAID0::init() {
    // assume disks/ exists
    for (const auto& d : disks) {
        ofstream out(diskDir + d, ios::binary | ios::trunc);
        if (!out) {
            cout << "ERROR: Cannot create " << d << "\n";
            return;
        }
    }
    cout << "RAID array initialized (all disks empty)\n";
}

RaidStatus RAID0::writeUserData(const std::string& data) {
    if (checkArray() != RAID_OK)
        return RAID_NEEDS_INIT;

    vector<size_t> diskSizes(disks.size(), 0);
    for (size_t i = 0; i < disks.size(); i++) {
        ifstream in(diskDir + disks[i], ios::binary | ios::ate);
            diskSizes[i] = in.tellg();
    }

    size_t diskIndex = 0;
    size_t minSize = diskSizes[0];
    for (size_t i = 1; i < disks.size(); i++)
        if (diskSizes[i] < minSize)
            minSize = diskSizes[i];

    for (size_t i = 0; i < disks.size(); i++)
        diskSizes[i] -= minSize;

    for (size_t i = 0; i < disks.size(); i++) {
        if (diskSizes[i] == 0) {
            diskIndex = i;
            break;
        }
    }

    for (char c : data) {
        ofstream out(diskDir + disks[diskIndex], ios::binary | ios::app);
        if (!out) {
            cout << "ERROR: Cannot write to " << disks[diskIndex] << "\n";
            return RAID_CORRUPTED;
        }
        out << c;
        diskIndex = (diskIndex + 1) % disks.size();
    }

    cout << "Write completed\n";
    return RAID_OK;
}


RaidStatus RAID0::readData() {
    if (checkArray() != RAID_OK)
        return RAID_NEEDS_INIT;

    vector<std::ifstream> ins;
    for (const auto& d : disks)
        ins.emplace_back(diskDir + d, std::ios::binary);

    cout << "Reconstructed Data:\n";

    bool readSomething = true;
    while (readSomething) {
        readSomething = false;
        for (auto& in : ins) {
            char c;
            if (in.get(c)) {
                cout << c;
                readSomething = true;
            }
        }
    }

    cout << "\n";
    return RAID_OK;
}

void RAID0::inspectDisks() {
    for (size_t i = 0; i < disks.size(); i++) {
        cout << "disk" << (i + 1) << ": ";
        ifstream in(diskDir + disks[i], ios::binary);
        if (!in) {
            cout << "[MISSING]\n";
            continue;
        }
        string content((istreambuf_iterator<char>(in)),
                             istreambuf_iterator<char>());
        if (content.empty())
            cout << "(empty)";
        else
            cout << content;
        cout << "\n";
    }
}

