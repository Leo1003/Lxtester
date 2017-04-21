#ifndef SUBMISSION_H
#define SUBMISSION_H
#include <string>
#include <boost/format.hpp>
#include "global.h"
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
    result();
    result(exec_opt option, meta metas);
};

class submission
{
public:
    submission();
    submission(int id, std::string lang, std::string exe, std::string src);
    int getId() const;
    pid_t getPID() const;
    exec_opt getOption() const;
    result getResult() const;
    void setResult(result res);
    std::string getCode() const;
    void setCode(std::string code);
    std::string getStdin() const;
    void setStdin(std::string data);
    pid_t compile();
    pid_t execute();
private:
    language getLang(std::string lang);
    std::string formatCMD(std::string fmstr);
    pid_t pid;
    int id;
    std::string code, exename, srcname, stdin;
    language lang;
    result res;
    exec_opt opt;
};

#endif // SUBMISSION_H
