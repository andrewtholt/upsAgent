#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <signal.h>
#include <unistd.h>
#include <time.h>
#include "ups.h"

#include <iostream>
using namespace std;
ups *aUps;

void error(const char *msg) {
    perror(msg);
    exit(0);
}

void toSpread() {
    time_t t,n;
    time_t cycleTime;

    t=time(NULL);
    n = cycleTime - abs(t % (long)cycleTime) ;

        cout << "^set ONBATT "      << boolalpha << aUps->onBattState() << endl;
        cout << "^set RUNTIME "     << aUps->getRunTime() << endl;
        cout << "^set LOWBATT "     << boolalpha << aUps->lowBattState() << endl;
        cout << "^set ONLINE "      << boolalpha << aUps->onLineState() << endl;
        cout << "^set MINLINEVOLTAGE " << aUps->getMinLineVoltage() << endl;
        cout << "^set LINEVOLTAGE " << aUps->getLineVoltage() << endl;
        cout << "^set MAXLINEVOLTAGE " << aUps->getMaxLineVoltage() << endl;
        cout << "^set BATLEVEL " << aUps->getBatteryLevel() << endl;
        cout << "^set LOAD " << aUps->getLoad() << endl;
        cout << "^set NEXTSCAN " << n << endl;
        cout << "^set NOW " << t << endl;
        cout << "^set CYCLECOUNT " << aUps->getCycleCount() << endl;

    cout.flush();
}

void toRedis() {
    time_t t,n;
    time_t cycleTime;

    t=time(NULL);
    n = aUps->getCycleTime();

    cout << "multi" << endl;
    cout << "set ONBATT "      << boolalpha << aUps->onBattState() << endl;
    cout << "expire ONBATT "      << n << endl;

    cout << "set RUNTIME "     << aUps->getRunTime() << endl;
    cout << "expire RUNTIME "   << n << endl;

    cout << "set LOWBATT "     << boolalpha << aUps->lowBattState() << endl;
    cout << "expire LOWBATT "   << n << endl;

    cout << "set ONLINE "      << boolalpha << aUps->onLineState() << endl;
    cout << "expire ONLINE "   << n << endl;

    cout << "set MINLINEVOLTAGE " << aUps->getMinLineVoltage() << endl;
    cout << "expire MINLINEVOLTAGE "   << n << endl;

    cout << "set LINEVOLTAGE " << aUps->getLineVoltage() << endl;
    cout << "expire LINEVOLTAGE "   << n << endl;

    cout << "set MAXLINEVOLTAGE " << aUps->getMaxLineVoltage() << endl;
    cout << "expire MAXLINEVOLTAGE "   << n << endl;

    cout << "set BATLEVEL " << aUps->getBatteryLevel() << endl;
    cout << "expire BATLEVEL "   << n << endl;

    cout << "set LOAD " << aUps->getLoad() << endl;
    cout << "expire LOAD "   << n << endl;

    cout << "exec" << endl;

    cout.flush();
}

void handler(int signo) {
    time_t t;
    time_t n;
    time_t cycleTime;
    
    aUps->updateUPSFlags();
    aUps->updateLineFrequency();
    aUps->updateLineVoltage();
    aUps->updateLoad();

    aUps->updateMaxLineVoltage();
    aUps->updateMinLineVoltage();

    aUps->updateOutputVoltage();
    aUps->updateBatteryLevel();
    aUps->updateRunTime();
        
    t=time(NULL);

    if(aUps->onLineState()) {
        cycleTime = (time_t)aUps->getOnPowerDelay();
    } else {
        cycleTime = (time_t)aUps->getOnBattDelay();
    }
    n = cycleTime - abs(t % (long)cycleTime) ;

    switch( aUps->getOutputFormat() ) {
        case TOSPREAD:
            toSpread();
            break;
        case TOREDIS:
            toRedis();
            break;
    }


    if(aUps->getDebug()) {
        aUps->dump();
        fprintf(stderr,"Fired, next in %ld Seconds.\n",n);
        fprintf(stderr,"Cycle Time %ld\n",cycleTime);
    }

    if ( aUps->getLoop()) {
        alarm(n);
    }
}

void usage() {
    printf("\n");
    printf("Usage: upsAgent -d -h -n <name> -p <port>\n\n");
    printf("\t-d\t\tEnable debug mode.\n");
    printf("\t-h\t\tDisplay help.\n");
    printf("\t-n <name/IP>\tConnect to node.\n");
    printf("\t-p <port>\tConnect to port.\n\n");
    printf("\tDefaults are: upsAgent -n localhost -p 4001\n\n");
}

int main(int argc, char *argv[]) {
    char c;
    int sockfd, portno, n;
    int opt;
    int debug=0;
    int state=0; // 0= On Power, 0<> On Battery

    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];

    aUps=new ups();

    while((opt=getopt(argc,argv,"b:dhn:p:xsr")) != -1) {
        switch(opt) {
            case 'd':
                debug=1;
                break;
            case 'h':
                usage();
                exit(0);
            case 'n':
                aUps->setHostname(optarg);
                break;
            case 'p':
                aUps->setPort(atoi(optarg));
                break;
            case 'x':
                aUps->setLoop(false);
                break;
            case 's':
                aUps->setOutputFormat( TOSPREAD );
                break;
            case 'r':
                aUps->setOutputFormat( TOREDIS );
                break;
        }
    }

    if(debug) {
        aUps->setDebug();
        aUps->dump();
    } else {
        aUps->clrDebug();
    }
    
    aUps->mkConnect();

    if(signal(SIGALRM, handler) == SIG_ERR) {
        printf("Can't catch signal\n");
        exit(1);
    }

    alarm(1);

    if( aUps->getLoop() == false) {
        handler(11);
    }
    while ( aUps->getLoop()) {
        if(aUps->getDebug()) {
            aUps->dump();
        }

        if (aUps->readFromUps()) {
            printf("VALID\n\t");
            alarm(1);
        }

    }
    close(sockfd);

    return 0;
}
