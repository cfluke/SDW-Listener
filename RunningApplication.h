
//#ifndef APPLICATION_H
//#define APPLICATION_H

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

//#pragma once

using namespace std;

class Application
{
public:
    Application(std::string applicationPathParam, 
                        int applicationLocationX, int applicationLocationY, 
                        int applicationSizeHeight, int applicationSizeWidth);
    ~Application();

private:
std::string applicationPath;

};

//#endif