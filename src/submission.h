#ifndef SUBMISSION_H
#define SUBMISSION_H
#include <string>
#include <boost/format.hpp>
#include "utils.h"
#include "runner.h"
#include "testsuite.h"

struct result
{
    std::string std_out, std_err;
    int time, mem, exitcode, signal;
    bool isKilled;
    result();
    result(exec_opt option, meta metas);
};

class submission
{
public:
    submission(int id, std::string lang);
    submission(int id, std::string lang, std::string exe, std::string src);
    ~submission();
    int getId() const;
    pid_t getPID() const;
    result getResult() const;
    void setResult(result res);
    std::string getCode() const;
    void setCode(std::string code);
    std::string getStdin() const;
    void setStdin(std::string data);
    pid_t compile();
    pid_t execute();
    
    static std::string BOXDIR;
private:
    language getLang(std::string lang);
    pid_t pid;
    bool created;
    int id;
    std::string code, exename, srcname, stdin;
    language lang;
    result res;
    exec_opt opt;
};

#endif // SUBMISSION_H
