#include "s907x.h"
#include "mqtt_test.h"
#include "MQTTPacket.h"
 
#if M_AT_TEST
 

static thread_hdl_t mqtt_test_task_hdl = NULL;
static int mqtt_thread_stop_flag = 0;
extern int mysock;

int transport_getdata(unsigned char* buf, int count);

//AT+MQTT=0,1 start mqtt thread
//AT+MQTT=1,1  stop mqtt thread

void mqtt_test_thread(void *arg)
{
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
  MQTTString receivedTopic;
  int rc = 0;
  
  char buf[200] = {0};
  int buflen = sizeof(buf);
  
//  int mysock = 0;
  MQTTString topicString = MQTTString_initializer;
  
  int payloadlen_in;
  unsigned char *payload_in;
  unsigned short msgid = 1;
  
  int subcount;
  int granted_qos = 0;
  
  unsigned char sessionPresent, connack_rc;
  unsigned short submsgid;
  int len = 0;
  int req_qos = 1;
  unsigned char dup;
  int qos;
  unsigned char retained;

//  char *host = "m2m.eclipse.org";
//  int port = 1883;
  
  char *host = "192.168.100.177";
  int port = 61613;
  
  uint8_t  msgtypes = CONNECT;
  uint32_t curtick = xTaskGetTickCount();
  
  while(1){
    printf("waiting to be connected with some AP...\n\r");
    wl_os_mdelay(2000);
    
    s907x_link_info_t link_infor;
    s907x_wlan_get_link_infor(&link_infor);
    
    if(link_infor.is_connected){
      break;
    }
    
    if(mqtt_thread_stop_flag){
        goto exit;
    }
  }
  printf("socket connect to server");
  mysock = transport_open(host, port);
  printf("current socket for mqtt is %d\n\r", mysock);
  
  if(mysock < 0)
    return;
  
  //hal_printf("Sending to hostname %s port %d\n", host, port);
//  data.clientID.cstring = "b31e142f1af84ed486cfcb3dd5f36857";
  data.clientID.cstring = "30c901c9802443a9969678096a270581";
  data.keepAliveInterval = 50;
  data.cleansession = 1;
//  data.username.cstring = "";
//  data.password.cstring = "";
  
  data.username.cstring = "admin";
  data.password.cstring = "password";
  data.MQTTVersion = 4;
  
  
  while(1)
  {
    if((xTaskGetTickCount() - curtick) >(data.keepAliveInterval/2*1000))
    {
      if(msgtypes == 0)
      {
        curtick = xTaskGetTickCount();
        msgtypes = PINGREQ;
        
      }
    }

    switch(msgtypes)
    {
      case CONNECT:
          
        len = MQTTSerialize_connect(buf, buflen, &data);        
        rc = transport_sendPacketBuffer(mysock, (unsigned char*)buf, len);
        if (rc == len)
          printf("send CONNECT Successfully\n\r");
        else
          printf("send CONNECT failed\n\r");                
        printf("MQTT concet to server!\n\r");
        msgtypes = 0;
        
      break;
                                            
      case CONNACK:
          
        if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) != 1 || connack_rc != 0){
          printf("Unable to connect, return code %d\n", connack_rc);
        }
        else{
          printf("MQTT is concet OK!\n\r");
        }
        msgtypes = SUBSCRIBE;
      break;

      case SUBSCRIBE:
          
        topicString.cstring = "ledtest";
        len = MQTTSerialize_subscribe(buf, buflen, 0, msgid, 1, &topicString, &req_qos);
        rc = transport_sendPacketBuffer(mysock, (unsigned char*)buf, len);
        if (rc == len)
          printf("send SUBSCRIBE Successfully\n\r");
        else
          printf("send SUBSCRIBE failed\n\r");        
        printf("client subscribe:[%s]",topicString.cstring);
        msgtypes = 0;
        
      break;
      
      case SUBACK:
          
        rc = MQTTDeserialize_suback(&submsgid, 1, &subcount, &granted_qos, buf, buflen);                                                        
        printf("granted qos is %d\n\r", granted_qos);
        msgtypes = 0;
        
      break;
                                      
      case PUBLISH:
          
        rc = MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &receivedTopic, &payload_in, &payloadlen_in, buf, buflen);
        printf("message arrived : %s\n", payload_in);
        if(strstr(payload_in,"on"))
        {
          printf("LED on!!");
        }
        else if(strstr(payload_in,"off"))
        {
          printf("LED off!!");
        }
        if(qos == 1)
        {
          printf("publish qos is 1,send publish ack.");
          memset(buf,0,buflen);
          len = MQTTSerialize_ack(buf,buflen,PUBACK,dup,msgid);   //publish ack
          rc = transport_sendPacketBuffer(mysock, (unsigned char*)buf, len);
          if (rc == len)
            printf("send PUBACK Successfully");
          else
            printf("send PUBACK failed");
        }
        msgtypes = 0;
        
      break;

      case PUBACK:
        printf("PUBACK!");
        msgtypes = 0;
      break;
      
      case PUBREC:
        printf("PUBREC!");     //just for qos2
      break;
      
      case PUBREL:
        printf("PUBREL!");        //just for qos2
      break;
      
      case PUBCOMP:
        printf("PUBCOMP!");        //just for qos2
      break;
      
      case PINGREQ:
          
        len = MQTTSerialize_pingreq(buf, buflen);
        rc = transport_sendPacketBuffer(mysock, (unsigned char*)buf, len);
        if (rc == len)
          printf("send PINGREQ Successfully\n");
        else
          printf("send PINGREQ failed\n");
        
        printf("time to ping mqtt server to keep alive!");
        msgtypes = 0;
        
        break;
        
      case PINGRESP:
          
        printf("mqtt server Pong");
        msgtypes = 0;
          
      break;
    }
    
    memset(buf, 0, buflen);
    rc = MQTTPacket_read(buf, buflen, transport_getdata);
//    
    if(rc >0)
    {
      printf("MQTTPacket_read returns %d\n\r", rc);
      msgtypes = rc;
    }
//    gpio_write(&gpio_led, !gpio_read(&gpio_led));
    if(mqtt_thread_stop_flag){
        goto exit;
    }
  }

exit:
  transport_close(mysock);
  printf("mqtt thread exit, %d.\n\r", (int)mqtt_test_task_hdl);
  thread_hdl_t temptask_hdl = mqtt_test_task_hdl;
  mqtt_test_task_hdl = NULL;
  mqtt_thread_stop_flag = 0;
  wl_destory_thread(temptask_hdl);
  
}


hal_test_name_map_t mqtt_test_map[] = 
{
  {0, "mqtt test start"},
  {1, "mqtt test stop"},

};

void mqtt_test_start(hal_test_t *test)
{
  ASSERT(test);

  if(test->arg[0]) {
    if(mqtt_test_task_hdl == NULL){
      mqtt_test_task_hdl = wl_create_thread("mqtt test", MQTT_TEST_STACK_SZ, MQTT_TEST_PRIO, (thread_func_t)mqtt_test_thread, NULL);
    }
    else{
      HAL_TEST_DBG("mqtt test thread has been established!\n\r");
    }
  }
}

void mqtt_test_stop(hal_test_t *test)
{
  ASSERT(test);

  if(test->arg[0]) {
    if(mqtt_test_task_hdl != NULL){
      mqtt_thread_stop_flag = 1;
    }
    else{
      HAL_TEST_DBG("mqtt test thread not start yet!\n\r");
    }
  }
}
 
void mqtt_test(hal_test_t *test)
{
  ASSERT(test);
  ASSERT(test->no < (sizeof(mqtt_test_map)/sizeof(hal_test_name_map_t)));

  HAL_TEST_DBG("test no %d = %s\n",test->no, mqtt_test_map[test->no].name);
     
  switch(test->no) 
  {
    case MQTT_TEST_START:    
      mqtt_test_start(test);
    break;

    case MQTT_TEST_STOP:
      mqtt_test_stop(test);
    break;

    default: break;
  }
}

#endif




