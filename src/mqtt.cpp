#include "mqtt.h"
#include "credentials.h"
#include "configurations.h"
#include "timer.h"
#include "signals.h"
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

  if (strcmp(topic, sig_pub_topic.c_str()) == 0)
  {
    // Serial.print("here\n");s
    parse_mqtt_signal_commands(payload);
  }

  String master_topic = "/status/master";
  if (strcmp(topic, master_topic.c_str()) == 0)
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

      // Reset everything and wait for next master command
      signals_off();
      timer_update(0, 0);
    }
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

  char s[10];
  sprintf(s, "S%d", SLAVE_ID);
  int state = parsed[s];

  sprintf(s, "t%d", SLAVE_ID);
  JsonObject &timer = parsed[s];
  int timer_red = timer["red"];
  int timer_green = timer["green"];

  // Serial.print("\nRed=");
  // Serial.print(timer_red);
  // Serial.print("\nGreen=");
  // Serial.print(timer_green);

  mode_select = parsed["mode"];

  timer_update(timer_red, timer_green);

  int prev_state = signals_get_state();

  if (state == SLAVE_STATE_RED)
  {
    // Set the state of the FSM so that it can continue on disconnection
    signals_set_state(SLAVE_STATE_RED);

    //..but also manually turn on the red lamp
    signals_red(1);

    //..and manually turn off the green lamps
    signals_green_forward(0);
    signals_green_left(0);
    signals_green_right(0);
  }
  else if (state == SLAVE_STATE_GREEN)
  {
    // Set mode for later usage in FSM
    signals_set_mode(mode_select);

    // Set the state of the FSM for later if master is disconnected
    signals_set_state(SLAVE_STATE_GREEN);

    //..but also manually turn ON/OFF the lights
    signals_red(0);

    if (mode_select == MODE_MULTIDIRECTION)
    {
      signals_green_forward(1);
      signals_green_left(1);
      signals_green_right(1);
    }
    else if (mode_select == MODE_STRAIGHT_ONLY)
    {
      signals_green_forward(1);
      signals_green_left(0);
      signals_green_right(0);
    }
  }

  if (prev_state != state)
  {
    // Restart timers
    timer_start(state);
  }
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
  obj["s"] = signals_get_state();
  JsonObject &timers = obj.createNestedObject("timers");
  timers["red"] = timer_get_time(SLAVE_STATE_RED);
  timers["green"] = timer_get_time(SLAVE_STATE_GREEN);
  obj["t_elapsed"] = timer_get_time_elapsed();

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