#include <iostream>
#include "HTTPServer.h"
#include "Utils.h"



#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

static void daemon()
{
    pid_t pid;

    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* On success: The child process becomes session leader */
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    /* Catch, ignore and handle signals */
    //TODO: Implement a working signal handler */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    /* Fork off for the second time*/
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* Set new file permissions */
    umask(0);

    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir("/");

    /* Close all open file descriptors */
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
    {
        close (x);
    }
}


int main(int argc, char** argv)
{
    std::cout << "Stepik final test. Main process started" << std::endl;

    ParseArgs parseArgs(argc, argv);

    std::cout << "IP = " << parseArgs.GetIP() << "; port = " << parseArgs.GetPort() << "; path = " << parseArgs.GetPath() << std::endl;

    daemon();
    try
    {
        HTTPServerStepik httpServer(parseArgs.GetIP(), parseArgs.GetPort(), parseArgs.GetPath());
    }
    catch (const std::string& ex)
    {
        std::cout << "EXCEPTION: " << ex << std::endl;
    }

    std::cout << "Stepik final test. Main process stopped" << std::endl;
    return 0;
}
