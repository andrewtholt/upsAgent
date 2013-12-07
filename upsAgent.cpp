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

    cout << "^set ONBATT "      << boolalpha << aUps->onBattState() << endl;
    cout << "^set RUNTIME "     << aUps->getRunTime() << endl;
    cout << "^set LOWBATT "     << boolalpha << aUps->lowBattState() << endl;
    cout << "^set ONLINE "      << boolalpha << aUps->onLineState() << endl;
    cout << "^set MINLINEVOLTAGE " << aUps->getMinLineVoltage() << endl;
    cout << "^set LINEVOLTAGE " << aUps->getLineVoltage() << endl;
    cout << "^set MAXLINEVOLTAGE " << aUps->getMaxLineVoltage() << endl;
    cout << "^set BATLEVEL " << aUps->getBatteryLevel() << endl;
    cout << "^set LOAD " << aUps->getLoad() << endl;


    if(aUps->getDebug()) {
        aUps->dump();
        fprintf(stderr,"Fired, next in %ld Seconds.\n",n);
        fprintf(stderr,"Cycle Time %ld\n",cycleTime);
    }

    cout << "^set NEXTSCAN " << n << endl;
    cout << "^set NOW " << t << endl;
    cout << "^set CYCLECOUNT " << aUps->getCycleCount() << endl;
    cout.flush();

    alarm(n);
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

    while((opt=getopt(argc,argv,"b:dhn:p:")) != -1) {
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

    while (1) {
        if (aUps->readFromUps()) {
            printf("VALID\n\t");
            alarm(1);
        }

        if(aUps->getDebug()) {
            aUps->dump();
        }

    }
    close(sockfd);

    return 0;
}
