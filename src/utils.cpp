#include "utils.h"
using namespace std;

loglevel lvlast = LVNU, lvset = LVIN;
void log(string mes, loglevel lvpre)
{
    if(lvpre < lvset)
        return;
    if(lvpre == LVNU && lvlast < lvset)
        return;
    string lvmes;
    switch(lvpre)
    {
        case LVFA : lvmes = "[FATAL]"; break;
        case LVER : lvmes = "[ERROR]"; break;
        case LVWA : lvmes = "[WARN] "; break;
        case LVIN : lvmes = "[INFO] "; break;
        case LVDE : lvmes = "[DEBUG]"; break;
        case LVNU : lvmes = "       "; break;
    }
    if(lvpre != LVNU)
        lvlast = lvpre;
    string buf;
    stringstream ss(mes);
    while(getline(ss, buf))
        cerr << lvmes << " " << buf << endl;
}

loglevel getLevel()
{
    return lvset;
}

void setLevel(loglevel lv)
{
    lvset = lv;
}


string getSelfPath()
{
    char buf[5001];
    readlink("/proc/self/exe", buf, 5000);
    string out(buf);
    return out;
}

string getConfDir()
{
    string path = getSelfPath();
    regex reg("\\/([^\\/]+)");
    smatch sm;
    vector<string> dirlv;
    while(regex_search(path, sm, reg))
    {
        dirlv.push_back(sm[1]);
        path = sm.suffix().str();
    }
    for(int i = dirlv.size() - 1; i >= 0; i--)
    {
        dirlv.pop_back();
        string dir;
        stringstream ss;
        for(int j = 0; j < dirlv.size(); j++)
            ss << "/" << dirlv[j];
        dir = ss.str();
        ss << "/lxtester.conf";
        struct stat st;
        if(stat(ss.str().c_str(), &st) != -1 && st.st_mode & S_IFREG)
        {
            log("Found config file: " + ss.str(), LVDE);
            return dir;
        }
    }
    log("Config file not found", LVER);
    return "";
}

bool isFile(string path)
{
    struct stat st;
    if(stat(path.c_str(), &st) != 0 && S_ISREG(st.st_mode))
    {
        return false;
    }
    return true;
}

bool isDir(string path)
{
    struct stat st;
    if(stat(path.c_str(), &st) != 0 && S_ISDIR(st.st_mode))
    {
        return false;
    }
    return true;
}

int tryParse(string str, int def)
{
    try
    {
        if(str == "")
            return def;
        return stoi(str);
    }
    catch(invalid_argument ex)
    {
        log("Parsing string error:", LVER);
        log(ex.what());
        return def;
    }
}

long long tryParsell(string str, long long def)
{
    try
    {
        if(str == "")
            return def;
        return stoll(str);
    }
    catch(invalid_argument ex)
    {
        log("Parsing string error:", LVER);
        log(ex.what());
        return def;
    }
}

string trim(const string& str)
{
    size_t first = str.find_first_not_of(' ');
    if (string::npos == first)
    {
        return str;
    }
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

char** parseVecstr(vector<string> vec)
{
    char** cstrings = new char*[vec.size() + 1];
    for(size_t i = 0; i < vec.size(); ++i)
    {
        cstrings[i] = new char[vec[i].size() + 1];
        std::strcpy(cstrings[i], vec[i].c_str());
    }
    cstrings[vec.size()] = NULL;
    return cstrings;
}

void delCStrings(char ** cstrings)
{
    int i = -1;
    do
    {
        i++;
        delete[] cstrings[i];
    } while(cstrings[i] != NULL);
    delete[] cstrings;
}

