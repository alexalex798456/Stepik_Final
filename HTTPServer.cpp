#include "HTTPServer.h"
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#include "HTTP1_0.h"


const int SIZE_BUFFER = 1024;


int set_nonblock(int fd)
{
    int flags;
#if defined(O_NONBLOCK)
    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
    flags = 1;
    return ioctl(fd, FIOBIO, &flags);
#endif
}


void ClientParams::executeRequest(std::string& buf, std::string& response)
{
    if (!m_pHttp)
        m_pHttp = std::make_shared<HTTP1_0>(path);
    if (m_pHttp->TryParseRequest(buf))
    {
        if (!m_pHttp->GetResponse(response))
            throw std::string("Can't get response");
    }
}


void* clientFunc(void* ptr)
{
    ClientParams* params = static_cast<ClientParams*>(ptr);
    int sockClient = params->socket;
    timeval timeout = {0, 0};
    char buf[SIZE_BUFFER + 1];
    std::string buffer;
    std::string response;
    while (true)
    {
        try
        {
            fd_set hc;
            FD_ZERO(&hc);
            FD_SET(sockClient, &hc);

            int res = select(sockClient + 1, &hc, nullptr, nullptr, &timeout);
            if (res == -1)
                std::cout << "select return -1; errno = " << errno << std::endl;
            if (!res)
                continue;

            if (FD_ISSET(sockClient, &hc))
            {
                memset(buf, 0, SIZE_BUFFER + 1);
                int recvSize = recv(sockClient, buf, SIZE_BUFFER, MSG_NOSIGNAL);

                if (!recvSize && errno != EAGAIN)
                {
                    shutdown(sockClient, SHUT_RDWR);
                    close(sockClient);
                    break;
                }
                else if (recvSize)
                {
                    buffer += buf;
                    params->executeRequest(buffer, response);
                    if (!response.empty())
                    {
                        int sendSize = send(sockClient, response.c_str(), response.size(), MSG_NOSIGNAL);
                        if (sendSize < (int)response.size())
                            throw std::string("Can't send response to the client #");
                    }
                }
            }

            if (params->stop) // Force stop
            {
                shutdown(sockClient, SHUT_RDWR);
                close(sockClient);
                break;
            }
        }
        catch (const std::string& ex)
        {
            std::cout << ex << std::endl;
        }
    }

    return nullptr;
}


HTTPClient::HTTPClient(const int sock, const std::string& path): m_clientId(-1), m_sock(sock), m_thread_id(-1)
{
    static int s_clientId = 0;
    m_clientId = s_clientId++;
    m_args.socket = m_sock;
    m_args.clientId = m_clientId;
    m_args.path = path;

    int res = pthread_create(&m_thread_id, nullptr, clientFunc, (void*)&m_args);
    if (res != 0)
        throw std::string("Can't create a thread for the new client. Status = ");

    std::cout << "=============== Created client #" << m_clientId << std::endl;
}

bool HTTPClient::CheckExit()
{
    if ((int)m_thread_id == -1)
        return true;
    int status_addr;
    int res = pthread_tryjoin_np(m_thread_id, (void**)&status_addr);
    if (res == EBUSY)
        return false;
    if (res != 0)
        throw std::string("Can't try join the thread for new client. Status = ");
    m_thread_id = -1;
    std::cout << "=============== Stopped (CheckExit) the client #" << m_clientId << std::endl;
    return true;
}

void HTTPClient::Join()
{
    if ((int)m_thread_id == -1)
        return;
    int status_addr;
    int res = pthread_join(m_thread_id, (void**)&status_addr);
    if (res != 0)
        throw std::string("Can't join the thread for new client. Status = ");
    m_thread_id = -1;
    std::cout << "=============== Stopped (Join) client #" << m_clientId << std::endl;
}

HTTPClient::~HTTPClient()
{
    std::cout << "=============== Destroied client #" << m_clientId << std::endl;
}


HTTPServerStepik::HTTPServerStepik(const std::string& ip, const int port, const std::string& path):
    m_ip(ip), m_port(port), m_path(path)
{
    std::cout << "HTTPServerStepik::HTTPServerStepik()" << std::endl;
    Start();
}

HTTPServerStepik::~HTTPServerStepik()
{
    std::cout << "HTTPServerStepik::~HTTPServerStepik()" << std::endl;
    try
    {
        Stop();
    }
    catch (...)
    {
        std::cout << "Exception run method Stop" << std::endl;
    }
}

void HTTPServerStepik::Start()
{
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
        throw std::string("Can't create socket");
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(m_port);
    sa.sin_addr.s_addr = htons(INADDR_ANY);
    int resBind = bind(sock, (struct sockaddr*)(&sa), sizeof(sa));
    if (resBind < 0)
        throw std::string("Can't bind socket");

    int resNonBlock = set_nonblock(sock);
    if (resNonBlock < 0)
        throw std::string("Can't make socket non block");

    int resListen = listen(sock, SOMAXCONN);
    if (resListen < 0)
        throw std::string("Can't listen socket");

    timeval timeout = {0, 0};

    while (true)
    {
        CheckClients();

        fd_set hs;
        FD_ZERO(&hs);
        FD_SET(sock, &hs);

        int res = select(sock + 1, &hs, nullptr, nullptr, &timeout);
        if (res == -1)
            std::cout << "select return -1; errno = " << errno << std::endl;
        if (!res)
            continue;

        if (FD_ISSET(sock, &hs))
        {
            int sockClient = accept(sock, 0, 0);
            set_nonblock(sockClient);
            try
            {
                m_clients.insert(std::make_unique<HTTPClient>(sockClient, m_path));
            }
            catch (const std::string& ex)
            {
                std::cout << ex << std::endl;
            }
            catch (...)
            {
                std::cout << "Can't create 1 new client" << std::endl;
            }
        }
    }
}

void HTTPServerStepik::Stop()
{
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it)
        (*it)->Stop();

    for (auto it = m_clients.begin(); it != m_clients.end(); ++it)
    {
        try
        {
            (*it)->Join();
        }
        catch (const std::string& ex)
        {
            std::cout << ex << std::endl;
        }
        catch (...)
        {
            std::cout << "Can't create 1 new client" << std::endl;
        }
    }
    m_clients.clear();
}

void HTTPServerStepik::CheckClients()
{
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it)
        if ((*it)->CheckExit())
        {
            std::cout << "Client was shutdowned" << std::endl;
            m_clients.erase(*it);
            break;
        }
}


