#include "Utils.h"
#include <unistd.h>
#include <getopt.h>
#include <iostream>


ParseArgs::ParseArgs(int argc, char** argv) : m_port(-1)
{
    int opt;
    while ((opt = getopt(argc, argv, "h:p:d:")) != -1) {
        switch (opt) {
        case 'h':
        {
            m_IP = std::string(optarg);
            break;
        }
        case 'p':
        {
            m_port = atoi(optarg);
            break;
        }
        case 'd':
        {
            m_path = std::string(optarg);
            break;
        }
        default: /* '?' */
            fprintf(stderr, "Usage: %s [-t nsecs] [-n] name\n",
                    std::string("" + char(opt)).c_str());
            exit(EXIT_FAILURE);
        }
    }
    if (m_IP.empty())
    {
        fprintf(stderr, "Argument -h must be specified\n");
        exit(EXIT_FAILURE);
    }
    if (m_port == -1)
    {
        fprintf(stderr, "Argument -p must be specified\n");
        exit(EXIT_FAILURE);
    }
    if (m_path.empty())
    {
        fprintf(stderr, "Argument -d must be specified\n");
        exit(EXIT_FAILURE);
    }
}


std::string ParseArgs::GetIP() const
{
    return m_IP;
}

int ParseArgs::GetPort() const
{
    return m_port;
}

std::string ParseArgs::GetPath() const
{
    return m_path;
}
