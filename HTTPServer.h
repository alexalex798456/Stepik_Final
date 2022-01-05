#ifndef HTTPSERVER_H_INCLUDED
#define HTTPSERVER_H_INCLUDED

#include <memory>
#include <string>
#include <set>



class HTTP1_0;
typedef std::shared_ptr<HTTP1_0> HTTP1_0Ptr;


struct ClientParams
{
public:
    ClientParams(): socket(-1), stop(false), clientId(-1) {}

    int  socket;
    bool stop;
    int  clientId;
    std::string path;

    void executeRequest(std::string& buf, std::string& response);

private:
    HTTP1_0Ptr m_pHttp;
};


class HTTPClient
{
public:
    HTTPClient(const int sock, const std::string& path);

    ~HTTPClient();

    int Socket() const { return m_sock; }

    void Stop() { m_args.stop = true; }

    bool Stopped() const { return (int)m_thread_id == -1; }

    bool CheckExit();

    void Join();

private:
    int          m_clientId;
    int          m_sock;
    pthread_t    m_thread_id;
    ClientParams m_args;
};

typedef std::unique_ptr<HTTPClient> HTTPClientUPtr;


class HTTPServerStepik
{
public:
    HTTPServerStepik(const std::string& ip, const int port, const std::string& path);

    ~HTTPServerStepik();

private:
    std::string              m_ip;
    int                      m_port;
    std::string              m_path;
    std::set<HTTPClientUPtr> m_clients;

    void Start();

    void Stop();

    void CheckClients();

};

#endif // HTTPSERVER_H_INCLUDED
