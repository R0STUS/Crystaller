#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fstream>
#include <tuple>
#include <cstdio>
#include <dirent.h>
#include <thread>

bool isRunningAsRoot() {
    return getuid() == 0;
}

const int BUFF_MAX_SIZE = 1024;
std::string logsStr = "";

void saveLogs() {
    std::ofstream file("latest.log");
    if (!file)
        printf("Failed to open logs ofstream file!\n");
    else {
        file << logsStr;
        logsStr = "";
        printf("Logs saved and logs buffer reset.\n");
    }
    file.close();
}

void handle_sigint(int sig) {
    printf("\nSIGINT received! Saving logs...\n");
    saveLogs();
    exit(0);
}

std::vector<std::tuple<long, long, float, std::string, std::string>> get_process_memory_usage(std::string patchCommandChecker) {
    std::vector<std::tuple<long, long, float, std::string, std::string>> result;
    FILE* fp = popen(patchCommandChecker.c_str(), "r");
    if (fp == nullptr) {
        std::cerr << "Failed to open channel!" << std::endl;
        return result;
    }
    char buffer[512];
    while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
        std::istringstream iss(buffer);
        long pid, rss;
        float pcpu;
        std::string comm, user;
        if (iss >> pid >> rss >> pcpu >> comm >> user) {
            result.push_back({pid, rss, pcpu, comm, user});
        }
    }
    pclose(fp);
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
        return std::get<1>(a) > std::get<1>(b);
    });
    return result;
}

long getCores() {
    long nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    if (nprocs < 1) {
        std::cerr << "Error: Unable to get the number of processors!" << std::endl;
        return -1;
    }
    return nprocs;
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

struct suspid {
    long pid;
    long counter;
    long total;
};

bool isDigital(std::string str) {
    for (char c : str) {
        if (!std::isdigit(c)) {
            return false;
        }
    }
    return true;
}

void doNotRunInRoot() {
    std::ofstream logsFile;
    logsFile.open("latest.log");
    while (true) {
        sleep(1);
        if (isRunningAsRoot()) {
            std::cout << "Do NOT run this in root" << std::endl;
            logsFile << "Do NOT run this in root" << std::endl;
            exit(1);
        }
    }
    logsFile.close();
}

int main() {
    signal(SIGINT, handle_sigint);
    if (isRunningAsRoot()) {
        std::cout << "Do NOT run this in root" << std::endl;
        logsStr += "Do NOT run this in root\n";
        return 1;
    }
    std::thread doNotRunInRootPls(doNotRunInRoot);
    system("clear");
    // BUILD VERSION
    std::string build = "#4.main";
    // $ = Preview; # = Release;
    // after '.' is the name of the branch
    std::cout << " BUILD: " << build << std::endl;
    logsStr += " BUILD: " + build + "\n";
    long cores = getCores();
    std::cout << " Total Proc. Cores: [" << cores << ']' << std::endl;
    logsStr += " Total Proc. Cores: [" + std::to_string(cores) + "]\n";
    long proccesMemNow;
    double sleepTime = 1;
    double sleepBeforeTime = 1;
    std::string patchCommandChecker;
    bool patchCommandCheckerBool = false;
    long proccesNameNow;
    float maxCPUusageD;
    bool maxCPUusageDbool;
    float maxCPUusage;
    float proccesCPUNow;
    std::string proccesUserNow;
    std::string typeOfBadStr;
    short typeOfBad;
    bool cpuon = false;
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
    std::vector<std::string> ignoringNamesCPU = {"ps", "wl-paste", "wl-copy", "chrome_crashpad", "sway", "hypr", "hypridle", "sleep", "dbus-broker-lau", "swaync-client", "playerctld", "login", "bluetoothd", "cpuUsage.sh", "Hyprland", "kernel", "waybar", "electron", "Xwayland", "wayland", "Xorg", "pypr", "pipewire", "init", "systemd", "udevd", "dbus-daemon", "dbus-broker", "gnome-shell", "kdeinit5", "lightdm", "sddm", "xmonad", "compiz", "xfwm4", "kwin_x11", "metacity", "openbox", "cinnamon-sess", "mutter", "lxsession", "xfce4-session", "pulseaudio", "gdm", "slack", "nautilus", "thunar", "dconf-service", "gsettings", "pidgin", "ssh-agent", "gnome-settings-daemon", "xfsettingsd", "nm-applet", "blueman-applet", "clipman", "lxpolkit", "xfce4-panel", "mate-settings-daemon", "mate-panel", "polkitd", "rsyslogd", "cron", "atd", "systemd-journald", "systemd-logind", "systemd-udevd", "rtkit-daemon", "colord", "cupsd", "lightdm-gtk-greeter", "xset", "xmodmap", "firewalld", "NetworkManager", "ntpd", "systemd-timesyncd", "acpid", "apt-daily-service", "snapd", "gimmik", "gnome-terminal-server", "kbd", "Xvnc", "vmtoolsd", "vmware-tools", "udisksd", "gnome-keyring-daemon", "docker", "httpd", "mysqld", "postgres", "ssh", "rsync", "mdadm", "fsck", "mount", "umount", "mnt", "parted", "fstab", "plymouth", "pstree", "kmod", "journalctl", "syslog-ng", "networkd-dispatcher", "anacron", "logrotate", "user@1000.service", "agetty", "plymouth-start", "systemd-sysctl", "systemd-suspend", "systemd-hibernate"};
    std::vector<std::string> killedProccesses;
    bool isSkipPID;
    bool isSkipPIDCPU;
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
    std::vector<suspid> suspids;
    patchFile.open("patch.properties");
    if (!patchFile) {
        std::cout << "\n    FATAL: Cannot open patch.properties." << std::endl;
        logsStr += "\n    FATAL: cannot open patch.properties.\n";
        saveLogs();
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
                            logsStr += "\n    FATAL: 'maxMemD' cannot be empty.";
                        }
                        break;
                    }
                    else if (sType == "maxCPUusageD") {
                        sType = getNext(tmp);
                        if (!sType.empty()) {
                            maxCPUusageD = std::__cxx11::stol(sType);
                            maxCPUusageDbool = true;
                        }
                        else {
                            std::cout << "\n    FATAL: 'maxCPUusageD' cannot be empty.";
                            logsStr += "\n    FATAL: 'maxCPUusageD' cannot be empty.";
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
                            logsStr += "\n    FATAL: 'settingsFilename' cannot be empty.";
                        }
                        break;
                    }
                    else if (sType == "patchCommandChecker") {
                        sType = getNext(tmp);
                        if (!sType.empty()) {
                            patchCommandChecker = sType;
                            patchCommandCheckerBool = true;
                        }
                        else {
                            std::cout << "\n    FATAL: 'patchCommandChecker' cannot be empty.";
                            logsStr += "\n    FATAL: 'patchCommandChecker' cannot be empty.";
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
                            std::cout << "From patch [Loading '" << std::get<0>(temp) << "'...]" << std::endl;
                            logsStr += "From patch [Loading '" + std::get<0>(temp) + "'...]\n";
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
                            logsStr += " Patch: " + sType + "\n";
                            patch = sType;
                            patchBool = true;
                        }
                        else {
                            std::cout << "\n    FATAL: 'patch' cannot be empty." << std::endl;
                            logsStr += "\n    FATAL: 'patch' cannot be empty.\n";
                            saveLogs();
                            return -1;
                        }
                        break;
                    }
                    else if (sType == "cpuon") {
                        sType = getNext(tmp);
                        if (!sType.empty()) {
                            std::istringstream(sType) >> cpuon;
                        }
                        else
                            break;
                    }
                }
                else
                    break;
            }
        }
    }
    if (maxMemDBool == 0 || settingsFilenameBool == 0 || patchBool == 0 || maxCPUusageDbool == 0 || patchCommandCheckerBool == 0) {
        if (maxMemDBool == 0) {
            std::cout << "\n    FATAL: Cannot find 'maxMemD' in 'patch.properties'" << std::endl;
            logsStr += "\n    FATAL: Cannot find 'maxMemD' in 'patch.properties'\n";
        }
        if (settingsFilenameBool == 0) {
            std::cout << "\n    FATAL: Cannot find 'settingsFilename' in 'patch.properties'" << std::endl;
            logsStr += "\n    FATAL: Cannot find 'settingsFilename' in 'patch.properties'\n";
        }
        if (patchBool == 0) {
            std::cout << "\n    FATAL: Cannot find 'patch' in 'patch.properties'" << std::endl;
            logsStr += "\n    FATAL: Cannot find 'patch' in 'patch.properties'\n";
        }
        if (maxCPUusageDbool == 0) {
            std::cout << "\n    FATAL: Cannot find 'maxCPUusageD' in 'patch.properties'" << std::endl;
            logsStr += "\n    FATAL: Cannot find 'maxCPUusageD' in 'patch.properties'\n";
        }
        if (patchCommandCheckerBool == 0) {
            std::cout << "\n    FATAL: Cannot find 'patchCommandChecker' in 'patch.properties'" << std::endl;
            logsStr += "\n    FATAL: Cannot find 'patchCommandChecker' in 'patch.properties'\n";
        }
        saveLogs();
        return -1;
    }

    maxCPUusage = maxCPUusageD;
    
    tmp.clear();
    patchFile.clear();
    patchFile.close();
    settingsFile.open(settingsFilename);
    if (!settingsFile) {
        std::cout << "Warining! Cannot open settings file, starting in default mode..." << std::endl;
        logsStr += "Warning! Cannot open settings file, starting in default mode...\n";
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
                        if (!sType.empty() && isDigital(sType)) {
                            maxMem = std::__cxx11::stol(sType);
                            maxMemBool = true;
                        }
                        else
                            maxMem = maxMemD;
                        break;
                    }
                    else if (sType == "maxCPUusage") {
                        sType = getNext(tmp);
                        if (!sType.empty()) {
                            maxCPUusage = std::__cxx11::stof(sType);
                        }
                        else
                            maxCPUusage = maxCPUusageD;
                        break;
                    }
                    else if (sType == "cpuon") {
                        sType = getNext(tmp);
                        if (!sType.empty()) {
                            std::istringstream(sType) >> cpuon;
                        }
                        else
                        break;
                    }
                    else if (sType == "ignoreName") {
                        sType = getNext(tmp);
                        if (!sType.empty()) {
                            ignoringNames.push_back(sType);
                            std::cout << "From config [" << sType << ignoreStr << ']' << std::endl;
                            logsStr += "From config [" + sType + ignoreStr + "]\n";
                        }
                        break;
                    }
                    else if (sType == "ignoreNameCPU") {
                        sType = getNext(tmp);
                        if (!sType.empty()) {
                            ignoringNamesCPU.push_back(sType);
                            std::cout << "From config [" << sType << ignoreStr << " {CPU} " << ']' << std::endl;
                            logsStr += "From config [" + sType + ignoreStr + " {CPU} ]\n";
                        }
                        break;
                    }
                    else if (sType == "sleepBeforeTime") {
                        sType = getNext(tmp);
                        if (!sType.empty()) {
                            sleepBeforeTime = std::__cxx11::stof(sType);
                        }
                        break;
                    }
                    else if (sType == "sleepTime") {
                        sType = getNext(tmp);
                        if (!sType.empty()) {
                            sleepTime = std::__cxx11::stof(sType);
                            std::cout << sleepTime << std::endl;
                            logsStr += std::to_string(sleepTime) + "\n";
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
    logsStr += "Max. Memory for proccess: " + std::to_string(maxMem) + "\n";
    std::cout << "Max. CPU usage for proccess: " << maxCPUusage << " [CPU check = " << cpuon << "] " << std::endl;
    logsStr += "Max. CPU usage for proccess: " + std::to_string(maxCPUusage) + " [CPU check = " + std::to_string(cpuon) + "] \n";
    if (cores == -1 && cpuon) {
        std::cout << "\nUnknown number of CPU cores." << std::endl;
        logsStr += "\nUnknown number of CPU cores.\n";
        saveLogs();
        return -1;
    }
    usleep(static_cast<int>(sleepBeforeTime * 1000000));
    if (sleepTime <= 0) {
        sleepTime = 0.01;
    }
    while (true) {
        system("clear");
        std::cout << " BUILD: " << build << std::endl;
        logsStr += " BUILD: " + build + "\n";
        std::cout << " Patch: " << patch << std::endl;
        logsStr += " Patch: " + patch + "\n";
        checkNum++;
        std::cout << checkerStr << "[" << checkNum << "]" << std::endl;
        logsStr += checkerStr + "[" + std::to_string(checkNum) + "]\n";
        std::cout << killedProccessesPatch << "{" << std::endl;
        logsStr += killedProccessesPatch + "{\n";
        for (auto sStr : killedProccesses) {
          std::cout << sStr << std::endl;
          logsStr += sStr + "\n";
        }
        std::cout << '}' << std::endl;
        logsStr += "}\n";
        std::vector<std::tuple<long, long, float, std::string, std::string>> procceses = get_process_memory_usage(patchCommandChecker);
        if (!procceses.empty()) {
            for (int i = 0; i < procceses.size(); i++) {
                isSkipPID = false;
                isSkipPIDCPU = false;
                std::string command = "kill ";
                proccesNameNow = std::get<0>(procceses[i]);
                proccesMemNow = std::get<1>(procceses[i]);
                proccesCPUNow = std::get<2>(procceses[i]);
                // std::cout << "Proccess CPU: " << proccesCPUNow << std::endl; // Debug
                proccesNameStrNow = std::get<3>(procceses[i]);
                proccesUserNow = std::get<4>(procceses[i]);
                proccesPidNow = std::to_string(proccesNameNow);
                // std::cout << "PID: " << proccesPidNow << " Mem: " << proccesMemNow << std::endl; // Debug
                command += proccesPidNow;
                for (auto nStr : ignoringNames) {
                    if (proccesNameStrNow == nStr) {
                        isSkipPID = true;
                    }
                }
                for (auto nStr : ignoringNamesCPU) {
                    if (proccesNameStrNow == nStr) {
                        isSkipPIDCPU = true;
                    }
                }
                if ((proccesMemNow / 1024) > maxMem) {
                    typeOfBad = 2;
                }
                bool isbad = false;
                bool isfound = false;
                bool isdelete = false;
                for (auto nStr : suspids) {
                    isfound = true;
                    if (proccesCPUNow > (maxCPUusage * cores) && cpuon == true && !isSkipPIDCPU) {
                        if (proccesNameNow == nStr.pid && nStr.counter > nStr.total) {
                            isbad = true;
                        }
                        else if (proccesNameNow == nStr.pid) {
                            nStr.counter++;
                        }
                    }
                    else
                        isdelete = true;
                }
                if (isbad) {
                    typeOfBad++;
                }
                if (!isfound) {
                    suspids.push_back({proccesNameNow, 0, (long)(5.0 / sleepTime)});
                }
                if (isfound && isdelete) {
                    suspids.erase(std::remove_if(suspids.begin(), suspids.end(), [&proccesNameNow](const suspid& o) { return o.pid == proccesNameNow; }), suspids.end());
                }
                switch (typeOfBad) {
                    case 1:
                        typeOfBadStr = " CPU Usage ";
                    break;
                    case 2:
                        typeOfBadStr = " Memory Usage ";
                    break;
                    case 3:
                        typeOfBadStr = " CPU and Memory Usage ";
                    break;
                    default:
                        typeOfBadStr = " nothing ";
                    break;
                }
                if (typeOfBad > 0 && isSkipPID != true && proccesUserNow != "root") {
                    killedProccesses.push_back(" [" + proccesPidNow + "] '" + proccesNameStrNow + "'" + " reason [" + typeOfBadStr + "]");
                    if (proccesPidNow == "1") {
                        std::cout << "\nAttention! Kernel's " << typeOfBadStr << " is within limits! Self-killing..." << std::endl;
                        logsStr += "\nKERNEL\n";
                        saveLogs();
                        return -1;
                    }
                    std::cout << "Killing PID " << proccesPidNow << " [" << proccesNameStrNow << "] for reason [" << typeOfBadStr << ']' << std::endl;
                    logsStr += "Killing PID " + proccesPidNow + " [" + proccesNameStrNow + "] for reason [" + typeOfBadStr + "]\n";
                    system(command.c_str());
                    typeOfBad = 0;
                }
            }
        }
        else
            std::cout << "Can't read any process!" << std::endl;
        logsStr += "\n\n";
        if (logsStr.size() >= BUFF_MAX_SIZE)
            saveLogs();
        usleep(static_cast<int>(sleepTime * 1000000));
    }
}
