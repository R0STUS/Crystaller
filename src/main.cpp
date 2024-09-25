#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>

std::vector<std::pair<long, long>> get_process_memory_usage() {
    std::vector<std::pair<long, long>> result;
    FILE* fp = popen("ps -e --no-headers -o pid,rss", "r");
    if (fp == nullptr) {
        std::cerr << "Failed to open channel!" << std::endl;
        return result;
    }
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
        std::istringstream iss(buffer);
        long pid, rss;
        iss >> pid >> rss;
        result.push_back({pid, rss});
    }
    int status = pclose(fp);
    if (status == -1) {
        std::cerr << "Failed to close channel!" << std::endl;
    }
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) { return a.first > b.first; });
    return result;
}

int main() {
    long proccesMemNow;
    long proccesNameNow;
    std::string proccesPidNow;
    std::string command = "kill ";
    long long checkNum = 0;
    long maxMem = 2560;
    std::string tmp;
    std::ifstream settingsFile;
    settingsFile.open("MemoryMax.txt");
    if (settingsFile) {
        settingsFile >> tmp;
        maxMem = stol(tmp);
    }
    settingsFile.clear();
    tmp.clear();
    tmp.shrink_to_fit();
    std::cout << "Max. Memory for proccess: " << maxMem << std::endl;
    while (true) {
        checkNum++;
        std::cout << "Checking... [" << checkNum << "]" << std::endl;
        std::vector<std::pair<long, long>> procceses = get_process_memory_usage();
        for (int i = 0; i < procceses.size(); i++) {
            std::string command = "kill ";
            proccesNameNow = std::get<0>(procceses[i]);
            proccesMemNow = std::get<1>(procceses[i]);
            proccesPidNow = std::to_string(proccesNameNow);
            command += proccesPidNow;
            if (((proccesMemNow / 1024)) > maxMem) {
                std::cout << "Killing PID " << proccesPidNow << std::endl;
                system(command.c_str());
            }
        }
        sleep(1);
    }
}