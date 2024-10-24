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

std::string getNext(std::string tmp) {
    bool getStr = false;
    std::string sType = "";
    for (char c2 : tmp) {
        if (getStr == true) {
            sType += c2;
        }
        if (c2 == '=') {
            getStr = true;
        }
    }
    return sType;
}

int main() {
    std::ofstream logsFile;
    logsFile.open("latest.log");
    if (isRunningAsRoot()) {
        std::cout << "Do NOT run this in root" << std::endl;
        logsFile << "Do NOT run this in root" << std::endl;
        return 1;
    }
    system("clear");
    long proccesMemNow;
    double sleepTime = 1;
    long proccesNameNow;
    std::string proccesNameStrNow;
    std::string proccesPidNow;
    std::string command = "kill ";
    long long checkNum = 0;
    long maxMem;
    bool maxMemBool = false;
    std::string tmp;
    std::ifstream settingsFile;
    std::ifstream patchFile;
    std::string sType;
    std::vector<std::string> ignoringNames;
    std::vector<std::string> killedProccesses;
    bool isSkipPID;
    std::string checkerStr = "";
    std::string ignoreStr = "";
    std::string killedProccessesPatch = "";
    long maxMemD;
    bool maxMemDBool = false;
    std::string settingsFilename;
    bool settingsFilenameBool = false;
    patchFile.open("patch.properties");
    if (!patchFile) {
        std::cout << "\n    FATAL: Cannot open patch.properties." << std::endl;
        logsFile << "\n    FATAL: cannot open patch.properties." << std::endl;
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
                        sType = getNext(tmp);
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
                        sType = getNext(tmp);
                        if (!sType.empty()) {
                            maxMemD = std::__cxx11::stol(sType);
                            maxMemDBool = true;
                        }
                        else {
                            std::cout << "\n    FATAL: 'maxMemD' cannot be empty.";
                            logsFile << "\n    FATAL: 'maxMemD' cannot be empty.";
                        }
                        break;
                    }
                    else if (sType == "settingsFilename") {
                        sType = getNext(tmp);
                        if (!sType.empty()) {
                            settingsFilename = sType;
                            settingsFilenameBool = true;
                        }
                        else {
                            std::cout << "\n    FATAL: 'settingsFilename' cannot be empty.";
                            logsFile << "\n    FATAL: 'settingsFilename' cannot be empty.";
                        }
                        break;
                    }
                    else if (sType == "killedProccessesPatch") {
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
                            killedProccessesPatch = sType;
                        }
                        break;
                    }
                }
                else
                    break;
            }
        }
    }
    if (maxMemDBool == 0 || settingsFilenameBool == 0) {
        if (maxMemDBool == 0) {
            std::cout << "\n    FATAL: Cannot find 'maxMemD' in 'patch.properties'" << std::endl;
            logsFile << "\n    FATAL: Cannot find 'maxMemD' in 'patch.properties'" << std::endl;
        }
        if (settingsFilenameBool == 0) {
            std::cout << "\n    FATAL: Cannot find 'settingsFilename' in 'patch.properties'" << std::endl;
            logsFile << "\n    FATAL: Cannot find 'settingsFilename' in 'patch.properties'" << std::endl;
        }
        return -1;
    }
    tmp.clear();
    patchFile.clear();
    patchFile.close();
    settingsFile.open(settingsFilename);
    if (!settingsFile) {
        std::cout << "Warining! Cannot open settings file, starting in default mode..." << std::endl;
        logsFile << "Warning! Cannot open settings file, starting in default mode..." << std::endl;
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
                        sType = getNext(tmp);
                        if (!sType.empty()) {
                            maxMem = std::__cxx11::stol(sType);
                            maxMemBool = true;
                        }
                        else
                            maxMem = maxMemD;
                        break;
                    }
                    else if (sType == "ignoreName") {
                        sType = getNext(tmp);
                        if (!sType.empty()) {
                            ignoringNames.push_back(sType);
                            std::cout << "From config [" << sType << ignoreStr << ']' << std::endl;
                        }
                        break;
                    }
                    else if (sType == "sleepTime") {
                        sType = getNext(tmp);
                        if (!sType.empty()) {
                            sleepTime = std::__cxx11::stod(sType);
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
    if (maxMemBool == 0)
        maxMem = maxMemD;
    std::cout << "Max. Memory for proccess: " << maxMem << std::endl;
    logsFile << "Max. Memory for proccess: " << maxMem << std::endl;
    sleep(sleepTime);
    logsFile.close();
    while (true) {
        logsFile.open("latest.log");
        system("clear");
        checkNum++;
        std::cout << checkerStr << "[" << checkNum << "]" << std::endl;
        logsFile << checkerStr << "[" << checkNum << "]" << std::endl;
        std::cout << killedProccessesPatch << "{" << std::endl;
        logsFile << killedProccessesPatch << "{" << std::endl;
        for (auto sStr : killedProccesses) {
          std::cout << sStr << std::endl;
          logsFile << sStr << std::endl;
        }
        std::cout << '}' << std::endl;
        logsFile << '}' << std::endl;
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
                    killedProccesses.push_back(" [" + proccesPidNow + "] '" + proccesNameStrNow + "'");
                    if (proccesPidNow == "1") {
                        std::cout << "\nAttention! Kernel's memory is within limits! Self-killing..." << std::endl;
                        logsFile << "\nKERNEL" << std::endl;
                        return -1;
                    }
                    std::cout << "Killing PID " << proccesPidNow << " [" << proccesNameStrNow << ']' << std::endl;
                    system(command.c_str());
                }
            }
        logsFile.close();
        sleep(1);
    }
}
