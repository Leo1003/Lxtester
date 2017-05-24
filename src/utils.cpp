#include "utils.h"
#include "global.h"
using namespace std;

string getSelfPath() {
    char buf[5001];
    readlink("/proc/self/exe", buf, 5000);
    string out(buf);
    return out;
}

string getConfDir() {
    string path = getSelfPath();
    regex reg("\\/([^\\/]+)");
    smatch sm;
    vector<string> dirlv;
    while (regex_search(path, sm, reg)) {
        dirlv.push_back(sm[1]);
        path = sm.suffix().str();
    }
    for (int i = dirlv.size() - 1; i >= 0; i--) {
        dirlv.pop_back();
        string dir;
        stringstream ss;
        for (int j = 0; j < dirlv.size(); j++)
            ss << "/" << dirlv[j];
        dir = ss.str();
        ss << "/lxtester.conf";
        struct stat st;
        if (stat(ss.str().c_str(), &st) != -1 && st.st_mode & S_IFREG) {
            mainlg.log("Found config file: " + ss.str(), LVDE);
            return dir;
        }
    }
    mainlg.log("Config file not found", LVER);
    return "";
}

bool isExec(string path) {
    struct stat st;
    return (stat(path.c_str(), &st) == 0 &&
        S_ISREG(st.st_mode) &&
        st.st_mode & S_IREAD &&
        st.st_mode & S_IEXEC );
}

bool isFile(string path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

bool isDir(string path) {
    struct stat st;
    return (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode));
}

int tryParse(string str, int def) {
    try {
        if (str == "")
            return def;
        return stoi(str);
    } catch (invalid_argument ex) {
        mainlg.log("Parsing string error: " + str, LVER);
        mainlg.log(ex.what());
        return def;
    }
}

long long tryParsell(string str, long long def) {
    try {
        if (str == "")
            return def;
        return stoll(str);
    } catch (invalid_argument ex) {
        mainlg.log("Parsing string error: " + str, LVER);
        mainlg.log(ex.what());
        return def;
    }
}

double tryParsed(string str, double def) {
    try {
        if (str == "")
            return def;
        return stod(str);
    } catch (invalid_argument ex) {
        mainlg.log("Parsing string error: " + str, LVER);
        mainlg.log(ex.what());
        return def;
    }
}

loglevel tryParseLevel(string str) {
    if (str == "Fatal")
        return LVFA;
    else if (str == "Error")
        return LVER;
    else if (str == "Warn")
        return LVWA;
    else if (str == "Info")
        return LVIN;
    else if (str == "Debug")
        return LVDE;
    else if (str == "Debug2")
        return LVD2;
    else
        return (loglevel)-1;
}

string delimstring(string& str, char delim)
{
    stringstream ss(str);
    string ret;
    getline(ss, ret, delim);
    str = ss.str().substr(ret.size());
    if (str.size() > 0)
        str = str.substr(1);
    return ret;
}

string trim(const string& str) {
    size_t first = str.find_first_not_of(' ');
    if (string::npos == first)
        return str;
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

char** parseVecstr(vector<string> vec) {
    char** cstrings = new char*[vec.size() + 1];
    for (size_t i = 0; i < vec.size(); ++i) {
        cstrings[i] = new char[vec[i].size() + 1];
        std::strcpy(cstrings[i], vec[i].c_str());
    }
    cstrings[vec.size()] = NULL;
    return cstrings;
}

void delCStrings(char ** cstrings) {
    int i = -1;
    do {
        i++;
        delete[] cstrings[i];
    } while (cstrings[i] != NULL);
    delete[] cstrings;
}

