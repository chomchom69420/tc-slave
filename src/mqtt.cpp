#include "mqtt.h"
#include "credentials.h"
// #include "timer.h"
#include "delay.h"
#include "signals.h"
#include "control.h"
#include <WiFi.h>
#include <ArduinoJson.h>

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// SlaveStates slave_states;
String sig_pub_topic = SIGNAL_PUBLISH_TOPIC;

int mode_select;

// Number of times the slaves got the signal - reset only at first time
int n_signal;

// Stores if mqtt master is online - assume offline by default
int master_online = 0;

// connects to the WiFi access point
void connectToWiFi()
{
  Serial.print("Connecting to ");
  String ssid = WIFI_SSID;
  String pwd = PWD;
  WiFi.begin(ssid.c_str(), pwd.c_str());
  Serial.println(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("Connected.");
}

// used to setup MQTT server and callback function
void mqtt_setup()
{
  connectToWiFi();
  // Set server and port for MQTT broker
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  // set the callback function
  mqttClient.setCallback(mqtt_callback);
  mqttClient.setKeepAlive(1);
}

// for connecting to MQTT broker
void reconnect()
{
  String clientId = "ESP32Slave-";
  String username = "traffic-controller";
  String password = "f3JuzTfh3utBpIow";

  char lastwilltopic[20];

  String lastwillmessage = "{\"status\":\"Offline\"}";

  Serial.println("Connecting to MQTT Broker...");
  sprintf(lastwilltopic, "/status/slave-%d", SLAVE_ID);
  while (!mqttClient.connected())
  {
    Serial.println("Reconnecting to MQTT Broker..");

    // clientId += String(random(0xffff), HEX);
    clientId += String(SLAVE_ID);
    if (mqttClient.connect(clientId.c_str(), username.c_str(), password.c_str(), lastwilltopic, 1, true, lastwillmessage.c_str()))
    {
      Serial.println("Connected.");
      // subscribe to topic
      mqttClient.subscribe(SIGNAL_PUBLISH_TOPIC, 1);

      // Each slave connects to the master's status topic
      mqttClient.subscribe("/status/master", 1);
    }
  }

  // Send a publish message to lastwilltopic after connecting
  mqttClient.publish(lastwilltopic, "{\"status\":\"Online\"}", true);
}

// mqtt_callback function is called when a message is received
void mqtt_callback(char *topic, byte *payload, unsigned int length)
{

  Serial.print("Callback occured\n");
  Serial.print("Topic: ");
  Serial.print(topic);
  Serial.print("\n");

  // String recv_payload = String((char*)payload);

  String master_topic = "/status/master";
  String control_topic = "/traffic/control";

  if (strcmp(topic, sig_pub_topic.c_str()) == 0)
  {
    // Serial.print("here\n");s
    parse_mqtt_signal_commands(payload);
  }
  else if (strcmp(topic, master_topic.c_str()) == 0)
  {
    const int capacity = JSON_OBJECT_SIZE(5);
    StaticJsonBuffer<capacity> jb;
    JsonObject &parsed = jb.parseObject(payload);

    String master_status = parsed["status"];
    // Serial.println(recv_payload);

    String offline = "Offline";
    String online = "Online";
    if (strcmp(master_status.c_str(), offline.c_str()) == 0)
    {
      // Make master_online = 0
      master_online = 0;
      Serial.println("Using the fsm...");
    }
    else if (strcmp(master_status.c_str(), online.c_str()) == 0)
    {
      Serial.println("Stopping fsm usage... Returning to master-slave mode");
      master_online = 1;

      //Wait for next master command
      // signals_off();
      // timer_update(0, 0);
    }
  }
  /*
  * If the topic is control topic
  */
  else if(strcmp(topic, control_topic.c_str()) == 0)
  {
    //Make the jsonobject for sending to parse 

    const int capacity = JSON_OBJECT_SIZE(6);
    StaticJsonBuffer<capacity> jb;
    JsonObject &parsed = jb.parseObject(payload);
    setControlMode(parsed);
  }
}

void parse_mqtt_signal_commands(byte *payload)
{
  Serial.print("Starting to parse commands on slave...\n");
  const int capacity = JSON_OBJECT_SIZE(50);
  StaticJsonBuffer<capacity> jb;
  JsonObject &parsed = jb.parseObject(payload);
  if (!parsed.success())
  {
    Serial.println("Parsing failed");
    delay(5000);
    return;
  }

  setEnvironment(parsed); //Set the environment
  setSlave(parsed);

  executeCommandedState_modeDependent();
}

void publish_state()
{
  // Publishing state to MQTT server
  char payload[500];
  const int capacity = JSON_OBJECT_SIZE(10);
  StaticJsonBuffer<capacity> jb;
  // Create a JsonObject
  JsonObject &obj = jb.createObject();

  obj["id"] = SLAVE_ID;
  obj["s"] = getPrimaryState();
  JsonObject &timers = obj.createNestedObject("timers");
  timers["red"] = getTimerValues(LampID::PRIMARY, LampState::RED);
  timers["green"] = getTimerValues(LampID::PRIMARY, LampState::GREEN);
  obj["t_elapsed"] = getElapsedTime(LampID::PRIMARY);

  // Serializing into payload
  obj.printTo(payload);
  mqttClient.publish(SLAVE_UPDATES_TOPIC, payload);
}

bool pubsubloop()
{
  return mqttClient.loop();
}

bool mqtt_master_online()
{
  return master_online ? true : false;
}

void mqtt_log(String log_message)
{
  char payload[1000];
  
  //Remove the species checking code based on context. When uploading code for master, slave, lcp
  const int capacity = JSON_OBJECT_SIZE(6);  //Required is 5 --> take one more 
  StaticJsonBuffer<capacity> jb;
  JsonObject &obj = jb.createObject();
  obj["species"]="slave";
  // obj["species"]="master"; 
  obj["slave_id"]=SLAVE_ID;
  // obj["panel_id"]=PANEL_ID;
  obj["log"]=log_message;

  // Serializing into payload
  obj.printTo(payload);
  mqttClient.publish("/status/logs", payload);
}