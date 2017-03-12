#include "utils.h"
using namespace std;

void log(string mes, string lvpre)
{
    cerr << lvpre << mes << endl;
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
        log("Parsing string error:", LVWA);
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
        log("Parsing string error:", LVWA);
        log(ex.what());
        return def;
    }
}

void parseVecstr(vector<string> vec, char *** output)
{
    vector<char *> v(vec.size() + 1);    // one extra for the null
    for (size_t i = 0; i < vec.size(); i++)
    {
        v[i] = &vec[i][0];
    }
    *output = v.data();
}
