#include "pch.h"
#include "CppUnitTest.h"
#include "../../ListenerAppFunctions/ListenerAppFunctions.h"
#include "..\..\ListenerAppFunctions\ListenerAppFunctions.cpp"
#include <vector>

using json = nlohmann::json;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;



namespace ListenerAppFunctionsNativeUnitTestProject
{
	TEST_CLASS(ListenerAppFunctionsNativeUnitTestProject)
	{
	public:
		
		TEST_METHOD(Test_JSON_Data_Is_Stored_In_Vector)
		{    
            string errorCode;
            vector<runningApplication> vectorOfRunningApps;

            const int numOfMessages = 3;
            json messagesList[numOfMessages];

            messagesList[0] = {
                    {"messageType", "StartApp"},
                    {"payload",
                     {{"path", "/usr/bin/firefox"},
                      {"args", "https://google.com/"},
                      {"x", 100},
                      {"y", 200},
                      {"w", 500},
                      {"h", 600}}} };

            std::string payloadStr = messagesList[0]["payload"].dump();

            runningApplication appD;

            parseJSONMessage(payloadStr, errorCode, appD);

            vectorOfRunningApps.push_back(appD);

            json parsedMsg = json::parse(payloadStr); //taken from parseJSONMessage function so that the parsed data is visible to test
            
            Assert::AreEqual(vectorOfRunningApps[0].applicationPath, string(parsedMsg["path"]));
            Assert::AreEqual(vectorOfRunningApps[0].applicationLocationX, int(parsedMsg["x"]));
            Assert::AreEqual(vectorOfRunningApps[0].applicationLocationY, int(parsedMsg["y"]));
            Assert::AreEqual(vectorOfRunningApps[0].applicationSizeWidth, int(parsedMsg["w"]));
            Assert::AreEqual(vectorOfRunningApps[0].applicationSizeHeight, int(parsedMsg["h"]));
		}
        TEST_METHOD(Test_Multiple_TCP_Messages_Are_Stored_In_Vector)
        {
            string errorCode;
            vector<runningApplication> vectorOfRunningApps;

            const int numOfMessages = 3;
            json messagesList[numOfMessages];

            messagesList[0] = {
        {"messageType", "StartApp"},
        {"payload",
         {{"path", "/usr/bin/firefox"},
          {"args", "https://google.com/"},
          {"x", 100},
          {"y", 200},
          {"w", 300},
          {"h", 600}}} };
            messagesList[1] = {
                {"messageType", "StartApp"},
                {"payload",
                 {{"path", "/usr/bin/gnome-calculator"},
                  {"args", ""},
                  {"x", 200},
                  {"y", 50},
                  {"w", 400},
                  {"h", 400}}} };
            messagesList[2] = {
                {"messageType", "StartApp"},
                {"payload",
                 {{"path", "/usr/bin/gnome-calculator"},
                  {"args", ""},
                  {"x", 500},
                  {"y", 100},
                  {"w", 400},
                  {"h", 900}}} };
           
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
                            //  errorCode = kill(vectorOfRunningApps[i].applicationProcessId, SIGTERM);
                           //   cout << strerror(errno) << endl;
                        }

                        // cout << "processes killed" << endl;
                    }
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Error parsing JSON message: " << e.what() << std::endl;
                }
            }

            json parsedMsg1 = json::parse(messagesList[0]["payload"].dump()); //taken from parseJSONMessage function so that the parsed data is visible to test
            json parsedMsg2 = json::parse(messagesList[1]["payload"].dump()); 
            json parsedMsg3 = json::parse(messagesList[2]["payload"].dump()); 

            Assert::AreEqual(vectorOfRunningApps[0].applicationPath, string(parsedMsg1["path"]));
            Assert::AreEqual(vectorOfRunningApps[0].applicationLocationX, int(parsedMsg1["x"]));
            Assert::AreEqual(vectorOfRunningApps[0].applicationLocationY, int(parsedMsg1["y"]));
            Assert::AreEqual(vectorOfRunningApps[0].applicationSizeWidth, int(parsedMsg1["w"]));
            Assert::AreEqual(vectorOfRunningApps[0].applicationSizeHeight, int(parsedMsg1["h"]));

            Assert::AreEqual(vectorOfRunningApps[1].applicationPath, string(parsedMsg2["path"]));
            Assert::AreEqual(vectorOfRunningApps[1].applicationLocationX, int(parsedMsg2["x"]));
            Assert::AreEqual(vectorOfRunningApps[1].applicationLocationY, int(parsedMsg2["y"]));
            Assert::AreEqual(vectorOfRunningApps[1].applicationSizeWidth, int(parsedMsg2["w"]));
            Assert::AreEqual(vectorOfRunningApps[1].applicationSizeHeight, int(parsedMsg2["h"]));

            Assert::AreEqual(vectorOfRunningApps[2].applicationPath, string(parsedMsg3["path"]));
            Assert::AreEqual(vectorOfRunningApps[2].applicationLocationX, int(parsedMsg3["x"]));
            Assert::AreEqual(vectorOfRunningApps[2].applicationLocationY, int(parsedMsg3["y"]));
            Assert::AreEqual(vectorOfRunningApps[2].applicationSizeWidth, int(parsedMsg3["w"]));
            Assert::AreEqual(vectorOfRunningApps[2].applicationSizeHeight, int(parsedMsg3["h"]));
        }
        TEST_METHOD(Test_Only_StartApp_Messages_Are_Stored_In_Vector)
        {
            string errorCode;
            vector<runningApplication> vectorOfRunningApps;

            const int numOfMessages = 4;
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
            messagesList[1] = {
                {"messageType", "StartApp"},
                {"payload",
                 {{"path", "/usr/bin/gnome-calculator"},
                  {"args", ""},
                  {"x", 200},
                  {"y", 50},
                  {"w", 400},
                  {"h", 400}}} };
            messagesList[2] = {
                {"messageType", "StartApp"},
                {"payload",
                 {{"path", "/usr/bin/gnome-calculator"},
                  {"args", ""},
                  {"x", 500},
                  {"y", 100},
                  {"w", 400},
                  {"h", 400}}} };
            messagesList[3] = {
       {"messageType", "StopApps"},
       {"payload", ""} };

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
                            //  errorCode = kill(vectorOfRunningApps[i].applicationProcessId, SIGTERM);
                           //   cout << strerror(errno) << endl;
                        }

                        // cout << "processes killed" << endl;
                    }
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Error parsing JSON message: " << e.what() << std::endl;
                }
            }


            Assert::AreEqual(int(vectorOfRunningApps.size()),3);
        }
        TEST_METHOD(Test_JSON_Data_Is_Stored_In_Struct)
        {
            string errorCode;
            vector<runningApplication> vectorOfRunningApps;

            const int numOfMessages = 3;
            json messagesList[numOfMessages];

            messagesList[0] = {
                    {"messageType", "StartApp"},
                    {"payload",
                     {{"path", "/usr/bin/firefox"},
                      {"args", "https://google.com/"},
                      {"x", 100},
                      {"y", 50},
                      {"w", 500},
                      {"h", 600}}} };

            std::string payloadStr = messagesList[0]["payload"].dump();

            runningApplication appD;

            parseJSONMessage(payloadStr, errorCode, appD);

            vectorOfRunningApps.push_back(appD);

            json parsedMsg = json::parse(payloadStr); //taken from parseJSONMessage function so that the parsed data is visible to test

            Assert::AreEqual(appD.applicationPath, string(parsedMsg["path"]));
            Assert::AreEqual(appD.applicationLocationX, int(parsedMsg["x"]));
            Assert::AreEqual(appD.applicationLocationY, int(parsedMsg["y"]));
            Assert::AreEqual(appD.applicationSizeWidth, int(parsedMsg["w"]));
            Assert::AreEqual(appD.applicationSizeHeight, int(parsedMsg["h"]));
        }

        //testing error handling from missing data from JSON data
        TEST_METHOD(Test_Missing_Payload_In_JSON_Data_Results_In_Error_Message)
        {
            string errorCode;
            vector<runningApplication> vectorOfRunningApps;

            const int numOfMessages = 3;
            json messagesList[numOfMessages];
            // payload is missing
            messagesList[0] = {
                    {"messageType", "StartApp"},
                    {"payload",{}} };

            std::string payloadStr = messagesList[0]["payload"].dump();

            runningApplication appD;

            
            
            Assert::ExpectException<std::invalid_argument>(
                [&]() {
                    parseJSONMessage(payloadStr, errorCode, appD);
                });
        }
        TEST_METHOD(Test_Missing_Path_In_JSON_Data_Results_In_Error_Message)
        {
            string errorCode;
            vector<runningApplication> vectorOfRunningApps;

            const int numOfMessages = 3;
            json messagesList[numOfMessages];
            // payload is missing
            messagesList[0] = {
                     {"messageType", "StartApp"},
                     {"payload",
                      {
                       {"args", "https://google.com/"},
                       {"x", 100},
                       {"y", 50},
                       {"w", 500},
                       {"h", 600}}} };


            std::string payloadStr = messagesList[0]["payload"].dump();

            runningApplication appD;



            Assert::ExpectException<std::invalid_argument>(
                [&]() {
                    parseJSONMessage(payloadStr, errorCode, appD);
                });
        }
        TEST_METHOD(Test_Missing_Args_In_JSON_Data_Results_In_Error_Message)
        {
            string errorCode;
            vector<runningApplication> vectorOfRunningApps;

            const int numOfMessages = 3;
            json messagesList[numOfMessages];
            // payload is missing
            messagesList[0] = {
                    {"messageType", "StartApp"},
                    {"payload",
                     {{"path", "/usr/bin/firefox"},
                      {"x", 100},
                      {"y", 50},
                      {"w", 500},
                      {"h", 600}}} };


            std::string payloadStr = messagesList[0]["payload"].dump();

            runningApplication appD;



            Assert::ExpectException<std::invalid_argument>(
                [&]() {
                    parseJSONMessage(payloadStr, errorCode, appD);
                });
        }
        TEST_METHOD(Test_Missing_X_Coordinate_In_JSON_Data_Results_In_Error_Message)
        {
            string errorCode;
            vector<runningApplication> vectorOfRunningApps;

            const int numOfMessages = 3;
            json messagesList[numOfMessages];
            // payload is missing
            messagesList[0] = {
                    {"messageType", "StartApp"},
                    {"payload",
                     {{"path", "/usr/bin/firefox"},
                      {"args", "https://google.com/"},
                      {"y", 50},
                      {"w", 500},
                      {"h", 600}}} };


            std::string payloadStr = messagesList[0]["payload"].dump();

            runningApplication appD;



            Assert::ExpectException<std::invalid_argument>(
                [&]() {
                    parseJSONMessage(payloadStr, errorCode, appD);
                });
        }
        TEST_METHOD(Test_Missing_Y_Coordinate_In_JSON_Data_Results_In_Error_Message)
        {
            string errorCode;
            vector<runningApplication> vectorOfRunningApps;

            const int numOfMessages = 3;
            json messagesList[numOfMessages];
            // payload is missing
            messagesList[0] = {
                    {"messageType", "StartApp"},
                    {"payload",
                     {{"path", "/usr/bin/firefox"},
                      {"args", "https://google.com/"},
                      {"x", 100},
                      {"w", 500},
                      {"h", 600}}} };


            std::string payloadStr = messagesList[0]["payload"].dump();

            runningApplication appD;



            Assert::ExpectException<std::invalid_argument>(
                [&]() {
                    parseJSONMessage(payloadStr, errorCode, appD);
                });
        }
        TEST_METHOD(Test_Missing_Width_In_JSON_Data_Results_In_Error_Message)
        {
            string errorCode;
            vector<runningApplication> vectorOfRunningApps;

            const int numOfMessages = 3;
            json messagesList[numOfMessages];
            // payload is missing
            messagesList[0] = {
                    {"messageType", "StartApp"},
                    {"payload",
                     {{"path", "/usr/bin/firefox"},
                      {"args", "https://google.com/"},
                      {"x", 100},
                      {"y", 50},
                      {"h", 600}}} };


            std::string payloadStr = messagesList[0]["payload"].dump();

            runningApplication appD;



            Assert::ExpectException<std::invalid_argument>(
                [&]() {
                    parseJSONMessage(payloadStr, errorCode, appD);
                });
        }
        TEST_METHOD(Test_Missing_Height_In_JSON_Data_Results_In_Error_Message)
        {
            string errorCode;
            vector<runningApplication> vectorOfRunningApps;

            const int numOfMessages = 3;
            json messagesList[numOfMessages];
            // payload is missing
            messagesList[0] = {
                     {"messageType", "StartApp"},
                     {"payload",
                      {{"path", "/usr/bin/firefox"},
                       {"args", "https://google.com/"},
                       {"x", 100},
                       {"y", 50},
                       {"w", 500}}} };


            std::string payloadStr = messagesList[0]["payload"].dump();

            runningApplication appD;



            Assert::ExpectException<std::invalid_argument>(
                [&]() {
                    parseJSONMessage(payloadStr, errorCode, appD);
                });
        }



        TEST_METHOD(Test_If_Args_From_JSON_Data_is_Split_And_Stored_In_RunningApps_Struct_One_Argument)
        {
            string errorCode;
            vector<runningApplication> vectorOfRunningApps;

            const int numOfMessages = 3;
            json messagesList[numOfMessages];

            messagesList[0] = {
                     {"messageType", "StartApp"},
                     {"payload",
                      {{"path", "/usr/bin/firefox"},
                       {"args", "https://google.com/"},
                       {"x", 100},
                       {"y", 50},
                       {"w", 500},
                       {"h", 600}}} };


            std::string payloadStr = messagesList[0]["payload"].dump();

            runningApplication appD;
            
            parseJSONMessage(payloadStr, errorCode, appD);

            std::vector<char*> argsTest;

            argsTest.push_back("https://google.com/");

            Assert::AreEqual(string(appD.args[0]), string(argsTest[0]));
 
        }
        TEST_METHOD(Test_If_Args_From_JSON_Data_is_Split_And_Stored_In_RunningApps_Struct_Three_Argument)
        {
            string errorCode;
            vector<runningApplication> vectorOfRunningApps;

            const int numOfMessages = 3;
            json messagesList[numOfMessages];

            messagesList[0] = {
                     {"messageType", "StartApp"},
                     {"payload",
                      {{"path", "/usr/bin/firefox"},
                       {"args", "https://google.com/ arg1 arg2"},
                       {"x", 100},
                       {"y", 50},
                       {"w", 500},
                       {"h", 600}}} };


            std::string payloadStr = messagesList[0]["payload"].dump();

            runningApplication appD;

            parseJSONMessage(payloadStr, errorCode, appD);

            std::vector<char*> argsTest;

            argsTest.push_back("https://google.com/");
            argsTest.push_back("arg1");
            argsTest.push_back("arg2");

            Assert::AreEqual(string(appD.args[0]), string(argsTest[0]));
            Assert::AreEqual(string(appD.args[1]), string(argsTest[1]));
            Assert::AreEqual(string(appD.args[2]), string(argsTest[2]));

        }
        TEST_METHOD(Test_If_Args_From_JSON_Data_is_Split_And_Stored_In_RunningApps_Struct_No_Argument)
        {
            string errorCode;
            vector<runningApplication> vectorOfRunningApps;

            const int numOfMessages = 3;
            json messagesList[numOfMessages];

            messagesList[0] = {
                     {"messageType", "StartApp"},
                     {"payload",
                      {{"path", "/usr/bin/firefox"},
                       {"args", ""},
                       {"x", 100},
                       {"y", 50},
                       {"w", 500},
                       {"h", 600}}} };


            std::string payloadStr = messagesList[0]["payload"].dump();

            runningApplication appD;

            parseJSONMessage(payloadStr, errorCode, appD);

            std::vector<char*> argsTest;

            argsTest.clear();

            Assert::AreEqual(appD.args.size(),argsTest.size());

        }


	};


}
