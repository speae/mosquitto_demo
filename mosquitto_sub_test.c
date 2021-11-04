#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

#ifndef __WIN32__
#include <unistd.h>
#endif

#include <mosquitto.h>
#include <mqtt_protocol.h>

/* MOTOR_CONTROL-value */
#define VERSION_MAJOR 1
#define VERSION_MINOR 0

#define DEVLENGTH 128
#define BAUDLENGTH 32

#define RX 1
#define TX 2

static const char *optString = "vd:b:rt";

typedef struct {
	char devpath[DEVLENGTH];	/* -d serial device */
	int baudrate;			/* baudrate to use */
	int fd_handle;				/* file descriptor */
    int fd_from_uart;
    int fd_to_uart;
} Settings_t;
Settings_t MySettings;

struct termios old_deviceOptions, new_deviceOptions;

long rxcount = 0;
long txcount = 0;

char* arg_chk;
    
// #define  FIFO_FROM_UART "/tmp/from_uart_fifo"
// #define  FIFO_TO_UART   "/tmp/to_uart_fifo"
#define  BUFF_SIZE  1024

char uart_buff[BUFF_SIZE];

/* MQTT-value */
#define HOST "test.mosquitto.org"
// #define HOST "localhost"
#define PORT 1883
#define KEEP_ALIVE 60
#define MSG_MAX_SIZE 512
#define TOPIC_NUM 3

bool session = true;
// const static char* test = "test ";
const static char* pub_to_sub_topic = "pub2sub ";
// const static char* sub_to_pub_topic = "sub2pub ";

char mqtt_buff[BUFF_SIZE];

/* MOTOR_CONTROL-function */
int openport(Settings_t *settings)
{
    strncpy(settings->devpath, "/dev/ttyTHS2", DEVLENGTH);
	settings->baudrate = 115200;
    settings->fd_handle = open(settings->devpath, O_RDWR | O_NOCTTY | O_NDELAY);
    if (settings->fd_handle < 0) {
        char errorString[0xFF];
        strerror_r(errno, errorString, sizeof(errorString));
        fprintf(stderr, "Open device failed: Unable to open device file %s. Error: %s\n", settings->devpath, errorString);
        return -1;
    }

    return 0;
}

int configureport(Settings_t *settings)
{
    // Modify the settings on the serial device (baud rate, 8n1, receiver enabled, ignore modem status, no flow control) and apply them
    tcgetattr(settings->fd_handle, &old_deviceOptions);
    memset (&new_deviceOptions, 0, sizeof new_deviceOptions);
    
    /* Set the baud rates... */
    new_deviceOptions.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
    new_deviceOptions.c_iflag = IGNPAR;
    new_deviceOptions.c_oflag = 0;
    new_deviceOptions.c_lflag = 0;
    new_deviceOptions.c_cc[VTIME] = 128; 
    new_deviceOptions.c_cc[VMIN]  = 0;
    
    tcflush(settings->fd_handle, TCIFLUSH);
    if (tcsetattr(settings->fd_handle, TCSANOW, &new_deviceOptions) != 0) {
        close(settings->fd_handle);
        char errorString[0xFF];
        strerror_r(errno, errorString, sizeof(errorString));
        fprintf(stderr, "Create failed: Unable to set options on device. Error: %s\n", errorString);
        return -1;
    }

    return 0;
}

void* tx_function(void *data)
{
    char testchar;
	
	// Begin writing
	testchar = *arg_chk;
    printf("testchar : %c\n", testchar);

    // write the data
    switch(testchar)
    {
        case 'a':
            printf("a\n");               
            *mqtt_buff = 'a';
            write( MySettings.fd_handle, mqtt_buff, 1 );
            break;

        case 'b':
            printf("b\n");               
            *mqtt_buff = 'b';
            write( MySettings.fd_handle, mqtt_buff, 1 );
            break;

        case 'c':
            printf("c\n");               
            *mqtt_buff = 'c';
            write( MySettings.fd_handle, mqtt_buff, 1 );
            break;

        case 'd':
            printf("d\n");               
            *mqtt_buff = 'd';
            write( MySettings.fd_handle, mqtt_buff, 1 );
            break;  

        case 'i':
            printf("i\n");               
            *mqtt_buff = 'i';
            write( MySettings.fd_handle, mqtt_buff, 1 );
            break;  

        case 'q':
            printf("exit write.\n");
            tcsetattr(MySettings.fd_handle, TCSANOW, &old_deviceOptions);
            close(MySettings.fd_handle);
            break;

        default :
            printf("Wrong key ..... try again\n");
            break;
    }
    txcount++;
    
    //usleep(50);

    // print status
    if (txcount % 100 == 0) {
        fprintf(stderr, "tx: %ld\n", txcount);
    }
}

void* rx_function(void *data)
{
	char testchar;

    testchar = *arg_chk;
	
    // Begin reading
	fprintf(stderr, "Waiting for data...\n");
    
	// Wait until there is data to read or a timeout happens
    // *uart_buff = *mqtt_buff;
    read(MySettings.fd_handle, mqtt_buff, 1);
    printf("uart handling : %s\n", mqtt_buff);
    printf("command recived : %c\n", testchar);
    memset(uart_buff, 0, BUFF_SIZE);
    memset(mqtt_buff, 0, BUFF_SIZE);

    rxcount++;
    
    // print status
    if (rxcount % 100 == 0) {
        fprintf(stderr, "rx: %ld\n", rxcount);
    }
}

void cmd_function(){
    Settings_t* settings;
    pthread_t p_thread;
    int thr_id;
    int tx = 1;
    int rx = 2;
    
    // if ( -1 == ( settings->fd_from_uart = open( FIFO_FROM_UART, O_RDWR)))
    // {
    //     if ( -1 == mkfifo( FIFO_FROM_UART, 0666))
    //     {
    //         perror( "mkfifo() run error");
    //         exit( 1);
    //     }

    //     if ( -1 == ( settings->fd_from_uart = open( FIFO_FROM_UART, O_RDWR)))
    //     {
    //         perror( "open() error");
    //         exit( 1);
    //     }
    // }
    // if ( -1 == ( settings->fd_to_uart = open( FIFO_TO_UART, O_RDWR)))
    // {
    //     if ( -1 == mkfifo( FIFO_TO_UART, 0666))
    //     {
    //         perror( "mkfifo() run error");
    //         exit( 1);
    //     }

    //     if ( -1 == ( settings->fd_to_uart = open( FIFO_TO_UART, O_RDWR)))
    //     {
    //         perror( "open() error");
    //         exit( 1);
    //     }
    // }

    if (openport(&MySettings) != 0) {
        printf("Can't file open.");
		exit(-1);
	}

    // Reset the serial device file descriptor for non-blocking read / write
    if (configureport(&MySettings) != 0) {
		exit(-1);
	}

    // --> add thread
	thr_id = pthread_create(&p_thread, NULL, tx_function, (void *)&tx);
    if (thr_id < 0)
    {
        perror("tx thread create error.");
        exit(0);
    }

    thr_id = pthread_create(&p_thread, NULL, rx_function, (void *)&rx);
    if (thr_id < 0)
    {
        perror("tx thread create error.");
        exit(0);
    }

    pthread_join(p_thread, (void*)&tx);
    pthread_join(p_thread, (void*)&rx);
	
    tcsetattr(settings->fd_handle, TCSANOW, &old_deviceOptions);
    close(settings->fd_handle);
}

/* MQTT-function */
void my_sub_log_callback(struct mosquitto* mosq, void* userdata, int level, const char* str)
{
    printf("%s\n", str);
}

void my_sub_connect_callback(struct mosquitto* mosq, void* userdata, int result){
    int i;
    if (!result)
    {
        //mosquitto_property_add_string(&mosq_prop, MQTT_PROP_CONTENT_TYPE, "application/json");
        printf("on_connect : %s\n", mosquitto_connack_string(result));
        mosquitto_subscribe(mosq, NULL, pub_to_sub_topic, 2);
    
    }
    else
    {
        printf("fatal error : %s\n", mosquitto_connack_string(result));
        exit(-1);
    }   
}

void my_sub_subscribe_callback(struct mosquitto* mosq, void* userdata, int mid, int qos_count, const int* granted_qos)
{
    int i;
    printf("Subscribed (mid : %d): %d", mid, granted_qos[0]);
    for (i = 0; i < qos_count; i++)
    {
        printf(", %d", granted_qos[i]);
    }
    printf("\n");   
}

void my_sub_msg_callback(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message){
    if (message->payloadlen)
    {
        arg_chk = message->payload;
        if (*arg_chk == 'q'){
            printf("publish to subscribe end command : %s\n", (char*)message->payload);
            mosquitto_disconnect(mosq);
            mosquitto_destroy(mosq);
            mosquitto_lib_cleanup();
            exit(-1);
        }
        else
        {
            printf("received messages : %s %s\n", message->topic, (char*)message->payload);
            cmd_function();
        }
    }
    else
    {
        printf("%s (null)\n", message->topic);
    }
    fflush(stdout);
}

int main(int argc, char* argv[])
{
    struct mosquitto* mosq = NULL;
    char buff[MSG_MAX_SIZE];
   
    mosquitto_lib_init();

    mosq = mosquitto_new(NULL, session, NULL);
    if (!mosq)
    {
        printf("create client failed..\n");
        mosquitto_lib_cleanup();
        return 1;
    }
    
    mosquitto_log_callback_set(mosq, my_sub_log_callback);
    mosquitto_connect_callback_set(mosq, my_sub_connect_callback);
    mosquitto_subscribe_callback_set(mosq, my_sub_subscribe_callback);
    mosquitto_message_callback_set(mosq, my_sub_msg_callback);
    
    if(mosquitto_connect(mosq, HOST, PORT, 60)){
        fprintf(stderr, "Unable to connect.\n");
        return 1;
    };
    
    // int loop = mosquitto_loop_start(mosq);
    // if (loop != MOSQ_ERR_SUCCESS)
    // {
    //     printf("mosquitto loop error.\n");
    //     return 1;
    // }
    
    // while (fgets(buff, MSG_MAX_SIZE, stdin) != NULL)
    // {
    //     mosquitto_publish(mosq, NULL, sub_to_pub_topic, strlen(buff) + 1, buff, 0, 0);
    //     memset(buff, 0, sizeof(buff));
    // }
       
    mosquitto_loop_forever(mosq, -1, 1);

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return 0;
}