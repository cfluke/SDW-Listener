#pragma once
#include <iostream>
//#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sys/types.h>
#include <signal.h>
//#include <sys/wait.h>
#include <errno.h>

//#include <sys/socket.h>
//#include <netdb.h>
//#include <arpa/inet.h>

#include <nlohmann/json.hpp> //need to download this library, for centos, check that epel-release is at latest version run: sudo yum install json-devel


using namespace std;
struct runningApplication;
void parseJSONMessage(string msgFromMasterApp, string& errorCode, runningApplication& app);
void runApplicationResizeReposition(runningApplication& app);
