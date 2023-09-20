#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <nlohmann/json.hpp> //need to download this library, for centos, check that epel-release is at latest version run: sudo yum install json-devel

using json = nlohmann::json;
using namespace std;

#define START_APP 4
#define KILL_APP 5

struct runningApplication
{
    std::string applicationPath;
    int applicationLocationX, applicationLocationY;
    int applicationSizeHeight, applicationSizeWidth;
    string windowId;
    pid_t applicationProcessId;
    std::vector<char *> args;
};
// JSON message format
/*
JSON MSG format

"{
  "messageType": "runApplicationAndSetSizeAndPosition",
  "payload":
    {
        "path": "appPath",
        "args": "",
        "x": "xCoordinate",
        "y": "yCoordinate",
        "w": "width",
        "h": "height"
    }
}"

*/

void parseJSONMessage(string msgFromMasterApp, string &errorCode, runningApplication &app)
{
    json tcpMessageAsJSON;

    tcpMessageAsJSON = json::parse(msgFromMasterApp);

    app.applicationPath = tcpMessageAsJSON["path"];
    app.applicationLocationX = tcpMessageAsJSON["x"];
    app.applicationLocationY = tcpMessageAsJSON["y"];
    app.applicationSizeWidth = tcpMessageAsJSON["w"];
    app.applicationSizeHeight = tcpMessageAsJSON["h"];

    std::string argStr = tcpMessageAsJSON["args"];
    int start = 0;
    bool inQuotes = false;

    app.args.clear();
    for (int i = 0; i < argStr.length(); ++i)
    {
        if (argStr[i] == ' ' && !inQuotes)
        { // found a space not within quotes; split the argument
            app.args.push_back(const_cast<char *>(strdup(argStr.substr(start, i - start).c_str())));
            start = i + 1;
        }
        else if (argStr[i] == '"')
        {
            inQuotes = !inQuotes;
        }
    }
    app.args.push_back(const_cast<char *>(strdup(argStr.substr(start).c_str()))); // add last arg

    if (argStr == "")
    {
        app.args.clear();
    }
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

    sprintf(programSizeWidth, "%d", app.applicationSizeWidth);
    sprintf(programSizeHeight, "%d", app.applicationSizeHeight);
    sprintf(programPositionX, "%d", app.applicationLocationX);
    sprintf(programPositionY, "%d", app.applicationLocationY);

    pid = fork();

    if (pid == 0)
    {
        // create char* array for execv()

        int n = app.args.size();
        char *args[n + 2];                                         // +1 for NULL
        args[0] = const_cast<char *>(app.applicationPath.c_str()); // Set the program name as the first argument
        // string a = "prog name";
        // args[0] = const_cast<char *>(a.c_str());
        for (int i = 0; i < n; i++)
        {
            args[i + 1] = const_cast<char *>(app.args[i]); // cast std::string to char*
        }
        args[n + 1] = NULL; // need NULL at end

        error = execv(app.applicationPath.c_str(), args);
        // error = execl(app.applicationPath.c_str(),NULL);

        if (error == -1)
        {
            errorCode = "Program execution failed";
        }
        else
        {
            cout << "success" << endl;
        }

        exit(0);
    }

    // TO DO: need to find a way to check if program has started yet, instead of waiting

    struct timespec delay;
    delay.tv_sec = 3;          // seconds
    delay.tv_nsec = 100000000; // nanoseconds, currently set to 100ms
    nanosleep(&delay, c);

    int status = 0;

    if (pid != 0)
    {
        // cout << "Parent PID: " << getpid() << endl;
        // cout << "Child from within Parent PID: " << pid << endl;
        app.applicationProcessId = pid;
        newlyLaunchedProcessPid = pid;
        sprintf(newlyLaunchedProcessPidAsString, "%d", newlyLaunchedProcessPid);
        /*
        pid_t ret = waitpid(newlyLaunchedProcessPid, &status, WNOHANG);
        while(waitpid(newlyLaunchedProcessPid, &status, WNOHANG) == 0)
        {
            nanosleep(&delay, c);
        }
        ret = waitpid(newlyLaunchedProcessPid, &status, WNOHANG);
        if (ret > 0)
        {

            if (WIFEXITED(status) && !WEXITSTATUS(status))
            {
                printf("program execution successful\n");
        */
        strcpy(commandFinal, "xdotool search --onlyvisible --pid ");
        strcat(commandFinal, newlyLaunchedProcessPidAsString);
         cout << commandFinal << endl;
        //  command = "xdotool search --onlyvisible --pid " + to_string(newlyLaunchedProcessPid);

        windowID = executeCommand(commandFinal); // TO DO: check if value is valid
        // TO DO: if result not good, errorCode += "xdotool command: <command> failed";
         cout << "window ID: " << windowID << endl;
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

// Main function for Production

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <server_ip> <server_port> <client_id>" << std::endl;
        return 1;
    }

    const char *serverIP = argv[1];
    int serverPort = std::atoi(argv[2]);
    const char *clientID = argv[3];

    // Create a socket
    int listening = socket(AF_INET, SOCK_STREAM, 0);
    string terminal, terminalName;
    string errorCode, recievedString;

    terminal = "gnome-terminal";
    terminalName = "osboxes@osboxes:~/Desktop/dev";

    int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    // string hello = "Hello from client";

    vector<runningApplication> vectorOfRunningApps;

    char buffer[1024] = {0};
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serverPort);

    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(AF_INET, serverIP, &serv_addr.sin_addr) <= 0)
    {
        printf(
            "\nInvalid address/ Address not supported \n");
        return -1;
    }

    if ((status = connect(client_fd, (struct sockaddr *)&serv_addr,
                          sizeof(serv_addr))) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    // Wait for first 4 bytes of 'RequestIdentify' message
    char lengthBuffer[4];
    if (recv(client_fd, lengthBuffer, sizeof(lengthBuffer), 0) <= 0)
    {

        for (int i = 0; i < 4; i++)
        {
            cout << lengthBuffer[i] << endl;
        }

        perror("Error receiving message length");

        close(client_fd);
        return 1;
    }

    // Fetch rest of 'RequestIdentify' message
    int messageLength = *(int *)lengthBuffer;
    char *messageBuffer = new char[messageLength + 1];
    if (recv(client_fd, messageBuffer, messageLength, 0) <= 0)
    {
        perror("Error receiving message");
        close(client_fd);
        delete[] messageBuffer;
        return 1;
    }
    messageBuffer[messageLength] = '\0';

    // Parse JSON message
    try
    {
        json root = json::parse(messageBuffer);

        // Check if it's a 'RequestIdentify' message
        if (root["messageType"] == "RequestIdentify")
        {
            json payload;
            payload["id"] = clientID;
            payload["ip"] = "insert static IP";
            
            // Create an array for 'displayDetails' and add two DisplayDetails objects
            json displayDetailsArray = json::array();

            // Create and populate the first DisplayDetails object
            json displayDetailsObject1;
            displayDetailsObject1["x"] = 0;
            displayDetailsObject1["y"] = 0;
            displayDetailsObject1["w"] = 3840;
            displayDetailsObject1["h"] = 2160;

            // Create and populate the second DisplayDetails object
            json displayDetailsObject2;
            displayDetailsObject2["x"] = 0;
            displayDetailsObject2["y"] = 2160;
            displayDetailsObject2["w"] = 3840;
            displayDetailsObject2["h"] = 2160;

            // Add both DisplayDetails objects to the array
            displayDetailsArray.push_back(displayDetailsObject1);

            // Add the 'displayDetails' array to the payload
            payload["displayDetails"] = displayDetailsArray;

            // Construct 'Identify' message
            json message;
            message["messageType"] = "Identify";
            message["payload"] = payload.dump();
            std::string messageStr = message.dump();

            // convert length of message from int to 4 bytes
            int n = messageStr.size();
            unsigned char bytes[4];
            bytes[0] = n & 0xFF;
            bytes[1] = (n >> 8) & 0xFF;
            bytes[2] = (n >> 16) & 0xFF;
            bytes[3] = (n >> 24) & 0xFF;

            // append length to start of message
            messageStr.insert(0, reinterpret_cast<const char *>(bytes), sizeof(bytes));
            std::cout << messageStr << std::endl;

            // Send message
            if (send(client_fd, messageStr.c_str(), messageStr.length(), 0) == -1)
            {
                perror("Error sending message");
                close(client_fd);
                return 1;
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error parsing JSON message: " << e.what() << std::endl;
        close(client_fd);
        delete[] messageBuffer;
        return 1;
    }

    // Continuously listen for messages
    while (true)
    {
        // Receive data
        char lengthBuffer[4];
        if (recv(client_fd, lengthBuffer, sizeof(lengthBuffer), 0) <= 0)
        {
            perror("Error receiving message length");
            close(client_fd);
            return 1;
        }

        int messageLength = *(int *)lengthBuffer;
        char *messageBuffer = new char[messageLength + 1];
        if (recv(client_fd, messageBuffer, messageLength, 0) <= 0)
        {
            perror("Error receiving message");
            close(client_fd);
            delete[] messageBuffer;
            return 1;
        }
        messageBuffer[messageLength] = '\0';

        // Parse JSON message
        try
        {
            json message = json::parse(messageBuffer);

            // Check if it's a 'StartApp' message
            if (message["messageType"] == "StartApp")
            {
                std::string payloadStr = message["payload"];

                runningApplication appD;
                parseJSONMessage(payloadStr, errorCode, appD);
                runApplicationResizeReposition(appD);

                vectorOfRunningApps.push_back(appD);
            }
            // Handles Kill processes/shurt down apps message from the master app
            else if (message["messageType"] == "StopApps")
            {
                for (int i = 0; i < vectorOfRunningApps.size(); i++)
                {
                    errorCode = kill(vectorOfRunningApps[i].applicationProcessId, SIGTERM);
                    cout << strerror(errno) << endl;
                }

                cout << "processes killed" << endl;
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error parsing JSON message: " << e.what() << std::endl;
        }

        delete[] messageBuffer;
    }

    // Clean up
    close(client_fd);

    return 0;
}

// main function for testing solo

/*
int main()

{
    string errorCode;

    vector<runningApplication> vectorOfRunningApps;

    json messagesList[6];

    messagesList[0] = {
        {"messageType", 4},
        {"payload",
         {{"path", "/usr/bin/firefox"},
          {"args", "https://google.com/"},
          {"x", 000},
          {"y", 000},
          {"w", 600},
          {"h", 600}}}};

    messagesList[1] = {
        {"messageType", 4},
        {"payload",
         {{"path", "/usr/bin/gnome-calculator"},
          {"args", ""},
          {"x", 200},
          {"y", 50},
          {"w", 400},
          {"h", 400}}}};
    messagesList[2] = {
        {"messageType", 4},
        {"payload",
         {{"path", "/usr/bin/gnome-calculator"},
          {"args", ""},
          {"x", 500},
          {"y", 100},
          {"w", 400},
          {"h", 400}}}};
    messagesList[3] = {
        {"messageType", 5},
        {"payload", ""}};
    messagesList[4] = {
        {"messageType", 4},
        {"payload",
         {{"path", "/usr/bin/gnome-calculator"},
          {"args", ""},
          {"x", 500},
          {"y", 100},
          {"w", 400},
          {"h", 400}}}};
    messagesList[5] = {
        {"messageType", 5},
        {"payload", ""}};

    for (int i = 0; i < 6; i++)
    {
        // Parse JSON message
        try
        {
            json message = messagesList[i];

            // Check if it's a 'StartApp' message
            if (message["messageType"] == START_APP)
            {
                std::string payloadStr = message["payload"].dump();

                runningApplication appD;
                parseJSONMessage(payloadStr, errorCode, appD);
                runApplicationResizeReposition(appD);

                vectorOfRunningApps.push_back(appD);
            }
            // Handles Kill processes/shurt down apps message from the master app
            else if (message["messageType"] == KILL_APP)
            {
                for (int i = 0; i < vectorOfRunningApps.size(); i++)
                {
                    errorCode = kill(vectorOfRunningApps[i].applicationProcessId, SIGTERM);
                    cout << strerror(errno) << endl;
                }

                cout << "processes killed" << endl;
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error parsing JSON message: " << e.what() << std::endl;
        }
    }
    cout << "done" << endl;

    return 0;
}
*/
// done