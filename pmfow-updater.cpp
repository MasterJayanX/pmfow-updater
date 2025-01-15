#define _WIN32_WINNT 0x0500
#include <iostream>
#include <string>
#include <windows.h>
#include <winver.h>
#include <fstream>
#include <map>
#include <vector>

using namespace std;

bool configExists = true;

class repo {
public:
    repo(const string& programpath) {
        // This constructor calls the loadRepos function
        loadUpdater(programpath);
    }

    string update(const string& key) const {
        auto it = packages.find(key);
        return (it != packages.end()) ? it->second : "Package not found";
    }

private:
    // This map contains the packages for each repo
    map<string, string> packages;

    void loadUpdater(const string& programpath){
        // This function loads the updater
        string fullpath = programpath + "\\", fullpath2 = programpath + "\\files\\";
        ifstream file;
        file.open(fullpath2 + "updater.dat");
        if(!file.is_open()){
            file.open(fullpath + "updater.dat");
        }
        if(!file.is_open()){
            cerr << "Error: updater.dat could not be opened.\n";
            return;
        }
        string line;
        while(getline(file, line)){
            size_t delimiterPos = line.find('=');
            if(delimiterPos != string::npos){
                string key = line.substr(0, delimiterPos);
                string value = line.substr(delimiterPos + 1);
                packages[key] = value;
            }
        }
        file.close();
    }
};

class Config {
    // This class reads the config file
public:
    Config(string filename) {
        // This function reads the config file
        ifstream file(filename);
        if(!file.is_open()) {
            cout << "Error: Could not open file " << filename << endl;
            configExists = false;
            return;
        }
        string line;
        while(getline(file, line)) {
            if(line[0] != '#' && line[0] != ';' && line[0] != '`'){
                int pos = line.find("=");
                string key = line.substr(0, pos);
                string value = line.substr(pos + 1);
                data[key] = value;
            }
        }
    }
    string get(string key) {
        // This function gets the value of a key
        return data[key];
    }
private:
    map<string, string> data;
};

string getWindowsVersion(int majorVersion, int minorVersion, int buildNumber){
    // This function returns the Windows version you are running
    string winver;
    if(majorVersion == 5 && minorVersion == 0){
        winver = "Windows 2000";
    }
    else if(majorVersion == 5 && (minorVersion == 1)){
        winver = "Windows XP";
    }
    else if(majorVersion == 5 && (minorVersion == 2)){
        winver = "Windows XP Professional x64/Windows Server 2003";
    }
    else if(majorVersion == 6 && minorVersion == 0){
        winver = "Windows Vista";
    }
    else if(majorVersion == 6 && minorVersion == 1){
        winver = "Windows 7";
    }
    else if(majorVersion == 6 && minorVersion == 2){
        winver = "Windows 8";
    }
    else if(majorVersion == 6 && minorVersion == 3){
        winver = "Windows 8.1";
    }
    else if(majorVersion == 10 && minorVersion == 0){
        winver = "Windows 10";
    }
    else{
        winver = "Other Windows version";
    }
    return winver;
}

string getArchitecture(){
    // This function returns the architecture of your Windows installation
    string arch;
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    switch(sysInfo.wProcessorArchitecture){
        case PROCESSOR_ARCHITECTURE_AMD64:
            arch = "x64";
            break;
        case PROCESSOR_ARCHITECTURE_ARM:
            arch = "arm64";
            break;
        case PROCESSOR_ARCHITECTURE_IA64:
            arch = "itanium";
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            arch = "x86";
            break;
        case PROCESSOR_ARCHITECTURE_UNKNOWN:
            arch = "Unknown";
            break;
    }
    return arch;
}

string wchar_to_string(const wchar_t* wchar){
    // This function converts a wchar_t to a string
    string str;
    for(int i = 0; wchar[i] != '\0'; i++){
        str += static_cast<char>(wchar[i]);
    }
    return str;
}

string getEXEpath(){
    // This function returns the path of pmfow-updater's executable
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    wstring::size_type pos = wstring(buffer).find_last_of(L"\\/");
    return wchar_to_string(buffer).substr(0, pos);
}

vector<string> repoDirectories(string programpath){
    vector<string> directories(2);
    ifstream file;
    string fullpath = programpath + "\\";
    string fullpath2 = programpath + "\\files\\";
    file.open(fullpath2 + "directories.txt");
    if(!file.is_open()){
        file.open(fullpath + "directories.txt");
    }
    if(file.is_open()){
        for(int i = 0; i < 2; i++){
            getline(file, directories[i]);
        }
        file.close();
    }
    else{
        cout << "Error: directories.txt was not found. The default directories will be used." << endl;
        directories[0] = "64%20bit";
        directories[1] = "32%20bit";
    }
    return directories;
}

int main(int argc, char** argv){
    OSVERSIONINFOW osv;
    osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
    osv.dwMajorVersion = 0, osv.dwMinorVersion = 0, osv.dwBuildNumber = 0;
    bool stable, lts;
    GetVersionExW(&osv);
    int majorVersion = osv.dwMajorVersion, minorVersion = osv.dwMinorVersion, build = osv.dwBuildNumber;
    string winver = getWindowsVersion(majorVersion, minorVersion, build), architecture = getArchitecture(), programpath = getEXEpath(), wget_exe, fullpath = programpath + "\\";
    string config_dir = programpath + "\\files\\config.ini";
    Config config(config_dir);

    if(config.get("winver") != "auto"){
        winver = config.get("winver");
        if(winver == "2000"){
            osv.dwMajorVersion = 5, osv.dwMinorVersion = 0, osv.dwBuildNumber = 0;
        }
        else if(winver == "xp"){
            osv.dwMajorVersion = 5, osv.dwMinorVersion = 1, osv.dwBuildNumber = 0;
        }
        else if(winver == "vista"){
            osv.dwMajorVersion = 6, osv.dwMinorVersion = 0, osv.dwBuildNumber = 0;
        }
        else if(winver == "7"){
            osv.dwMajorVersion = 6, osv.dwMinorVersion = 1, osv.dwBuildNumber = 0;
        }
        else if(winver == "8"){
            osv.dwMajorVersion = 6, osv.dwMinorVersion = 3, osv.dwBuildNumber = 0;
        }
        else if(winver == "10"){
            osv.dwMajorVersion = 10, osv.dwMinorVersion = 0, osv.dwBuildNumber = 0;
        }
        winver = getWindowsVersion(osv.dwMajorVersion, osv.dwMinorVersion, osv.dwBuildNumber);
    }
    if(config.get("wget_version") == "auto"){
        if(winver == "Windows XP" || winver == "Windows XP Professional x64/Windows Server 2003"){
            wget_exe = "wget_xp";
        }
        else if(winver == "Windows 2000"){
            wget_exe = "wget_2k";
        }
        else{
            wget_exe = "wget";
        }
    }
    if(config.get("architecture") != "auto"){
        architecture = config.get("architecture");
    }
    if(config.get("check_lts") == "true"){
        lts = true;
    }
    else if(config.get("check_lts") == "false"){
        lts = false;
    }
    
    vector<string> directories = repoDirectories(programpath);
    if(osv.dwMajorVersion >= 5){
        // Windows 2000 or later
        repo r(programpath);
        string package;
        stable = true, lts = false;
        if(argc > 1){
            if(string(argv[1]) == "-u" || string(argv[1]) == "--unstable"){
                stable = false;
            }
            else if(string(argv[1]) == "-l" || string(argv[1]) == "--lts"){
                lts = true;
            }
        }
        if(architecture == "x64"){
            if(stable){
                package = r.update("pmfow64");
            }
            else if(lts){
                package = r.update("pmfow-lts64");
            }
            else{
                package = r.update("pmfow-unstable64");
            }
        }
        else if(architecture == "x86"){
            if(stable){
                package = r.update("pmfow32");
            }
            else if(lts){
                package = r.update("pmfow-lts32");
            }
            else{
                package = r.update("pmfow-unstable32");
            }
        }
        else{
            cout << "Your architecture is not supported.\n";
            return 0;
        }
        system(("del " + fullpath + "pmfow.exe").c_str());
        system((wget_exe + " -O " + fullpath + "pmfow.exe " + package + " -q --show-progress").c_str());
    }
    else{
        // Windows 98 or earlier
        cout << "Your Windows version is not supported.\n";
    }
    return 0;
}