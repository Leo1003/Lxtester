#include "submission.h"
using namespace std;

submission::submission(int id, string lang)
{
    this->id = id;
    this->lang = lang;
}

string submission::getCode()
{
    return code;
}

void submission::setCode(string code)
{
    this->code = code;
}

int submission::getId()
{
    return id;
}

pid_t submission::getPID()
{
    return pid;
}

result submission::getResult()
{
    return res;
}

void submission::setResult(result res)
{
    this->res = res;
}

int submission::compile()
{
    //TODO:Not yet implement
    return 127;
}

int submission::execute()
{
    //TODO:Not yet implement
    return 127;
}
