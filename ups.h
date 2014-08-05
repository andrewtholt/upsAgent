#include <string>
#define TOSPREAD 1
#define TOREDIS 2

using namespace std;

class ups {
    // Connection
    int port;
    char *hostname;
    // Config
    int onPowerDelay;
    int onBattDelay;
    // State
    bool onBatt;
    bool onLine;
    bool lowBatt;

    unsigned char battCharge;
    unsigned char runTime;
    int lineFrequency;
    int lineVoltage;
    int maxLineVoltage;
    int minLineVoltage;

    int outputVoltage;
    int outputLoad; // A percentage of the rated max load
    int maxLoad; // Watts
    unsigned char flags;
    //
    // Stuff
    //
    int outputFormat;
    bool loopFlag;
    // 
    int cycleCount;
    char *strsave(char *);
    char buffer[255];
    
    int sfd;
    int portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char   alertChars[11];

    int    getUPSNumeric(char );
    void   onPowerUpdate();
    void   onBattUpdate();
    void   lowPowerUpdate();
    string readResponse();

    bool debug;

    public:
        ups();
        void updateUPSFlags();
        int getCycleCount();

        void updateLineFrequency();
        void updateLoad();
        int getLoad();

        void updateMaxLineVoltage();
        int getMaxLineVoltage();

        void updateMinLineVoltage();
        int getMinLineVoltage();

        void updateLineVoltage();
        int  getLineVoltage();

        void updateOutputVoltage();
        void updateBatteryLevel();
        int  getBatteryLevel();
        void updateRunTime();
        int  getRunTime();

        void mkConnect();
        bool readFromUps();
        
        void setPort(int );
        int getPort( );

        void setHostname(char * );
        char *getHostname();

        void setOnBattDelay(int);
        int  getOnBattDelay();

        void setOnPowerDelay(int);
        int  getOnPowerDelay();
        
        bool onLineState();
        bool onBattState();
        bool lowBattState();
        void dump();
        void setDebug();        
        void clrDebug();        
        bool getDebug();        

        bool getLoop();
        void setLoop(bool );

        int getOutputFormat();
        void setOutputFormat(int );
        int getCycleTime();
};

