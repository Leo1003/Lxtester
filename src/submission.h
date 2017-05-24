#ifndef SUBMISSION_H
#define SUBMISSION_H
#include <bitset>
#include <string>
#include <boost/format.hpp>
#include "logger.h"
#include "utils.h"
#include "runner.h"
#include "testsuite.h"

enum RESULT_TYPE
{
    TYPE_EXECUTION = 0,
    TYPE_COMPILATION = 1,
    TYPE_FAILED = 2
};

struct result
{
    std::string std_out, std_err;
    int time, mem, exitcode, signal;
    bool isKilled;
    RESULT_TYPE type;
    result(std::string error);
    result(int boxid, meta metas);
};

class submission
{
public:
    submission();
    submission(int id, std::string lang, std::string exe, std::string src);
    int getId() const;
    exec_opt& getOption();
    result getResult() const;
    void setResult(result res);
    std::string getCode() const;
    void setCode(std::string code);
    std::string getStdin() const;
    void setStdin(std::string data);

    /** Execute submission **/
    int initBoxid();
    int setup();
    int compile();
    int execute();
    int clean();
private:
    language getLang(std::string lang);
    std::string formatCMD(std::string fmstr);
    void extract(std::string file, std::string data);
    int id;
    std::string code, exename, srcname, stdin;
    language lang;
    result res;
    exec_opt opt;
};

class unsupport_language: public exception { };

#endif // SUBMISSION_H
