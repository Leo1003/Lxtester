#ifndef SUBMISSION_H
#define SUBMISSION_H
#include <string>
#include "utils.h"
#include "runner.h"

class submission
{
public:
    submission(int id, std::string lang);
    int getId();
    pid_t getPID();
    result getResult();
    void setResult();
    int compile();
    int execute();
private:
    pid_t pid;
    int id;
    std::string lang;
    result res;
};

#endif // SUBMISSION_H
