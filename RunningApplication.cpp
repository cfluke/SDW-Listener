// #include "test.h"
#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <nlohmann/json.hpp> //need to download this library, for centos, check that epel-release is at latest version run: sudo yum install json-devel

using json = nlohmann::json;
using namespace std;

struct runningApplication
{
    std::string applicationPath;
    int applicationLocationX, applicationLocationY;
    int applicationSizeHeight, applicationSizeWidth;
    string windowId;
    pid_t applicationProcessId;
};

/*
JSON MSG format

"{
  "messageType": "runApplicationAndSetSizeAndPosition",
  "requestingProcess": processName,
  "requestingComputer": computerName,
  "payload":
    {
        applicationPath: "appPath",
        positionX: "xCoordinate",
        positionY: "yCoordinate",
        windowHeight: "height",
        windowWidth: "width"
    }
}"

*/

void parseJSONMessage(string msgFromMasterApp, string &errorCode, runningApplication &app)
{
    json tcpMessageAsJSON;

    tcpMessageAsJSON = json::parse(msgFromMasterApp);

    app.applicationPath = tcpMessageAsJSON["payload"]["applicationPath"];
    app.applicationLocationX = tcpMessageAsJSON["payload"]["positionX"];
    app.applicationLocationY = tcpMessageAsJSON["payload"]["positionY"];
    app.applicationSizeHeight = tcpMessageAsJSON["payload"]["windowHeight"];
    app.applicationSizeWidth = tcpMessageAsJSON["payload"]["windowWidth"];
}

string executeCommand(string command)
{
    string result;
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe)
    {
        cerr << "Error executing command." << endl;
        return result;
    }

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL)
    {
        result += buffer;
    }

    pclose(pipe);
    return result;
}

string runApplicationResizeReposition(runningApplication &app)
{
    char programPath[50];
    char programPositionX[50];
    char programPositionY[50];
    char programSizeHeight[50];
    char programSizeWidth[50];

    string command, response, windowID;
    string windowIDAfterNewLineStrip;
    string errorCode = "No error, OK";
    timespec *c = NULL;
    pid_t pid;
    pid_t newlyLaunchedProcessPid = 0;
    char newlyLaunchedProcessPidAsString[6]; // pid_t is 6 bytes in length
    char commandFinal[50];

    int error = 0;

    string delimiter = ";";

    size_t pos = 0;
    string token;

    sprintf(programSizeWidth, "%d", app.applicationSizeWidth);
    sprintf(programSizeHeight, "%d", app.applicationSizeHeight);
    sprintf(programPositionX, "%d", app.applicationLocationX);
    sprintf(programPositionY, "%d", app.applicationLocationY);

    pid = fork();

    if (pid == 0)
    {
        // cout << "Child PID: " << getpid() << endl;
        error = execl(app.applicationPath.c_str(), " ", c);

        if (error == -1)
        {
            errorCode = "Program execution failed";  //useless errorcode, child will exit soon after, parent has no knowledge
        }
        else
        {
            cout << "success" << endl;
        }

        exit(0);
    }

    // TO DO: need to find a way to check if program has started yet, instead of waiting

    struct timespec delay;
    delay.tv_sec = 3;  // seconds
    delay.tv_nsec = 0; // nanoseconds
    nanosleep(&delay, c);

    int status = 0;

    if (pid != 0)
    {
        // cout << "Parent PID: " << getpid() << endl;
        // cout << "Child from within Parent PID: " << pid << endl;
        // newlyLaunchedProcessPid = getppid();
        newlyLaunchedProcessPid = pid;
        sprintf(newlyLaunchedProcessPidAsString, "%d", newlyLaunchedProcessPid);

        /*   if (waitpid(newlyLaunchedProcessPid, &status, 0) > 0)
           {

               if (WIFEXITED(status) && !WEXITSTATUS(status))
               {
                   printf("program execution successful\n");
   */
        strcpy(commandFinal, "xdotool search --onlyvisible --pid ");
        strcat(commandFinal, newlyLaunchedProcessPidAsString);
        // cout << commandFinal << endl;
        //  command = "xdotool search --onlyvisible --pid " + to_string(newlyLaunchedProcessPid);

        windowID = executeCommand(commandFinal); // TO DO: check if value is valid
        // TO DO: if result not good, errorCode += "xdotool command: <command> failed";
        // cout << "window ID: " << windowID << endl;
        windowIDAfterNewLineStrip = windowID;
        windowIDAfterNewLineStrip[windowIDAfterNewLineStrip.length() - 1] = ' ';

        // cout << "windowIDAfterNewLineStrip:\t" << windowIDAfterNewLineStrip << endl;
        app.windowId = windowIDAfterNewLineStrip;

        strcpy(commandFinal, "xdotool windowsize ");
        strcat(commandFinal, windowIDAfterNewLineStrip.c_str());
        strcat(commandFinal, " ");
        strcat(commandFinal, programSizeWidth);
        strcat(commandFinal, " ");
        strcat(commandFinal, programSizeHeight);

        // command = "xdotool windowsize " + windowIDAfterNewLineStrip + " " + programSizeWidth + " " + programSizeHeight;
        // cout << commandFinal << endl;
        response = executeCommand(commandFinal);

        strcpy(commandFinal, "xdotool windowmove ");
        strcat(commandFinal, windowIDAfterNewLineStrip.c_str());
        strcat(commandFinal, " ");
        strcat(commandFinal, programPositionX);
        strcat(commandFinal, " ");
        strcat(commandFinal, programPositionY);

        // command = "xdotool windowmove " + windowIDAfterNewLineStrip + " " + programPositionX + " " + programPositionY;
        // cout << commandFinal << endl;
        response = executeCommand(commandFinal);
        /*
                }
                else if (WIFEXITED(status) && WEXITSTATUS(status))
                {
                    if (WEXITSTATUS(status) == 127)
                    {

                        // execv failed
                        printf("execv failed\n");
                    }
                    else
                        printf("program terminated normally,"
                               " but returned a non-zero status\n");
                }
                else
                    printf("program didn't terminate normally\n");
            }
            else
            {
                // waitpid() failed
                printf("waitpid() failed\n");
            }

            */
    }

    return errorCode;
}

int main()
{
    // Create a socket
    int listening = socket(AF_INET, SOCK_STREAM, 0);
    string terminal, terminalName;
    string errorCode, recievedString;

    terminal = "gnome-terminal";
    terminalName = "osboxes@osboxes:~/Desktop/dev";

    // Establish Socket
    /*
         int listening = socket(AF_INET, SOCK_STREAM, 0);
        if (listening == -1)
        {
            errorCode = "error creating a socket";
        }

        //bind ip address (any) and port number to socket.

        sockaddr_in hint;
        hint.sin_family = AF_INET;
        hint.sin_port = htons(54000); //Port 54000
        inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);

        bind(listening, (sockaddr*)&hint, sizeof(hint));

        //tells socket to listen for messages with length up to max length
        listen(listening, SOMAXCONN);

        sockaddr_in client;
        socklen_t clientSize = sizeof(client);

        int clientSocket = accept(listening, (sockaddr*)&client, &clientSize);

        char host[NI_MAXHOST];      // Client's remote name
        char service[NI_MAXSERV];   // Service (i.e. port) the client is connect on

        memset(host, 0, NI_MAXHOST); // same as memset(host, 0, NI_MAXHOST);
        memset(service, 0, NI_MAXSERV);

         if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
        {
            cout << host << " connected on port " << service << endl;
        }
        else
        {
            inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
            cout << host << " connected on port " << ntohs(client.sin_port) << endl;
        }

        close(listening);

        // While loop: accept and echo message back to client
        char buf[4096];

        while (true)
        {
            memset(buf, 0, 4096);

            // Wait for client to send data
            int bytesReceived = recv(clientSocket, buf, 4096, 0);
            if (bytesReceived == -1)
            {
                cerr << "Error in receiving" << endl;
              //  break;
            }

            if (bytesReceived == 0)
            {
                cout << "Client has been disconnected" << endl;
              //  break;
            }

            cout << "Received Message: " << string(buf, 0, bytesReceived) << endl;
            recievedString = string(buf, 0, bytesReceived);
            */
    runningApplication appD;

    string prog1 = "/usr/bin/firefox";
    string prog2 = "/usr/bin/gnome-calculator";
    string prog3 = "/usr/bin/gnome-clocks";
    // prog = "/usr/bin/gnome-terminal";

    string sampleMsg = "{ \"messageType\": \"runApplicationAndSetSizeAndPosition\", \"requestingProcess\": \"processName\", \"requestingComputer\": \"computerName\", \"payload\": { \"applicationPath\": \"/usr/bin/gnome-calculator\", \"positionX\" : 100,\"positionY\" : 100, \"windowHeight\" : 600, \"windowWidth\" : 600}}";

    parseJSONMessage(sampleMsg, errorCode, appD);
    /*
    cout << "parsed app path: " << appD.applicationPath << endl;

    cout << "parsed app x coord: " << appD.applicationLocationX << endl;
    cout << "parsed app y coord: " << appD.applicationLocationY << endl;

    cout << "parsed app height: " << appD.applicationSizeHeight << endl;
    cout << "parsed app width: " << appD.applicationSizeWidth << endl;
    */

    errorCode = runApplicationResizeReposition(appD);
    cout << "error code: " << errorCode << endl;

    // Echo message back to client
    // send(clientSocket, buf, bytesReceived + 1, 0);

    // }

    // Close the socket
    // close(clientSocket);

    return 0;
}
