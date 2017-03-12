#include "utils.h"
using namespace std;

void warn(std::string mes, bool showlv)
{
    string lv = (showlv ? "[WARN]" : "      ");
    cerr << lv << mes << endl;
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
        warn("Parsing string error:");
        warn(ex.what(), false);
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
        warn("Parsing string error:");
        warn(ex.what(), false);
        return def;
    }
}

void parseVecstr(std::vector<std::string> vec, char *** output)
{
    vector<char *> v(vec.size() + 1);    // one extra for the null
    for (size_t i = 0; i < vec.size(); i++)
    {
        v[i] = &vec[i][0];
    }
    *output = v.data();
}
