#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <string>

class ParseArgs
{
public:
    ParseArgs(int argc, char** argv);

    std::string GetIP() const;
    int GetPort() const;
    std::string GetPath() const;

private:
    std::string m_IP;
    int         m_port;
    std::string m_path;
};

#endif // UTILS_H_INCLUDED
