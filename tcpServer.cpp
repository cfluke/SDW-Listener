
#include "RunningApplication.cpp"

using namespace std;



string runApplicationResizeReposition(string recievedString)
{
    string programPath;
    string programPositionX, programPositionY;
    string programSizeHeight, programSizeWidth;

    string command, response, windowID;
    string windowIDAfterNewLineStrip;
    string errorCode = "No error, OK";
    timespec *c = NULL;
    pid_t pid;
    pid_t newlyLaunchedProcessPid = 0;
    char newlyLaunchedProcessPidAsString[6]; //pid_t is 6 bytes in length
    char commandFinal[50];

    int error = 0;


    string delimiter = ";";

    size_t pos = 0;
    string token;
    int i = 0;
    while ((pos = recievedString.find(delimiter)) != std::string::npos) {
        token = recievedString.substr(0, pos);
        switch (i) {
            case 0: 
                programPath = token;
                break;
            case 1: 
                programPositionX = token;
                break;
            case 2: 
                programPositionY = token;
                break;
            case 3: 
                programSizeHeight = token;
                break;
            default:
                break;
        }
        recievedString.erase(0, pos + 1);
        i++;
    }

    token = recievedString.substr(0, pos);
    programSizeWidth = token;

    pid = fork();

    if (pid != 0)
    {
        error = execl(programPath.c_str()," ", c);

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
    delay.tv_sec = 1;  // seconds
    delay.tv_nsec = 0; // nanoseconds
    nanosleep(&delay, c);

    if (pid == 0)
    {
        newlyLaunchedProcessPid = getppid();
        sprintf(newlyLaunchedProcessPidAsString, "%d",newlyLaunchedProcessPid);

        strcpy(commandFinal,"xdotool search --onlyvisible --pid ");
        strcat(commandFinal, newlyLaunchedProcessPidAsString);
        //cout << commandFinal << endl;
      //  command = "xdotool search --onlyvisible --pid " + to_string(newlyLaunchedProcessPid);

        windowID = executeCommand(commandFinal); // TO DO: check if value is valid
        //TO DO: if result not good, errorCode += "xdotool command: <command> failed";
        windowIDAfterNewLineStrip = windowID;
        windowIDAfterNewLineStrip[windowIDAfterNewLineStrip.length() - 1] = ' ';

        //cout << "windowIDAfterNewLineStrip:\t" << windowIDAfterNewLineStrip << endl;

        command = "xdotool windowsize " + windowIDAfterNewLineStrip + " " + programSizeWidth + " " + programSizeHeight;
       // cout << command << endl;
        response = executeCommand(command);

        command = "xdotool windowmove " + windowIDAfterNewLineStrip + " " + programPositionX + " " + programPositionY;
       // cout << command << endl;
        response = executeCommand(command);
    }

    return errorCode;
    
}

int main()
{
    string terminal, terminalName;
    string errorCode, recievedString;

    terminal = "gnome-terminal";
    terminalName = "osboxes@osboxes:~/Desktop/dev";
//"programPath;programWindowName;programPositionX;programPositionY;programSizeWidth;programSizeHeight"

    //Establish Socket
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
        recievedString = "/usr/bin/gnome-calculator;1000;200;500;500";

        errorCode = runApplicationResizeReposition(recievedString);

        recievedString = "/usr/bin/gnome-calculator;50;50;100;100";

        errorCode = runApplicationResizeReposition(recievedString);


        // Echo message back to client
        //send(clientSocket, buf, bytesReceived + 1, 0);

        
   // }
 
    // Close the socket
   // close(clientSocket);

    
    
}