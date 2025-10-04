#include <iostream>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    const pid_t childPid = fork();
    if (childPid < 0)
    {
        std::cerr << "Fork failed" << std::endl;
        exit(1);
    }
    if (childPid == 0)
    {
        _exit(0);
    }

    int status;
    pid_t inputPid;

    std::cout << "Parent process PID: " << getpid() << std::endl;

    do
    {
        std::cout << "Enter zombie PID to wait for (ps -el | grep Z): ";
        std::cin >> inputPid;

        // TODO: для чего состояние z
        const pid_t result = waitpid(inputPid, &status, 0);

        if (result == -1)
        {
            perror("waitpid failed");
            std::cout << "Please enter correct PID" << std::endl;
        }
        else
        {
            std::cout << "Child process " << result << " has been reaped.\n";
            break;
        }
    } while (true);

    return 0;
}
