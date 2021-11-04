#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

#include <mosquitto.h>

/* MOSQUITTO */
//#define HOST "localhost"
#define HOST "test.mosquitto.org"
#define PORT  1883
#define KEEP_ALIVE 60
#define MSG_MAX_SIZE  512
#define TOPIC_NUM 3

bool session = true;

// const static char* test = "test ";
const static char* pub_to_sub_topic = "pub2sub ";
// const static char* sub_to_pub_topic = "sub2pub ";

/* MQTT-function */
void my_pub_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message){
    
    if(message->payloadlen){
        printf("%s %s", message->topic, (char *)message->payload);
    }else{
        printf("%s (null)\n", message->topic);
    }
    fflush(stdout);
}

void my_pub_connect_callback(struct mosquitto *mosq, void *userdata, int result){
    int i;
    if(!result){
        /* Subscribe to broker information topics on successful connect. */
        //mosquitto_subscribe(mosq, NULL, "sub2pub ", 2);
        printf("on_connect : %s\n", mosquitto_connack_string(result));
    }else{
        printf("fatal error : %s\n", mosquitto_connack_string(result));
        exit(-1);
    }
}

void my_pub_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos){
    int i;
    printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
    for(i=1; i<qos_count; i++){
        printf(", %d", granted_qos[i]);
    }
    printf("\n");
}

void my_pub_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str){
    /* Print all log messages regardless of level. */
    printf("%s\n", str);
}

// <-- motor_control to publish
int getch(void)
{
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( STDIN_FILENO, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
    return ch;
}

int main_menu(void)
{
    int key;

    printf("\n\n");
    printf("-------------------------------------------------\n");
    printf("                    MAIN MENU\n");
    printf("-------------------------------------------------\n");
    printf(" 1. apple                                        \n");
    printf(" 2. person(banana)                               \n");
    printf(" 3. bicycle                                      \n");
    printf(" 4. dog                                          \n");
    printf(" 5. truck                                        \n");

    printf(" a. Turn Left                                    \n");
    printf(" b. Turn Right                                   \n");
    printf(" c. Forward                                      \n");
    printf(" d. backward                                     \n");
    printf(" i. stop                                         \n");
    printf(" I. speed up +10                                 \n");
    printf(" D. speed down -10                               \n");


    printf("-------------------------------------------------\n");
    printf(" q. Motor Control application QUIT               \n");
    printf("-------------------------------------------------\n");
    printf("\n\n");
 
    printf("SELECT THE COMMAND NUMBER ▼\n");

    key=getch();
 
    return key;
}
// --> motor_control to publish


int main()
{
    struct mosquitto *mosq = NULL;
    char buff[MSG_MAX_SIZE];

    int key;
    
    // libmosquitto 초기화
    mosquitto_lib_init();
    
    // mosquitto 인스턴스 생성
    mosq = mosquitto_new(NULL,session,NULL);
    if(!mosq){
        printf("create client failed..\n");
        mosquitto_lib_cleanup();
        return 1;
    }
    
    // callback function
    mosquitto_log_callback_set(mosq, my_pub_log_callback);
    mosquitto_connect_callback_set(mosq, my_pub_connect_callback);
    mosquitto_message_callback_set(mosq, my_pub_message_callback);
    mosquitto_subscribe_callback_set(mosq, my_pub_subscribe_callback); 

    // connect
    if(mosquitto_connect(mosq, HOST, PORT, KEEP_ALIVE)){
        fprintf(stderr, "Unable to connect.\n");
        return 1;
    }
    // loop
    int loop = mosquitto_loop_start(mosq); 
    if(loop != MOSQ_ERR_SUCCESS)
    {
        printf("mosquitto loop error\n");
        return 1;
    }

    // publish의 payload에 들어갈 데이터 설정
    // while(fgets(buff, MSG_MAX_SIZE, stdin) != NULL)
    // {
    //     // publish
    //     mosquitto_publish(mosq, NULL, "pub2sub ", strlen(buff)+1,buff,0,0);
    //     memset(buff,0,sizeof(buff));
    // }

    // publish
    while ((key = main_menu()) != 0)
    {
        *buff = key; 
        *(buff + 1) = '\0';
        printf("buffer : %s\n", buff);
        mosquitto_publish(mosq, NULL, pub_to_sub_topic, strlen(buff)+1, buff, 0, 0);
        if (*buff == 'q'){
            memset(buff,0,sizeof(buff));
            break;
        }
        memset(buff,0,sizeof(buff));
    }
    
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return 0;
}
