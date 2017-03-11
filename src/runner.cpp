#include "runner.h"
using namespace std;

int parsemeta(string metafile, meta &metas)
{
    ifstream mf(metafile);
    map<string, string> m;
    string buff;
    while(getline(mf, buff))
    {
        stringstream ss(buff);
        string key, data;
        getline(ss, key, ':');
        getline(ss, data, ':');
        m[key] = data;
    }
    try
    {
        metas.cg_mem = (m["cg-mem"] != "" ? stoll(m["cg-mem"]) : 0);
        metas.csw_forced = (m["csw-forced"] != "" ? stoll(m["csw-forced"]) : 0);
        metas.csw_voluntary = (m["csw-voluntary"] != "" ? stoll(m["csw-voluntary"]) : 0);
        metas.max_rss = (m["max-rss"] != "" ? stoll(m["max-rss"]) : 0);
        metas.time = (m["time"] != "" ? stod(m["time"]) * 1000 : -1);
        metas.time_wall = (m["time-wall"] != "" ? stod(m["time-wall"]) * 1000 : -1);
        metas.exitcode = (m["exitcode"] != "" ? stoi(m["exitcode"]) : 0);
        metas.exitsig = (m["exitsig"] != "" ? stoi(m["exitsig"]) : 0);
        metas.isKilled = (m["killed"] != "" ? stoi(m["killed"]) : 0);
        metas.message = m["message"];
        metas.status = m["status"];
    }
    catch(exception ex)
    {
        cerr << ex.what() << endl;
        cerr << "Bad META file format" <<endl;
        return 2;
    }
    return 0;
}
