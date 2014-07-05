#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>

#include "ups.h"
using namespace std;

ups::ups() {
    port=4001;
    hostname=strsave((char *)"localhost");
    
//    onPowerDelay=2*60 ;
    onPowerDelay=15*60 ;
    onBattDelay=1*60 ;

    cycleCount=0;
    
    lowBatt = false;
    onBatt = false;
    onLine = true;
    battCharge=0;     // %
    runTime=0;        // two hours
    lineFrequency=0;
    maxLoad=390; // Watts
    outputLoad=100; // %
    debug=true;
    
    alertChars[0] = '!';
    alertChars[1] = '$';
    alertChars[2] = '%';
    alertChars[3] = '+';
    alertChars[4] = '?';
    alertChars[5] = '=';
    alertChars[6] = '*';
    alertChars[7] = '#';
    alertChars[8] = '&';
    alertChars[9] = '|';
    alertChars[10] = 0x00;
}

void ups::mkConnect() {
    
    sfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sfd < 0) {
        perror("ERROR opening socket");
        exit(-1);
    }
    
    server = gethostbyname(hostname);
    if (server == NULL) {
        perror("ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof (serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    if (connect(sfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0){
        perror("ERROR connecting");
        exit(-2);
    }
    
}

bool ups::readFromUps() {
    int n;
    int count=0;
    int bytesAvailable=0;
    
    char lbuffer[2];
    char *ptr=(char *)NULL;

    bool status=false;

    lbuffer[0] = lbuffer[1] = 0;

    if(debug) {
        printf("readFromUPS\n");
    }
    n=ioctl(sfd,FIONREAD,&bytesAvailable);
    
    n = recv(sfd, lbuffer, 1, 0);
    
    if( n < 0) {
        perror("read");
    }
    
    ptr = index(alertChars,lbuffer[0]);
    
    if((char *)NULL != ptr) {
        printf("ALERT !!!!");
        status=true;
    } else {
        status=false;
    }

    
    n=ioctl(sfd,FIONREAD,&bytesAvailable);
    
    if(bytesAvailable > 0) {
        n = recv(sfd, lbuffer, bytesAvailable, 0);
    }
    
    return(status);
}

char *ups::strsave(char *s) {
    char *p;

    if ((p = (char *) malloc(strlen(s) + 1)) != NULL) {
        strcpy(p, s);
    }

    return(p);
}

bool ups::onLineState() {
    return((bool)onLine);
}

bool ups::onBattState() {
    return((bool)onBatt);
}

bool ups::lowBattState() {
    return((bool)lowBatt);
}

void ups::dump() {
    printf("Cycle Count    : %d\n", cycleCount);
    printf("Debug Flag     : %d\n", debug);
    printf("Host           : %s\n", hostname);
    printf("Port           : %4d\n", port);
    printf("Flags          : %04x\n", flags);
    printf("Line Frequency : %4d\n", lineFrequency);
    printf("Line   Voltage : %4d\n", lineVoltage);
    printf("Output Voltage : %4d\n", outputVoltage);
    printf("Output Load    : %4d %%\n", outputLoad);
    printf("Battery Charge : %4d\n", battCharge);
    printf("Run Time       : %4d Minutes.\n", runTime);

    printf("\n");
    printf("On Power Delay : %4d Seconds\n", onPowerDelay);
    printf("On Batt Delay  : %4d Seconds\n", onBattDelay);
    
    printf("On Batt        : ");
    if(onBatt) {
        printf("true\n");
    } else {
        printf("false\n");
    }
    
    printf("On Line        : ");
    if(onLine) {
        printf("true\n");
    } else {
        printf("false\n");
    }
    
    printf("Low Battery    : ");
    if(lowBatt) {
        printf("true\n");
    } else {
        printf("false\n");
    }
    printf("===============\n");
}
void ups::setPort(int p) {
    port=p;
}

int ups::getPort() {
    return(port);
}

void ups::setHostname(char *n) {
    
    if( hostname != (char *)NULL ) {
        free(hostname);
    }
    hostname=strsave(n);
}

char * ups::getHostname() {
    return(hostname);
}

void ups::setOnBattDelay(int n) {
    onBattDelay = n * 60;
}

int ups::getOnBattDelay() {
    return(onBattDelay);
    
}

void ups::setOnPowerDelay(int n) {
    onPowerDelay = n * 60;
}

int ups::getOnPowerDelay() {
    return(onPowerDelay);
    
}

void ups::onPowerUpdate() {
    bool t;
    unsigned char x;
    
    x = flags & 0x08;
    onLine = (x != 0) ? true : false ;
}

void ups::lowPowerUpdate() {
    bool t;
    unsigned char x;
    
    x = flags & 0x40;
    lowBatt = (x != 0) ? true : false ;
}


void ups::onBattUpdate() {
    bool t;
    unsigned char x;
    
    x = flags & 0x10;
    onBatt = (x != 0) ? true : false ;
}

string ups::readResponse() {
    int i=0;
    int n=0;

    int lfCount=0;
    char buffer[32];
    int bytesAvailable=0;

    n=ioctl(sfd,FIONREAD,&bytesAvailable);

    bzero(buffer,32);

    while( i < 32 ) {

        n = recv(sfd, &buffer[i], 1, 0);

        if( 0x0a == buffer[i]) {
            lfCount++;
        }

        if( lfCount >=2) {
            buffer[i] = 0x00;
            break;
        }

        if( 0 == lfCount) {
            i++;
        }
    }

    string ret = buffer;
    return(ret);


}

int ups::getCycleCount() {
    return(cycleCount);
}

int ups::getUPSNumeric(char cmd) {
    int n;

    n = send(sfd, &cmd, 1, 0);
    string r = readResponse();
    n = atoi(r.c_str());

    return(n);

}
void ups::updateUPSFlags() {
    char cmd='Q';
    int n;
    int bytesAvailable=0;
    
    if(debug) {
        printf("updateUPSFlags\n");
    }

    n = send(sfd, &cmd, 1, 0);
    string r = readResponse();
    sscanf(r.c_str(),"%x", &n);

    flags=(unsigned char) n &0xff;
    cycleCount++;
    
    onPowerUpdate();
    onBattUpdate();
    lowPowerUpdate();
}

void ups::updateLineFrequency() {
    char cmd='F';
    int n;

    lineFrequency = getUPSNumeric(cmd);

}

void ups::updateLoad() {
    char cmd = 'P';

    outputLoad = getUPSNumeric(cmd);
}

int ups::getLoad() {
    return((maxLoad * outputLoad) / 100);
}

void ups::updateLineVoltage() {
    char cmd='L';
    int n;

    lineVoltage = getUPSNumeric(cmd);
}

int ups::getLineVoltage() {
    return( lineVoltage);
}

void ups::updateMaxLineVoltage() {
    char cmd='M';
    int n;

    maxLineVoltage = getUPSNumeric(cmd);
}
int ups::getMaxLineVoltage() {
   return( maxLineVoltage);
}

void ups::updateMinLineVoltage() {
    char cmd='N';
    int n;

    minLineVoltage = getUPSNumeric(cmd);
}

int ups::getMinLineVoltage() {
   return( minLineVoltage);
}

void ups::updateOutputVoltage() {
    char cmd='O';
    int n;

    outputVoltage = getUPSNumeric(cmd);
}

// Battery level 

void ups::updateBatteryLevel() {
    char cmd='f';
    int n;

    battCharge = getUPSNumeric(cmd);
}

int ups::getBatteryLevel() {
    return( battCharge );
}
// End

void ups::updateRunTime() {
    char cmd='j';
    int n;

    runTime = getUPSNumeric(cmd);
}

int ups::getRunTime() {
    return runTime;
}

void ups::setDebug() {
    debug=true;
}

void ups::clrDebug() {
    debug=false;
}

bool ups::getDebug() {
    return(debug);
}
