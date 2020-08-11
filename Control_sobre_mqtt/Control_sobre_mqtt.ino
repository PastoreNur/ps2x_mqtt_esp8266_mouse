#include <PubSubClient.h>
#include <PS2X_lib.h>  //for v1.6
#include <ESP8266WiFi.h>
#include <string.h>


#ifndef STASSID
#define STASSID "Blues"
#define STAPSK  "06081972"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;

const char* mqttserver = "192.168.0.18";
const int mqttport = 1883;


/******************************************************************
 * set pins connected to PS2 controller:
 *   - 1e column: original 
 *   - 2e colmun: Stef?
 * replace pin numbers by the ones you use
 ******************************************************************/
#define PS2_DAT        14  //14    
#define PS2_CMD        13  //15
#define PS2_SEL        12  //16
#define PS2_CLK        15  //17

/******************************************************************
 * select modes of PS2 controller:
 *   - pressures = analog reading of push-butttons 
 *   - rumble    = motor rumbling
 * uncomment 1 of the lines for each mode selection
 ******************************************************************/
//#define pressures   true
#define pressures   false
//#define rumble      true
#define rumble      false

PS2X ps2x; // create PS2 Controller Class

//right now, the library does NOT support hot pluggable controllers, meaning 
//you must always either restart your Arduino after you connect the controller, 
//or call config_gamepad(pins) again after connecting the controller.

int error = 0;
byte type = 0;
byte vibrate = 0;

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      publicar("connected to MQTT server");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void publicar(char *msg){
    client.publish("Prueba", msg);
  }
  

void setup(){
 
  Serial.begin(57600);
  
  delay(300);  //added delay to give wireless ps2 module some time to startup, before configuring it
  
  //setup pins and settings: GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
  do{
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
  delay(500);
  }while(error != 0);
  if(error == 0){
    Serial.println("Found Controller, configured successful ");
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    
  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    client.setServer(mqttserver, mqttport);
    client.setCallback(callback);
  }

}

void loop() {
  /* You must Read Gamepad to get new values and set vibration values
     ps2x.read_gamepad(small motor on/off, larger motor strenght from 0-255)
     if you don't enable the rumble, use ps2x.read_gamepad(); with no values
     You should call this at least once a second
   */  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

    ps2x.read_gamepad(false, vibrate); //read controller and set large motor to spin at 'vibrate' speed
    
    if(ps2x.Button(PSB_START)){         //will be TRUE as long as button is pressed
      publicar("Start");
    }
    if(ps2x.Button(PSB_SELECT))
      Serial.println("Select");      
   

    vibrate = ps2x.Analog(PSAB_CROSS);  //this will set the large motor vibrate speed based on how hard you press the blue (X) button

    if(ps2x.ButtonPressed(PSB_CIRCLE)){             //will be TRUE if button was JUST pressed
      publicar("Derecho");
    }
    if(ps2x.ButtonPressed(PSB_CROSS)){               //will be TRUE if button was JUST pressed OR released
      publicar("Izquierdo");
    }
      
    if (ps2x.Analog(PSS_LY) != 127 || ps2x.Analog(PSS_LX) != 128)
    {
      String payload = "{\"X\":";
      payload += ps2x.Analog(PSS_LX);
      payload += ",\"Y\":";
      payload += ps2x.Analog(PSS_LY);
      payload += "}";
      char MQTT_msg[256];
      payload.toCharArray(MQTT_msg,256);
      publicar(MQTT_msg);
    }
    
  
  delay(50);  
}
