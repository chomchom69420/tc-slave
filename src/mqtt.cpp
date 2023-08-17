#include "mqtt.h"
#include "credentials.h"
#include "signals_mqtt.h"
#include "control_mqtt.h"
#include "unordered_map"

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

void mqtt_reconnect()
{
  String clientId = "ESP32Slave-";
  String username = "traffic-controller";
  String password = "f3JuzTfh3utBpIow";

  String lastwilltopic = "/traffic/status";

  // serialize and store the lastWillMessage
  char lastWillMessage[100];
  const size_t capacity = JSON_OBJECT_SIZE(3);
  StaticJsonBuffer<capacity> jb;
  JsonObject &obj = jb.createObject();
  obj["species"] = "slave";
  obj["slave_id"] = SLAVE_ID;
  obj["status"] = "offline";
  obj.printTo(lastWillMessage);

  // String lastwillmessage = "{\"status\":\"Offline\"}";

  Serial.println("Connecting to MQTT Broker...");
  
  while (!mqttClient.connected())
  {
    Serial.println("Reconnecting to MQTT Broker..");
    // clientId += String(random(0xffff), HEX);
    clientId += String(SLAVE_ID);
    if (mqttClient.connect(clientId.c_str(), username.c_str(), password.c_str(), lastwilltopic.c_str(), 1, true, lastWillMessage))
    {
      Serial.println("Connected.");
      // subscribe to topic
      mqttClient.subscribe(SIGNAL_PUBLISH_TOPIC, 1);

      // Each slave connects to the master's status topic
      mqttClient.subscribe("/status/master", 1);
      mqttClient.subscribe("/traffic/config");
    }
  }

  // serialize and store the message to be sent to the lastWillTopic
  char message[100];
  StaticJsonBuffer<capacity> jbuff;
  JsonObject &obj1 = jbuff.createObject();
  obj1["species"] = "slave";
  obj1["slave_id"] = SLAVE_ID;
  obj1["status"] = "offline";
  obj1.printTo(message);

  // Send a publish message to lastwilltopic after connecting
  mqttClient.publish(lastwilltopic.c_str(), message, true);
}

void mqtt_callback(char *topic, byte *payload, unsigned int length)
{

  Serial.print("Callback occured\n");
  Serial.print("Topic: ");
  Serial.print(topic);
  Serial.print("\n");

  String status_topic = "/traffic/status";
  String control_topic = "/traffic/control";
  String config_topic = "/traffic/config";

  if (strcmp(topic, sig_pub_topic.c_str()) == 0)
  {
    parse_mqtt_signal_commands(payload);
  }
  else if (strcmp(topic, status_topic.c_str()) == 0)
  {
    const int capacity = JSON_OBJECT_SIZE(2);
    StaticJsonBuffer<capacity> jb;
    JsonObject &parsed = jb.parseObject(payload);

    String species = parsed["species"];

    /*Master offline*/
    if (species == "offline")
    {
      master_online = 0;
      setControlModeEnum(ControlMode::AUTO);
    }

    /*Master online*/
    else if (species == "online")
    {
      master_online = 1;
      setControlModeEnum(ControlMode::DICTATED);
    }
  }

  /*control topic*/
  else if (strcmp(topic, control_topic.c_str()) == 0)
  {
    const size_t capacity = JSON_OBJECT_SIZE(2) + 30;
    StaticJsonBuffer<capacity> jsonBuffer;
    JsonObject &parsed = jsonBuffer.parseObject(payload);
    setControlMode(parsed);
  }

  /*Config topic*/
  else if (strcmp(topic, config_topic.c_str()) == 0)
  {
    const size_t capacity = 56 * JSON_OBJECT_SIZE(3) + 7 * JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(7) + 1860;
    StaticJsonBuffer<capacity> jsonBuffer;
    JsonObject &parsed = jsonBuffer.parseObject(payload);
    signals_config_lamps(parsed);
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
  setEnvironment(parsed); // Set the environment
  setSlave(parsed);
  // signals_move_to_state(); -- this will be done in main
}

void mqtt_publish_state()
{
  // Publishing state to MQTT broker
  char payload[500];
  const int capacity = JSON_OBJECT_SIZE(10);
  StaticJsonBuffer<capacity> jb;
  JsonObject &obj = jb.createObject();

  std::unordered_map<int, String> state_map = {
      {0, "off"},
      {1, "red"},
      {4, "red"},
      {2, "amber"},
      {5, "amber"},
      {3, "green"},
      {6, "green"},
      {7, "blinker"}};

  obj["slave_id"] = SLAVE_ID;
  obj["state"] = state_map[getSecondaryState()];
  if(getSecondaryState()!=SlaveState::BLINKER)
    obj["t_remaining"] = signals_get_secondary_remaining_time();

  // Serializing into payload
  obj.printTo(payload);
  mqttClient.publish("/traffic/slave_feedback", payload);
}

bool mqtt_pubsubloop()
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
  const size_t capacity = JSON_OBJECT_SIZE(3) + 150;
  StaticJsonBuffer<capacity> jsonBuffer;
  JsonObject &obj = jsonBuffer.createObject();

  obj["species"] = "slave";
  obj["slave_id"] = SLAVE_ID;
  obj["log"] = log_message;

  obj.printTo(payload);
  mqttClient.publish("/status/logs", payload);
}

void mqtt_publish(const char *topic, const char *payload)
{
  mqttClient.publish(topic, payload);
}