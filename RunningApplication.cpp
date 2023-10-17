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

#define TIMEOUT_VALUE_SECONDS 3
#define TIMEOUT_VALUE_NANOSECONDS 100000000            // currently set to 100ms
#define REFRESH_TIME_PERIOD_VALUE_NANOSECONDS 10000000 // currently set to 1ms
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

    if (tcpMessageAsJSON.contains("path"))
    {
        app.applicationPath = tcpMessageAsJSON["path"];
    }
    else
    {
        printf("\nEncountered an error, program path was not found in the JSON object which was parsed from the TCP message.\n");
        throw std::invalid_argument("Encountered an error, program path was not found in the JSON object which was parsed from the TCP message.");
    }

    if (tcpMessageAsJSON.contains("x"))
    {
        app.applicationLocationX = tcpMessageAsJSON["x"];
    }
    else
    {
        printf("\nEncountered an error, the x-coordinate for the window was not found in the JSON object which was parsed from the TCP message.\n");
        throw std::invalid_argument("Encountered an error, the x - coordinate for the window was not found in the JSON object which was parsed from the TCP message.");
    }

    if (tcpMessageAsJSON.contains("y"))
    {
        app.applicationLocationY = tcpMessageAsJSON["y"];
    }
    else
    {
        printf("\nEncountered an error, y-coordinate for the window was not found in the JSON object which was parsed from the TCP message.\n");
        throw std::invalid_argument("Encountered an error, y-coordinate for the window was not found in the JSON object which was parsed from the TCP message.");
    }

    if (tcpMessageAsJSON.contains("w"))
    {
        app.applicationSizeWidth = tcpMessageAsJSON["w"];
    }
    else
    {
        printf("\nEncountered an error, the width value for the window was not found in the JSON object which was parsed from the TCP message.\n");
        throw std::invalid_argument("Encountered an error, the width value for the window was not found in the JSON object which was parsed from the TCP message.");
    }

    if (tcpMessageAsJSON.contains("h"))
    {
        app.applicationSizeHeight = tcpMessageAsJSON["h"];
    }
    else
    {
        printf("\nEncountered an error, the height value for the window was not found in the JSON object which was parsed from the TCP message.\n");
        throw std::invalid_argument("Encountered an error, the height value for the window was not found in the JSON object which was parsed from the TCP message.");
    }

    if (!tcpMessageAsJSON.contains("args"))
    {
        printf("\nEncountered an error, the arguments field was not in the correct format in the JSON object which was parsed from the TCP message.\n");
        throw std::invalid_argument("Encountered an error, the arguments field was not in the correct format in the JSON object which was parsed from the TCP message.");
    }

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
    char error[1000];
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe)
    {
        sprintf(error,"\nEncountered an error while executing a shell command: %s\n", strerror(errno));
        throw std::invalid_argument(error);

        result = "";
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

runApplicationResizeReposition(runningApplication &app)
{
    char programPath[50];
    char programPositionX[50];
    char programPositionY[50];
    char programSizeHeight[50];
    char programSizeWidth[50];

    string command, response, windowID;
    string windowIDAfterNewLineStrip;
    char error[1000];
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

        // error = execl(app.applicationPath.c_str(),NULL);

        if (execv(app.applicationPath.c_str(), args) == -1)
        {
            sprintf(error, "\nLaunching application at: \"%s\" failed. launching application error (from function \"execv()\"): %s", app.applicationPath.c_str(), strerror(errno));
            throw std::invalid_argument(error);
        }

        exit(0);
    }

    // TO DO: need to find a way to check if program has started yet, instead of waiting

    struct timespec delay;

    unsigned long int timeoutInNanoSeconds = TIMEOUT_VALUE_SECONDS * (unsigned long int)1000000000 + TIMEOUT_VALUE_NANOSECONDS;

    int status = 0;

    if (pid > 0)
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
        // cout << commandFinal << endl;
        //    command = "xdotool search --onlyvisible --pid " + to_string(newlyLaunchedProcessPid);

        unsigned long int timePassedInNanoseconds = 0;
        windowID = "";

        delay.tv_nsec = REFRESH_TIME_PERIOD_VALUE_NANOSECONDS;
        delay.tv_sec = 0;

        // while loop runs if windowID has a length less than 15 because that means an error message from xdotool is returned instead of a valid window ID
        while (windowID.length() < 2 && timePassedInNanoseconds < timeoutInNanoSeconds)
        {
            windowID = executeCommand(commandFinal);

            // cout << "window ID from while loop: " << windowID << endl;

            timePassedInNanoseconds += REFRESH_TIME_PERIOD_VALUE_NANOSECONDS;

            nanosleep(&delay, c);
        }

        if (windowID < 2) {
            sprintf(error, "\nError finding window ID for launched application: \"%s\" failed. Window ID search timed out, potential fix: increase value of \"TIMEOUT_VALUE_NANOSECONDS\" const in listener app.\n", app.applicationPath.c_str());
            throw std::invalid_argument(error);
        }
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
    else if (pid == -1)
    {
        printf("\nCreating a child process to launch the application failed.\n");
        printf("Creating a child process failed (from function \"fork\"): %s", strerror(errno));
    }

    return errorCode;
}

// Main function for Production
/*
int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "Please provide: " << argv[0] << " <server_ip> <server_port> <client_id>\n"
                  << "For example: ./ListenerApp 192.168.1.1 8000 KekDisplay1" << std::endl;
        return 1;
    }

    const char *serverIP = argv[1];
    int serverPort = std::atoi(argv[2]);
    const char *clientID = argv[3];

    errno = 0;

    // Create a socket
    int listening = socket(AF_INET, SOCK_STREAM, 0);
    string terminal, terminalName;
    string errorCode, recievedString;

    int status, valread, client_fd;
    struct sockaddr_in serv_addr;

    vector<runningApplication> vectorOfRunningApps;

    char buffer[1024] = {0};
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {

        printf("\n Connection Error: Failed to Create a Socket for TCP Connection. \n");
        printf("Socket error: %s", strerror(errno));
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serverPort);

    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(AF_INET, serverIP, &serv_addr.sin_addr) <= 0)
    {
        printf("\n Invalid IP address. Could not convert IP address to binary form. Exiting the program now.\n");
        printf("IP address conversion error: %s", strerror(errno));
        return -1;
    }

    if ((status = connect(client_fd, (struct sockaddr *)&serv_addr,
                          sizeof(serv_addr))) < 0)
    {
        printf("\nTCP connection with server failed. Exiting the program now. \n");
        printf("TCP connection errror: %s", strerror(errno));
        return -1;
    }

    // Wait for first 4 bytes of 'RequestIdentify' message
    char lengthBuffer[4];
    ssize_t bytesRecieved;

    bytesRecieved = recv(client_fd, lengthBuffer, sizeof(lengthBuffer), 0);

    if (bytesRecieved <= 0)
    {
        if (bytesRecieved == 0)
        {
            printf("\nMaster application has terminated connection. Exiting the program now \n");
        }
        else
        {
            printf("TCP \"RequestIdentify\" message recieving error: %s", strerror(errno));
            printf("\n. Exiting the program now. \n");
        }

        close(client_fd);
        return 1;
    }

    // Fetch rest of 'RequestIdentify' message
    int messageLength = *(int *)lengthBuffer;
    char *messageBuffer = new char[messageLength + 1];

    bytesRecieved = recv(client_fd, messageBuffer, messageLength, 0);
    if (bytesRecieved <= 0)
    {
        if (bytesRecieved == 0)
        {
            printf("\nMaster application has terminated connection. Exiting the program now \n");
        }
        else
        {
            printf("TCP message recieving error: %s", strerror(errno));
            printf("\n. Exiting the program now. \n");
        }

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
            displayDetailsArray.push_back(displayDetailsObject2);

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
            // std::cout << messageStr << std::endl;

            // Send message
            if (send(client_fd, messageStr.c_str(), messageStr.length(), 0) == -1)
            {
                printf("Listener app \"Identify\" TCP message send error: %s", strerror(errno));
                printf("\n. Exiting the program now. \n");
                close(client_fd);
                return 1;
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error parsing JSON message: " << e.what() << std::endl;
        printf("\n. Exiting the program now. \n");
        close(client_fd);
        delete[] messageBuffer;
        return 1;
    }

    // Continuously listen for messages

    printf("\nListening for Messages.\n");
    while (true)
    {
        // Receive data
        char lengthBuffer[4];

        bytesRecieved = recv(client_fd, lengthBuffer, sizeof(lengthBuffer), 0);
        if (bytesRecieved <= 0)
        {
            if (bytesRecieved == 0)
            {
                printf("\nMaster application has terminated connection. Exiting the program now \n");
            }
            else
            {
                printf("Encountered an error when recieving the first 4 bytes from a command message via TCP from master application: %s", strerror(errno));
                printf("\n. Exiting the program now. \n");
            }

            close(client_fd);
            return 1;
        }

        int messageLength = *(int *)lengthBuffer;
        char *messageBuffer = new char[messageLength + 1];

        bytesRecieved = recv(client_fd, messageBuffer, messageLength, 0);
        if (bytesRecieved <= 0)
        {
            if (bytesRecieved == 0)
            {
                printf("\nMaster application has terminated connection. Exiting the program now \n");
            }
            else
            {
                printf("Encountered an error when recieving a command message via TCP from master application: %s", strerror(errno));
                printf("\n. Exiting the program now. \n");
            }
            close(client_fd);
            delete[] messageBuffer;
            return 1;
        }
        messageBuffer[messageLength] = '\0';

        // Parse JSON message
        try
        {
            json message = json::parse(messageBuffer);

            string messageType = message["messageType"];

            // Check if it's a 'StartApp' message
            if (messageType == "StartApp")
            {
                std::string payloadStr = message["payload"];

                 try
                {
                    runningApplication appD;
                    parseJSONMessage(payloadStr, errorCode, appD);
                    runApplicationResizeReposition(appD);

                    vectorOfRunningApps.push_back(appD);
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Error parsing JSON payload data field in message: " << e.what() << std::endl;
                }
            }
            // Handles Kill processes/shut down apps message from the master app
            else if (messageType == "StopApps")
            {
                for (int i = 0; i < vectorOfRunningApps.size(); i++)
                {
                    errorCode = kill(vectorOfRunningApps[i].applicationProcessId, SIGTERM);
                    cout << strerror(errno) << endl;
                }

                cout << "processes killed" << endl;
            }
            else
            {
                printf("Encountered an Error, \"%s\" was received as the message type which is an invalid value.\n", messageType.c_str());
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
*/

// main function for testing solo
int main()

{
    string errorCode;

    vector<runningApplication> vectorOfRunningApps;

    int numOfMessages = 1;
    json messagesList[numOfMessages];

    /*messagesList[0] = {
        {"messageType", "StartApp"},
        {"payload",
         {{"path", "/usr/bin/firefox"},
          {"args", "https://google.com/"},
          {"x", 000},
          {"y", 10},
          {"w", 600},
          {"h", 600}}}}; */
    messagesList[0] = {
        {"messageType", "StartApp"},
        {"payload",
         {{"path", "/usr/bin/gnome-calculator"},
          {"args", ""},
          {"x", 700},
          {"y", 50},
          {"w", 400},
          {"h", 400}}}};
    /* messagesList[2] = {
         {"messageType", "StartApp"},
         {"payload",
          {{"path", "/usr/bin/gnome-calculator"},
           {"x", 500},
           {"y", 100},
           {"w", 400},
           {"h", 400}}}}; */
    /* messagesList[3] = {
         {"messageType", "StopApps"},
         {"payload", ""}}; */
    /* messagesList[4] = {
         {"messageType", "StartApp"},
         {"payload",
          {{"path", "/usr/bin/gnome-calculator"},
           {"args", ""},
           {"x", 500},
           {"y", 100},
           {"w", 400},
           {"h", 400}}}}; */
    /* messagesList[5] = {
         {"messageType", "StopApps"},
         {"payload", ""}}; */

    struct timespec delay;
    delay.tv_nsec = 0;
    delay.tv_sec = 2;

    for (int i = 0; i < numOfMessages; i++)
    {
        // Parse JSON message
        try
        {
            json message = messagesList[i];

            // Check if it's a 'StartApp' message
            if (message["messageType"] == "StartApp")
            {
                std::string payloadStr = message["payload"].dump();

                try
                {
                    runningApplication appD;
                    parseJSONMessage(payloadStr, errorCode, appD);
                    runApplicationResizeReposition(appD);

                    vectorOfRunningApps.push_back(appD);
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Error parsing JSON payload data field in message: " << e.what() << std::endl;
                }
            }
            // Handles Kill processes/shurt down apps message from the master app
            else if (message["messageType"] == "StopApps")
            {
                for (int i = 0; i < vectorOfRunningApps.size(); i++)
                {
                    errorCode = kill(vectorOfRunningApps[i].applicationProcessId, SIGTERM);
                    cout << strerror(errno) << endl;
                }

                cout << "processes killed\n" << endl;
                cout << "\nDeleting Firefox User Profiles in the directory \"usr/firefoxProfiles/\"." << endl;
                cout << executeCommand("rm -r usr/firefoxProfiles/*");
            }

            nanosleep(&delay, nullptr);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error parsing JSON message: " << e.what() << std::endl;
        }
    }

    return 0;
}

// done