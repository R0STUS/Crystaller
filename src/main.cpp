#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <tuple>

bool isRunningAsRoot() {
    return getuid() == 0;
}

std::vector<std::tuple<long, long, std::string>> get_process_memory_usage() {
    std::vector<std::tuple<long, long, std::string>> result;
    FILE* fp = popen("ps -e --no-headers -o pid,rss,comm", "r");
    if (fp == nullptr) {
        std::cerr << "Failed to open channel!" << std::endl;
        return result;
    }
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
        std::istringstream iss(buffer);
        long pid, rss;
        std::string comm;
        if (iss >> pid >> rss >> comm) {
            result.push_back({pid, rss, comm});
        }
    }
    pclose(fp);
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
        return std::get<1>(a) > std::get<1>(b);
    });
    return result;
}

std::string getAfterLine(std::string line) {
    bool getName = false;
    std::string str = "";
    for (char c : line) {
        if (getName == true) {
            str += c;
        }
        if (c == '=') {
            getName = true;
        }
    }
    return str;
}
bool isDigital(std::string str) {
    for (char c : str) {
        if (!std::isdigit(c)) {
            return false;
        }
    }
    return true;
}

int main() {
    if (isRunningAsRoot()) {
        std::cout << "Do NOT run this in root" << std::endl;
        return 1;
    }
    system("clear");
    long proccesMemNow;
    long proccesNameNow;
    std::string proccesNameStrNow;
    std::string proccesPidNow;
    std::string command = "kill ";
    long long checkNum = 0;
    long maxMem;
    std::string tmp;
    std::ifstream settingsFile;
    std::string sType;
    std::vector<std::string> ignoringNames;
    bool isSkipPID;
    settingsFile.open("settings.properties");
    if (!settingsFile) {
        std::cout << "Warining! Cannot open settings file, starting in default mode..." << std::endl;
        maxMem = 2560;
    }
    else {
        while (std::getline(settingsFile, tmp)) {
            sType = "";
            for (char c : tmp) {
                if (c != '=') {
                    sType += c;
                }
                else if (c == '=' && !sType.empty()) {
                    if (sType == "maxMem") {
                        sType = getAfterLine(tmp);
                        if (!sType.empty() && isDigital(sType)) {
                            maxMem = std::__cxx11::stol(sType);
                        }
                        else
                            maxMem = 2560;
                        break;
                    }
                    if (sType == "ignoreName") {
                        sType = getAfterLine(tmp);
                        if (!sType.empty()) {
                            ignoringNames.push_back(sType);
                            std::cout << sType << " ignoring." << std::endl;
                        }
                        break;
                    }
                }
                else
                    break;
            }
        }
    }
    settingsFile.clear();
    settingsFile.close();
    tmp.clear();
    tmp.shrink_to_fit();
    std::cout << "Max. Memory for proccess: " << maxMem << std::endl;
    std::string checkerStr;
    while (true) {
        checkNum++;
        std::cout << "\r" << "Checking... [" << checkNum << "]" << std::flush;
        std::vector<std::tuple<long, long, std::string>> procceses = get_process_memory_usage();
        for (int i = 0; i < procceses.size(); i++) {
            isSkipPID = false;
            std::string command = "kill ";
            proccesNameNow = std::get<0>(procceses[i]);
            proccesMemNow = std::get<1>(procceses[i]);
            proccesNameStrNow = std::get<2>(procceses[i]);
            proccesPidNow = std::to_string(proccesNameNow);
            command += proccesPidNow;
            for (auto nStr : ignoringNames) {
                if (proccesNameStrNow == nStr) {
                    isSkipPID = true;
                }
            }
            if (((proccesMemNow / 1024)) > maxMem && isSkipPID != true) {
                std::cout << std::string(checkerStr.length(), '\b');
                checkerStr.clear();
                if (proccesPidNow == "1") {
                    std::cout << "Attention! Kernel's memory is within limits! Self-killing..." << std::endl;
                    return -1;
                }
                std::cout << "Killing PID " << proccesPidNow << std::endl;
                system(command.c_str());
            }
        }
        sleep(1);
    }
}
