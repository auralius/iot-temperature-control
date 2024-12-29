#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>


const int   mqtt_port     = 8883;
const char* mqtt_server   = "bcf21ddabed1412aaf638a09b4732f50.s1.eu.hivemq.cloud";
const char* mqtt_clientid = "Device01";
const char* mqtt_username = "Device01";
const char* mqtt_password = "Device01";


const char* wifi_id       = "Polo";
const char* wifi_password = "pabl0picas0";


WiFiClientSecure espClient;
PubSubClient client(espClient);


float PV        = 0.0;   // process value
float SV        = 0.0;   // set value


const byte numChars = 32;
char       receivedChars[numChars];
boolean    newData = false;


void connect_to_wifi(const char* ssid, const char* pwd)
{
  Serial.print("\nConnecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, pwd);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("\nWiFi connected\nIP address: ");
  Serial.println(WiFi.localIP());

}


void reconnect() 
{
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect(mqtt_clientid, mqtt_username, mqtt_password))
      Serial.println("connected");
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");   // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
  String incommingMessage = "";
  for (int i = 0; i < length; i++)
    incommingMessage += (char)payload[i];

  SV = incommingMessage.toFloat();
}


void publishMessage(const char* topic, String payload , boolean retained)
{
  client.publish(topic, payload.c_str(), retained);
}


void setup()
{
  Serial.begin(9600);
  while (!Serial) 
    delay(1);

  connect_to_wifi(wifi_id, wifi_password);
  espClient.setInsecure();

  // Connect to the broker
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}


void loop() 
{
  if (!client.connected()) {
    reconnect();
    client.subscribe("system-1/SV"); // Subscribe the topics here !!!
  }

  client.loop();

  rxPV();
  txSV();

  publishMessage("system-1/PV", receivedChars, true);

  delay(1000);
}


void txSV()
{
  Serial.print("<");
  Serial.print(SV, 3);
  Serial.print(">");
}


void rxPV() 
{
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }

  if (newData == true) {
    PV = atof(receivedChars);
    //Serial.print("Received new PV: ");
    //Serial.println(PV);

    newData = false;
  }
}
