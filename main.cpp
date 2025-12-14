#include "raid0.h"
#include <iostream>
#include <sstream>

using namespace std;

void runCommand(RAID0& raid, const std::string& line) {
    stringstream ss(line);
    string cmd;
    ss >> cmd;

    if (cmd == "init" || cmd == "format") {
        raid.init();
    }
    else if (cmd == "write") {
        string data;
        getline(ss, data);
        if (!data.empty() && data[0] == ' ')
            data.erase(0, 1);
        raid.writeUserData(data);
    }
    else if (cmd == "read") {
        raid.readData();
    }
    else if (cmd == "inspect") {
        raid.inspectDisks();
    }
    else if (cmd == "status") {
        raid.checkArray();
    }
    else {
        std::cout << "Unknown command\n";
    }
}

int main(int argc, char* argv[]) {
    RAID0 raid(64);

    if (argc > 1) {
        std::string cmd;
        for (int i = 1; i < argc; i++) {
            cmd += argv[i];
            if (i != argc - 1)
                cmd += " ";
        }
        runCommand(raid, cmd);
        return 0;
    }

    raid.checkArray();

    cout << "\nCommands:\n  init | format\n  write <text>\n  read\n  inspect\n  status\n  exit\n";

    string line;
    while (true) {
        cout << "\nraid0> ";
        getline(std::cin, line);
        if (line == "exit") break;
        if (!line.empty()) runCommand(raid, line);
    }

    return 0;
}

