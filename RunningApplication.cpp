#include "RunningApplication.h"

using namespace std;

class Application
{
public:
    Application(string applicationPathParam, 
                        int applicationLocationXParam, int applicationLocationYParam, 
                        int applicationSizeHeightParam, int applicationSizeWidthParam) {
    applicationPath = applicationPathParam;
    applicationLocationX = applicationLocationXParam;
    applicationLocationY = applicationLocationYParam;
    applicationSizeHeight = applicationSizeHeightParam;
    applicationSizeWidth = applicationSizeWidthParam;
    }
                        
    ~Application();

private:
string applicationPath;
int applicationLocationX;
int applicationLocationY;
int applicationSizeHeight;
int applicationSizeWidth;

};