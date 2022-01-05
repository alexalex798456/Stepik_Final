#ifndef HTTP1_0_H_INCLUDED
#define HTTP1_0_H_INCLUDED

#include <string>
#include <memory>


class HTTP1_0
{
public:
    HTTP1_0(const std::string& path);

    bool TryParseRequest(std::string& request);

    bool GetResponse(std::string& response);

private:
    std::string m_response;
    std::string m_path;

};

typedef std::shared_ptr<HTTP1_0> HTTP1_0Ptr;
typedef std::unique_ptr<HTTP1_0> HTTP1_0UPtr;


#endif // HTTP1_0_H_INCLUDED
