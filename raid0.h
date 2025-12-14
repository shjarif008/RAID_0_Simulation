#ifndef RAID0_H
#define RAID0_H

#include <vector>
#include <string>

using namespace std;

enum RaidStatus {
    RAID_OK,
    RAID_NEEDS_INIT,
    RAID_CORRUPTED
};

class RAID0 {
public:
    RAID0(size_t stripeSize);

    RaidStatus checkArray();
    void init();
    RaidStatus writeUserData(const std::string& data);
    RaidStatus readData();
    void inspectDisks();

private:
    const string diskDir = "disks/";
    const vector<string> disks = {
        "disk1.bin", "disk2.bin", "disk3.bin"
    };

    size_t stripe;
    bool diskExists(const string& disk);
};

#endif

