#ifndef WRITETODBENTITY_H
#define WRITETODBENTITY_H

#include <string>

using namespace std;

class WriteToDBEntity
{

  public:
    WriteToDBEntity()
    {
    }

    WriteToDBEntity(string msg, string req_in, string req_out, string rsp_in, string rsp_out, int action) : msg(msg), req_in(req_in), req_out(req_out), rsp_in(rsp_in), rsp_out(rsp_out), action(action)
    {
    }

    string getMsg();
    string getReq_in();
    string getReq_out();
    string getRsp_in();
    string getRsp_out();
    int getAction();

  private:
    string msg;
    string req_in;
    string req_out;
    string rsp_in;
    string rsp_out;
    int action;
};

string WriteToDBEntity::getMsg()
{
    return msg;
}

string WriteToDBEntity::getReq_in()
{
    return req_in;
}

string WriteToDBEntity::getReq_out()
{
    return req_out;
}

string WriteToDBEntity::getRsp_in()
{
    return rsp_in;
}

string WriteToDBEntity::getRsp_out()
{
    return rsp_out;
}

int WriteToDBEntity::getAction()
{
    return action;
}

#endif