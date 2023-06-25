#include <PubSubClient.h>

#define SLAVE_UPDATES_TOPIC "/traffic/slave_feedback"
#define MASTER_UPDATES_TOPIC "/traffic/updates"
#define SIGNAL_PUBLISH_TOPIC "/traffic/signals"

//used to setup MQTT server and callback function
void mqtt_setup();

//for connecting to MQTT broker
void reconnect();

//callback function is called when a message is received
void mqtt_callback(char* topic, byte* payload, unsigned int length);

void parse_mqtt_signal_commands(byte* payload);

void publish_state();

bool mqtt_master_online();

bool pubsubloop();