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
    std::ifstream patchFile;
    std::string sType;
    std::vector<std::string> ignoringNames;
    bool isSkipPID;
    std::string checkerStr = "";
    std::string ignoreStr = "";
    long maxMemD;
    bool maxMemDBool = false;
    settingsFile.open("settings.properties");
    patchFile.open("patch.properties");
    if (!patchFile) {
        std::cout << "\n    FATAL: Cannot open patch.properties." << std::endl;
        return -1;
    }
    else {
        while (std::getline(patchFile, tmp)) {
            sType = "";
            for (char c : tmp) {
                if (c != '=') {
                    sType += c;
                }
                else if (c == '=' && !sType.empty()) {
                    if (sType == "ignoring") {
                        bool getStr = false;
                        sType = "";
                        for (char c2 : tmp) {
                            if (getStr == true) {
                                sType += c2;
                            }
                            if (c2 == '=') {
                                getStr = true;
                            }
                        }
                        if (!sType.empty()) {
                            ignoreStr = sType;
                        }
                        break;
                    }
                    else if (sType == "checking") {
                        bool getStr = false;
                        sType = "";
                        for (char c2 : tmp) {
                            if (getStr == true) {
                                sType += c2;
                            }
                            if (c2 == '=') {
                                getStr = true;
                            }
                        }
                        if (!sType.empty()) {
                            checkerStr = sType;
                        }
                        break;
                    }
                    else if (sType == "maxMemD") {
                        bool getStr = false;
                        sType = "";
                        for (char c2 : tmp) {
                            if (getStr == true) {
                                sType += c2;
                            }
                            if (c2 == '=') {
                                getStr = true;
                            }
                        }
                        if (!sType.empty()) {
                            maxMemD = std::__cxx11::stol(sType);
                            maxMemDBool = true;
                        }
                        else {
                            std::cout << "\n    FATAL: 'maxMemD' cannot be empty.";
                        }
                        break;
                    }
                }
                else
                    break;
            }
        }
    }
    if (maxMemDBool == 0) {
        std::cout << "\n    FATAL: Cannot find 'maxMemD' in patch.properties." << std::endl;
        return -1;
    }
    tmp.clear();
    patchFile.clear();
    patchFile.close();
    if (!settingsFile) {
        std::cout << "Warining! Cannot open settings file, starting in default mode..." << std::endl;
        maxMem = maxMemD;
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
                        bool getLong = false;
                        sType = "";
                        for (char c2 : tmp) {
                            if (getLong == true && std::isdigit(c2)) {
                                sType += c2;
                            }
                            if (c2 == '=') {
                                getLong = true;
                            }
                        }
                        if (!sType.empty()) {
                            maxMem = std::__cxx11::stol(sType);
                        }
                        else
                            maxMem = maxMemD;
                        break;
                    }
                    else if (sType == "ignoreName") {
                        bool getName = false;
                        sType = "";
                        for (char c2 : tmp) {
                            if (getName == true) {
                                sType += c2;
                            }
                            if (c2 == '=') {
                                getName = true;
                            }
                        }
                        if (!sType.empty()) {
                            ignoringNames.push_back(sType);
                            std::cout << sType << ignoreStr << std::endl;
                        }
                        break;
                    }
                }
                else
                    break;
            }
            tmp.clear();
        }
    }
    settingsFile.clear();
    settingsFile.close();
    tmp.clear();
    tmp.shrink_to_fit();
    std::cout << "Max. Memory for proccess: " << maxMem << std::endl;
    while (true) {
        checkNum++;
        std::cout << "\r" << checkerStr << "[" << checkNum << "]" << std::flush;
        std::vector<std::tuple<long, long, std::string>> procceses = get_process_memory_usage();
        if (!procceses.empty())
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
