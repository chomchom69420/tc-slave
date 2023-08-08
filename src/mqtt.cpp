#include "mqtt.h"
#include "credentials.h"
#include "signals_mqtt.h"
#include "control_mqtt.h"

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
      mqttClient.subscribe("/traffic/config");
    }
  }

  // Send a publish message to lastwilltopic after connecting
  mqttClient.publish(lastwilltopic, "{\"status\":\"Online\"}", true);
}

void mqtt_callback(char *topic, byte *payload, unsigned int length)
{

  Serial.print("Callback occured\n");
  Serial.print("Topic: ");
  Serial.print(topic);
  Serial.print("\n");

  String master_topic = "/status/master";
  String control_topic = "/traffic/control";
  String config_topic = "/traffic/config";

  if (strcmp(topic, sig_pub_topic.c_str()) == 0)
  {
    parse_mqtt_signal_commands(payload);
  }
  else if (strcmp(topic, master_topic.c_str()) == 0)
  {
    const int capacity = JSON_OBJECT_SIZE(5);
    StaticJsonBuffer<capacity> jb;
    JsonObject &parsed = jb.parseObject(payload);

    String master_status = parsed["status"];
    String offline = "Offline";
    String online = "Online";

    /*Master offline*/
    if (strcmp(master_status.c_str(), offline.c_str()) == 0)
    {
      master_online = 0;
      setControlMode(parsed);
    }

    /*Master offline*/
    else if (strcmp(master_status.c_str(), online.c_str()) == 0)
    {
      master_online = 1;
      setControlMode(parsed);
    }
  }

  /*control topic*/
  else if (strcmp(topic, control_topic.c_str()) == 0)
  {
    const size_t capacity = JSON_OBJECT_SIZE(2) + 30;
    DynamicJsonBuffer jsonBuffer(capacity);
    JsonObject &parsed = jsonBuffer.parseObject(payload);
    setControlMode(parsed);
  }

  /*Config topic*/
  else if (strcmp(topic, config_topic.c_str()) == 0)
  {
    const size_t capacity = 56 * JSON_OBJECT_SIZE(3) + 7 * JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(7) + 1860;
    DynamicJsonBuffer jsonBuffer(capacity);
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
  // moveToState(); -- this will be done in main
}

void mqtt_publish_state()
{
  // Publishing state to MQTT broker
  char payload[500];
  const int capacity = JSON_OBJECT_SIZE(10);
  StaticJsonBuffer<capacity> jb;
  JsonObject &obj = jb.createObject();

  obj["id"] = SLAVE_ID;
  obj["s"] = getPrimaryState();
  JsonObject &timers = obj.createNestedObject("timers");
  timers["red"] = getTimerValues(LampID::PRIMARY, SlaveState::AUTO_RED);
  timers["green"] = getTimerValues(LampID::PRIMARY, SlaveState::AUTO_GREEN);
  obj["t_elapsed"] = getElapsedTime(LampID::PRIMARY);

  // Serializing into payload
  obj.printTo(payload);
  mqttClient.publish(SLAVE_UPDATES_TOPIC, payload);
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
  DynamicJsonBuffer jsonBuffer(capacity);
  JsonObject &obj = jsonBuffer.createObject();

  obj["species"] = "slave";
  obj["slave_id"] = SLAVE_ID;
  obj["log"] = log_message;

  obj.printTo(payload);
  mqttClient.publish("/status/logs", payload);
}

void mqtt_publish(char *topic, char *payload)
{
  mqttClient.publish(topic, payload);
}