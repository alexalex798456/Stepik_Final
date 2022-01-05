#include "HTTP1_0.h"
#include <cstring>
#include <assert.h>
#include <sstream>
#include <fstream>
#include <iostream>


std::string IntToStr(const int value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}

std::string Join(const std::string& left, const std::string& right)
{
    std::string _left = left;
    std::string _right = right;
    if (!left.empty() && left[left.size() - 1] == '/')
        _left = left.substr(0, left.size() - 1);
    if (!right.empty() && right[0] == '/')
        _right = right.substr(1);
    return _left + "/" + _right;
}

std::string FileContent(const std::string& fileName)
{
    std::ifstream f(fileName.c_str());
    if (f.good())
    {
        std::stringstream buffer;
        buffer << f.rdbuf();
        return buffer.str();
    }
    return "";
}


class IHTTPResponse
{
public:
    virtual ~IHTTPResponse() = default;

    virtual void PrerareResponse(const std::string& pathRequest, std::string& response) = 0;
};
typedef std::unique_ptr<IHTTPResponse> IHTTPResponseUPtr;


class HTTPResponseGet: public IHTTPResponse
{
public:
    void PrerareResponse(const std::string& pathRequest, std::string& response) override;

    std::string GetStatusString(const int statusCode) const
    {
        const std::string status200 = "200 OK";
        const std::string status404 = "404 Not Found";

        switch (statusCode)
        {
        case 200:
            return status200;
        case 404:
            return status404;
        }
        return "";
    }

    std::string GetTypeContentString(const int typeCode = 0) const
    {
        const std::string typeHtml = "text/html";
        return typeHtml;
    }

    bool ExistsFile(const std::string& name) const
    {
        std::ifstream f(name.c_str());
        return f.good();
    }

};
typedef std::unique_ptr<HTTPResponseGet> HTTPResponseGetUPtr;


void HTTPResponseGet::PrerareResponse(const std::string& pathRequest, std::string& response)
{
    int statusCode = 200;

    if (!ExistsFile(pathRequest))
        statusCode = 404;

    std::string content = "";
    if (statusCode == 200)
        content = FileContent(pathRequest);

    /*content += "<!DOCTYPE HTML>\r\n";
    content += "<html lang=\"en\">\r\n";
    content += "<head>\r\n";
    content += "<META charset=\"UTF-8\">\r\n";
    content += "<META name=\"viewport\"\r\n";
    content += " content=\"width=device-width, initial-scale=1.0\">\r\n";
    content += "<title>Sample Web Page</title>\r\n";
    content += "</head>\r\n";
    content += "<body>\r\n";
    content += "\r\n";
    content += "(Contents go here 000)\r\n";
    content += "\r\n";
    content += "</body>\r\n";
    content += "</html>\r\n";*/

    response = std::string("HTTP/1.0 ") + GetStatusString(statusCode)
        + "\r\nContent-Length: " + IntToStr(content.size())
        + "\r\nContent-Type: " + GetTypeContentString()
        + "\r\nX-Powered-By: ASP.NET\r\nConnection: close\r\n\r\n" + content;

}




HTTP1_0::HTTP1_0(const std::string& path): m_path(path)
{
}

bool HTTP1_0::TryParseRequest(std::string& request)
{
    //std::cout << request << std::endl;

    if (!m_response.empty())
        throw std::string("Last response is still here");

    size_t posEnd = request.find("\r\n\r\n");
    size_t lenTerm = 4;
    if (posEnd == std::string::npos)
    {
        posEnd = request.find("\n\n");
        lenTerm = 2;
    }
    if (posEnd != std::string::npos)
    {
        m_response.clear();
        std::string stRequest = request.substr(0, posEnd + 2);
        std::string stFirstLine = stRequest.substr(0, stRequest.find("\n"));
        request = request.substr(posEnd + lenTerm);
        IHTTPResponseUPtr responseGenerator = nullptr;
        std::string pathRequest;

        // ===== GET request
        if (strcmp(stRequest.substr(0, 3).c_str(), "GET") == 0)
        {
            assert(stFirstLine.size() > 3);
            size_t posHttp = stFirstLine.find("HTTP/1.");
            if (posHttp != std::string::npos)
            {
                pathRequest = stFirstLine.substr(4, posHttp - 5);
                if (!pathRequest.empty())
                {
                    responseGenerator = std::make_unique<HTTPResponseGet>();
                }
            }
        }

        if (responseGenerator)
            responseGenerator->PrerareResponse(Join(m_path, pathRequest), m_response);

        if (!m_response.empty())
        {
            std::cout << m_response << std::endl;
            return true;
        }
        else
            std::cout << "ERROR: Can't parse the request" << std::endl;
    }

    return false;
}

bool HTTP1_0::GetResponse(std::string& response)
{
    if (m_response.empty())
        return false;
    response = m_response;
    m_response.clear();
    return true;
}
