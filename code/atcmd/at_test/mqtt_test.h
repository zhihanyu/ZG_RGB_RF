#ifndef __MQTT_AT_TEST__
#define __MQTT_AT_TEST__


typedef enum mqtt_test_item_
{
    MQTT_TEST_START,
    MQTT_TEST_STOP
        
}mqtt_test_item_e;

void mqtt_test(hal_test_t *test);

#endif
