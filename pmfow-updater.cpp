#define _WIN32_WINNT 0x0500
#include <iostream>
#include <string>
#include <windows.h>
#include <winver.h>
#include <fstream>
#include <map>
#include <vector>

using namespace std;

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

string getEXEpath(){
    // This function returns the path of pmfow's executable
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    string::size_type pos = string(buffer).find_last_of("\\/");
    return string(buffer).substr(0, pos);
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
    GetVersionExW(&osv);
    int majorVersion = osv.dwMajorVersion, minorVersion = osv.dwMinorVersion, build = osv.dwBuildNumber;
    string winver = getWindowsVersion(majorVersion, minorVersion, build), architecture = getArchitecture(), programpath = getEXEpath(), wget_exe, fullpath = programpath + "\\";
    if(winver == "Windows XP" || winver == "Windows XP Professional x64/Windows Server 2003"){
        wget_exe = "wget_xp";
    }
    else if(winver == "Windows 2000"){
        wget_exe = "wget_2k";
    }
    else{
        wget_exe = "wget";
    }
    vector<string> directories = repoDirectories(programpath);
    if(osv.dwMajorVersion >= 5){
        // Windows 2000 or later
        repo r(programpath);
        string package;
        bool stable = true;
        if(argc > 1){
            if(string(argv[1]) == "-u" || string(argv[1]) == "--unstable"){
                stable = false;
            }
        }
        if(architecture == "x64"){
            if(stable){
                package = r.update("pmfow64");
            }
            else{
                package = r.update("pmfow-unstable64");
            }
        }
        else if(architecture == "x86"){
            if(stable){
                package = r.update("pmfow32");
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
        system((wget_exe + " -O " + fullpath + "pmfow.exe " + package).c_str());
    }
    else{
        // Windows 98 or earlier
        cout << "Your Windows version is not supported.\n";
    }
    return 0;
}