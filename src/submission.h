#ifndef SUBMISSION_H
#define SUBMISSION_H
#include <string>
#include <boost/format.hpp>
#include "utils.h"
#include "runner.h"
#include "testsuite.h"

class submission
{
public:
    submission(int id, std::string lang);
    submission(int id, std::string lang, std::string exe, std::string src);
    ~submission();
    int getId();
    pid_t getPID();
    result getResult();
    void setResult(result res);
    std::string getCode();
    void setCode(std::string code);
    pid_t compile();
    pid_t execute();
private:
    language getLang(std::string lang);
    pid_t pid;
    bool created;
    int id;
    std::string code, exename, srcname;
    language lang;
    result res;
    exec_opt opt;
};

#endif // SUBMISSION_H
