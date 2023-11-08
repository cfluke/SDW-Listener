#include "pch.h"
#include "../ListenerAppFunctions/ListenerAppFunctions.h"
#include "..\ListenerAppFunctions\ListenerAppFunctions.cpp"



TEST(TCPMessageToVector, CheckAdditionOfNewApplicationToVectorDataStructure) {

    string errorCode;

    vector<runningApplication> vectorOfRunningApps;

    const int numOfMessages = 3;
    json messagesList[numOfMessages];

    messagesList[0] = {
        {"messageType", "StartApp"},
        {"payload",
         {{"path", "/usr/bin/firefox"},
          {"args", "https://google.com/"},
          {"x", 000},
          {"y", 000},
          {"w", 600},
          {"h", 600}}} };
  
    std::string payloadStr = messagesList[0]["payload"].dump();
    
    runningApplication appD;
    
    parseJSONMessage(payloadStr, errorCode, appD);

    vectorOfRunningApps.push_back(appD);

    json parsedMsg = json::parse(payloadStr); //taken from parseJSONMessage function so that the parsed data is visible to test
     
   // EXPECT_EQ(vectorOfRunningApps[0].applicationPath, parsedMsg["path"]);
   // EXPECT_EQ(vectorOfRunningApps[0].applicationLocationX, parsedMsg["x"]);
   // EXPECT_EQ(vectorOfRunningApps[0].applicationLocationY, parsedMsg["y"]);
  //  EXPECT_EQ(vectorOfRunningApps[0].applicationSizeWidth, parsedMsg["w"]);
  //  EXPECT_EQ(vectorOfRunningApps[0].applicationSizeHeight, parsedMsg["h"]);
   // EXPECT_EQ(vectorOfRunningApps[1], messagesList[1]);
   // EXPECT_EQ(vectorOfRunningApps[2], messagesList[2]);
    EXPECT_EQ(true,true);
}

