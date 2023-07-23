#include <PubSubClient.h>
#include "configurations.h"

#define SLAVE_UPDATES_TOPIC "/traffic/slave_feedback"
#define MASTER_UPDATES_TOPIC "/traffic/updates"
#define SIGNAL_PUBLISH_TOPIC "/traffic/signals"

//used to setup MQTT server and callback function
void mqtt_setup();

//for connecting to MQTT broker
void mqtt_reconnect();

//callback function is called when a message is received
void mqtt_callback(char* topic, byte* payload, unsigned int length);

void parse_mqtt_signal_commands(byte* payload);

void mqtt_publish_state();

bool mqtt_master_online();

bool mqtt_pubsubloop();

/*
* This function publishes to the /status/logs topic on MQTT
* Species: master, slave, GUI, app, lcp   (lcp --> local control panel)
* slave_id : (only required for slave)
* panel_id : (only required for panel)
* log_message: message that is to be logged  (TODO: set a character limit on this)
* Payload format:
{
“species”: “”
(if slave) “slave_id”: ,
(if panel) “panel_id”: ,
“timestamp”: “”,
“log” : “”
}
* TODO: Implement the timestamp feature in logging 
*/
void mqtt_log(String log_message);