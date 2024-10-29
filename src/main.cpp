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

std::tuple<std::string, char, std::string> getVar(std::string tmp) {
    bool getName = true;
    bool getType = false;
    bool getValue = false;
    std::string name;
    char type;
    std::string value;
    std::string sType = "";
    for (char c2 : tmp) {
        if (c2 == ';') {
            if (getName == true) {
                name = sType;
                sType = "";
                getName = false;
                getType = true;
            }
            else if (getType == true) {
                for (char c : sType) {
                    type = c;
                    break;
                }
                sType = "";
                getType = false;
                getValue = true;
            }
            else if (getValue == true) {
                value = sType;
                sType = "";
                getValue = false;
            }
        }
        if (getName == true) {
            sType += c2;
        }
        else if (getType == true) {
            sType += c2;
        }
        else if (getValue == true) {
            sType += c2;
        }
    }
    return {name, type, value};
}

struct varBool {
  std::string name;
  bool value;
};
struct varLong {
  std::string name;
  long value;
};
struct varStr {
  std::string name;
  std::string value;
};

int main() {
    std::ofstream logsFile;
    logsFile.open("latest.log");
    if (isRunningAsRoot()) {
        std::cout << "Do NOT run this in root" << std::endl;
        logsFile << "Do NOT run this in root" << std::endl;
        return 1;
    }
    system("clear");
    // BUILD VERSION
    std::string build = "$1.patch-support";
    // $ = Preview; # = Release;
    // after '.' this is the name of the branch
    std::cout << " BUILD: " << build << std::endl;
    logsFile << " BUILD: " << build << std::endl;
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
    std::string patch;
    bool patchBool;
    std::vector<varStr> strings;
    std::vector<varLong> longs;
    std::vector<varBool> bools;
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
                        sType = getNext(tmp);
                        if (!sType.empty()) {
                            killedProccessesPatch = sType;
                        }
                        break;
                    }
                    else if (sType == "var") {
                        sType = getNext(tmp);
                        if (!sType.empty()) {
                            std::tuple<std::string, char, std::string> temp = getVar(sType);
                            std::cout << "From patch [Loading " << std::get<0>(temp) << "...]" << std::endl;
                            logsFile << "From patch [Loading " << std::get<0>(temp) << "...]" << std::endl;
                            if (std::get<1>(temp) == 'l') {
                              longs.push_back({std::get<0>(temp), std::__cxx11::stol(std::get<2>(temp))});
                            }
                            else if (std::get<1>(temp) == 's') {
                              strings.push_back({std::get<0>(temp), std::get<2>(temp)});
                            }
                            else if (std::get<1>(temp) == 'b') {
                              bool booled;
                              std::istringstream(std::get<2>(temp)) >> booled;
                              bools.push_back({std::get<0>(temp), booled});
                            }
                        }
                        break;
                    }
                    else if (sType == "patch") {
                        sType = getNext(tmp);
                        if (!sType.empty()) {
                            std::cout << " Patch: " << sType << std::endl;
                            logsFile << " Patch: " << sType << std::endl;
                            patch = sType;
                            patchBool = true;
                        }
                        else {
                            std::cout << "\n    FATAL: 'patch' cannot be empty." << std::endl;
                            logsFile << "\n    FATAL: 'patch' cannot be empty." << std::endl;
                            return -1;
                        }
                        break;
                    }
                }
                else
                    break;
            }
        }
    }
    if (maxMemDBool == 0 || settingsFilenameBool == 0 || patchBool == 0) {
        if (maxMemDBool == 0) {
            std::cout << "\n    FATAL: Cannot find 'maxMemD' in 'patch.properties'" << std::endl;
            logsFile << "\n    FATAL: Cannot find 'maxMemD' in 'patch.properties'" << std::endl;
        }
        if (settingsFilenameBool == 0) {
            std::cout << "\n    FATAL: Cannot find 'settingsFilename' in 'patch.properties'" << std::endl;
            logsFile << "\n    FATAL: Cannot find 'settingsFilename' in 'patch.properties'" << std::endl;
        }
        if (patchBool == 0) {
            std::cout << "\n    FATAL: Cannot find 'patch' in 'patch.properties'" << std::endl;
            logsFile << "\n    FATAL: Cannot find 'patch' in 'patch.properties'" << std::endl;
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
        std::cout << " BUILD: " << build << std::endl;
        logsFile << " BUILD: " << build << std::endl;
        std::cout << " Patch: " << patch << std::endl;
        logsFile << " Patch: " << patch << std::endl;
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
