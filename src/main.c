#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define GETSTAT "{\"method\": \"getPilot\", \"params\": {}}"
#define GETSYSCONF "{\"method\": \"getSystemConfig\", \"params\": {}}"
#define POWEROFF "{\"method\": \"setPilot\", \"params\": {\"state\": false}}"
#define POWERON "{\"method\": \"setPilot\", \"params\": {\"state\": true}}"
/* 
#define CHCOLOR "{\"method\": \"setPilot\", \"params\": {\"r\": RED, \"g\": GREEN, \"b\": BLUE, \"dimming\": 50}}" 
#define CHBRI "{\"method\": \"setPilot\", \"params\": {\"dimming\": DIM}}" 
#define CHSCENE "{\"method\": \"setPilot\", \"params\": {\"sceneId\": SCENEID}}"
*/

typedef struct color{
    int red;
    int green;
    int blue;
} color;

enum state{
    off = 0,
    on = 1
};

static color *_get_color();
static int _get_scene();
static void _discover();
static void _blink(int, struct sockaddr_in, int, int, enum state);
static void _broadcast_blink(int);

int main(int argc, char **argv){
    int sd, dst_len, choise, ret, tmp;
    char *buff, *baddr, ctmp;  
    color *col;
    struct sockaddr_in dst; 
    enum state s;

    if(argc < 2){
        baddr = (char *) malloc(sizeof(char) * 16);
        printf("No bulb on argv[1], do you want to discover the bulb on network? (y/n): ");
        scanf(" %c", (char *) &choise);

        while(choise != 121 && choise != 110){
            printf("Wrong choise. Do you want to discover the bulb on network? (y/n): ");
            scanf(" %c", (char *) &choise);
        }

        if(choise == 121){
            _discover();
        }

        printf("Enter bulbs address you want use: ");
        scanf(" %s", baddr);
    }else{
        baddr = argv[1];
    }

    buff = (char *) malloc(sizeof(char) * 2048);

    sd = socket(AF_INET, SOCK_DGRAM, 0);

    if(sd < 0){
        exit(1);
    }

    dst_len = sizeof(dst);
    dst.sin_family = AF_INET;
    dst.sin_addr.s_addr = inet_addr(baddr);
    dst.sin_port = htons(38899);

    printf("Choose an action: (type \"h\" for list all)\n");

    while(1){
        memset(buff, 0, 2048);

        printf(" > ");
        scanf(" %c", (char *) &choise);

        switch(choise){
            case 48: strcpy(buff, GETSYSCONF); break;
            case 49: strcpy(buff, GETSTAT); break; 
            case 50: strcpy(buff, POWERON); break;
            case 51: strcpy(buff, POWEROFF); break; 
            case 52: col = _get_color(); 
                    sprintf(buff, "{\"method\": \"setPilot\", \"params\": {\"r\": %d, \"g\": %d, \"b\": %d, \"dimming\": 50}}", 
                    col -> red, col -> green, col -> blue); 
                    break;
            case 53: printf("Enter brightness percentage: ");
                    scanf(" %d", &tmp);
                    sprintf(buff, "{\"method\": \"setPilot\", \"params\": {\"dimming\": %d}}", tmp);
                    break;
            case 54: tmp = _get_scene();
                    sprintf(buff, "{\"method\": \"setPilot\", \"params\": {\"sceneId\": %d}}", tmp);
                    break;
            case 55: printf("How many times do you want your bulb blinks? "); scanf(" %d", &tmp); 
                    printf("Is your bulb switched on? "); scanf(" %c", &ctmp); 
                    if(ctmp == 'y'){
                        s = on;
                    }else{
                        s = off;
                    }
                    _blink(sd, dst, dst_len, tmp, s);
            case 56: printf("How many times do you want your bulb blinks? "); scanf(" %d", &tmp);
                    _broadcast_blink(tmp);
            case 104: printf("   0: Get system configuration info.\n \
  1: Get the status of the light.\n \
  2: Power ON the light.\n \
  3: Power OFF the light.\n \
  4: Change light color.\n \
  5: Change brightness.\n \
  6: Change scene.\n \
  7: Make the light blink.\n \
  8: Make all the light blink\n"); 
                    break;
        }

        if(choise > 47 && choise < 55){
            ret = sendto(sd, buff, strlen(buff) + 1, 0, (struct sockaddr *) &dst, dst_len);
            
            if(ret < 0){
                exit(3);
            }
            
            memset(buff, 0, 2048);
            ret = recvfrom(sd, buff, 2048, 0, (struct sockaddr *) &dst,  (socklen_t *) &dst_len);

            if(ret < 0){
                exit(4);
            }

            printf("LIGHT RESPONSE: %s\n", buff);
        }
    }

    close(sd);
    
    return 0;
}

static int _get_scene(){
    char scene[2]; 

    do{
        printf("Enter a scene ID (type L to list all): ");
        scanf(" %s", scene);

        if(strcmp(scene, "L") == 0){
            printf("Default scene are:\n \
- 1: Ocean.\n \
- 2: Romance.\n \
- 3: Sunset.\n \
- 4: Party.\n \
- 5: Fireplace.\n \
- 6: Wellcoming.\n \
- 7: Forest.\n \
- 8: Pastel colours\n \
- 9: Waking up\n \
- 10: Sleep time\n");
        }
    }while(strcmp(scene, "L") == 0);

    return atoi(scene); 
}

static color *_get_color(){
    color *color = (struct color *) malloc(sizeof(struct color));

    printf(" Red > "); 
    scanf(" %d", &color -> red);

    printf(" Green > "); 
    scanf(" %d", &color -> green);

    printf(" Blue > "); 
    scanf(" %d", &color -> blue);

    return color;
}

static void _discover(){
    int sd, dst_len, ret, bc = 1;
    char buff[512], baddr[16];
    struct timeval timeout;
    struct sockaddr_in dst; 
    fd_set readfds;

    memset(buff, 0, 512);
    memset(baddr, 0, 16);

    printf("Enter broadcast address: ");
    scanf(" %s", baddr);

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    
    if(sd < 0){
        exit(1);
    }
    
    ret = setsockopt(sd, SOL_SOCKET, SO_BROADCAST, &bc, sizeof(bc));

    if(ret < 0){
        exit(2);
    }

    dst_len = sizeof(dst);
    dst.sin_family = AF_INET;
    dst.sin_addr.s_addr = inet_addr(baddr);
    dst.sin_port = htons(38899);

    timeout.tv_sec = 2; 

    FD_ZERO(&readfds);
    FD_SET(sd, &readfds);    
    
    ret = sendto(sd, GETSTAT, strlen(GETSTAT), 0, (struct sockaddr *) &dst, dst_len);

    if(ret < 0){
        exit(3);
    }

    while(select(sd + 1, &readfds, NULL, NULL, &timeout) > 0){
        ret = recvfrom(sd, buff, 2048, 0, (struct sockaddr *) &dst,  (socklen_t *) &dst_len);
        
        if(ret < 0){
            exit(4);
        }

        if(strstr(buff, "\"method\":\"getPilot\"") != NULL){
            printf("Bulb found on: %s\n", inet_ntoa(dst.sin_addr));
        }

        timeout.tv_sec = 2; 
    }

    close(sd);
}

static void _blink(int sd, struct sockaddr_in dst, int dst_len, int blink_counter, enum state s){
    char *buff[2];
    int i;
    enum state curr = !s;

    for(i = 0; i < 2; i++){
        buff[i] = (char *) malloc(sizeof(char) * 51);

        if(i == 0){
            strcpy(buff[i], POWEROFF);
        }else{
            strcpy(buff[i], POWERON);
        }
    }
    
    for(i = 0; i < blink_counter * 2; i++){
        sendto(sd, buff[curr], strlen(buff[curr]) + 1, 0, (struct sockaddr *) &dst, dst_len);

        curr = !curr;

        sleep(1);
    } 
}

static void _broadcast_blink(int blink_counter){
    int sd, dst_len, ret, bc = 1;
    char baddr[16];
    struct sockaddr_in dst; 

    printf("Enter broadcast address: ");
    scanf(" %s", baddr);

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    
    if(sd < 0){
        exit(1);
    }
    
    ret = setsockopt(sd, SOL_SOCKET, SO_BROADCAST, &bc, sizeof(bc));

    if(ret < 0){
        exit(2);
    }

    dst_len = sizeof(dst);
    dst.sin_family = AF_INET;
    dst.sin_addr.s_addr = inet_addr(baddr);
    dst.sin_port = htons(38899);

    _blink(sd, dst, dst_len, blink_counter, on);

    close(sd);
}
