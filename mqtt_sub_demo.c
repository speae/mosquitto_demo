#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <mosquitto.h>

#define mqtt_host "test.mosquitto.org"
#define mqtt_port 1883
#define MQTT_TOPIC "myTopic"

static int run = 1;

void handle_signal(int s)
{
   run = 0;
}

void connect_callback(struct mosquitto *mosq, void *obj, int result)
{
   printf("connect callback, rc=%d\n", result);
}

void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
   /*printf("got message '%.*s' for topic '%s'\n", message->payloadlen, (char*) message->payload, message->topic);*/
	printf("receive message(%s) : %p",message->topic, message->payload);
}

int main(int argc, char *argv[])
{
   uint8_t reconnect = true;
   //char clientid[24];//id를 사용하는 경우
   struct mosquitto *mosq;
   int rc = 0;

   signal(SIGINT, handle_signal);
   signal(SIGTERM, handle_signal);

   mosquitto_lib_init();

   //메모리 초기화
   //memset(clientid, 0, 24);//맨 앞부터 0을 24개 삽입 (초기화)
   //snprintf(clientid, 23, "mysql_log_%d", getpid());//23길이의 clientid에 pid를 가진 문자열 삽입
   // mosq = mosquitto_new(clientid, true, 0);//mosquitto 구조체 생성 <-
   mosq = mosquitto_new(NULL, true, 0);//mosquitto 구조체 생성

   if(mosq){
      mosquitto_connect_callback_set(mosq, connect_callback);
      mosquitto_message_callback_set(mosq, message_callback);

      rc = mosquitto_connect(mosq, mqtt_host, mqtt_port, 60);//mosqutiio 서버와 연결

      mosquitto_subscribe(mosq, NULL, MQTT_TOPIC, 0);//subscribe

      while(run){
         rc = mosquitto_loop(mosq, -1, 1);
         if(run && rc){
            printf("connection error!\n");
            sleep(10);
            mosquitto_reconnect(mosq);
         }
      }
      mosquitto_destroy(mosq);
   }

   mosquitto_lib_cleanup();

   return rc;
}