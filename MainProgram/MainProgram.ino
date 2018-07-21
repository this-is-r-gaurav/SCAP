#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>

//For PUBSUB - just used for demonstration of device
#define ORG "n69fbf"
#define DEVICE_TYPE "Beast-Device-type"
#define DEVICE_ID "Beast-Device"
#define TOKEN "lIOg8FAE@dL+dxJk(-"
String command;


char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char topic[] = "iot-2/evt/send-speed/fmt/json";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;

const char* gssid     = "Gionee M2";
const char* password = "15013029";
const char* host = "beast-app.eu-gb.mybluemix.net";
const int port = 443;
//analog pin : D1,D2,D3, D4,D6,D7,D8,RSV
//

//Sensor Pin Number and Node MCU conncetion
const int trigPinFront = D1;  
const int echoPinFront = D2;
const int trigPinBack= D3;  
const int echoPinBack = D4; 
int motor = D6;
int alert_led = D7;


//other variables defining states of car
boolean brake_failed=false, brake_failed_status;
int distFromFront,distFromBack;
int very_near = 5, near = 10, mid = 20, far =30; 
int spd,working_speed;


//Method Declaration
int get_direction(int, int);
boolean check_status_of_brake(boolean);
void post_brake_status();
void wifiConnect(const char *,const char *);
//void callback(char* topic, byte* payload, unsigned int payloadLength);

WiFiClient wifiClient;
PubSubClient client(server, 1883, wifiClient);

void setup() {
    Serial.begin(115200);
    delay(1000);
//    Serial.println("Trigger Front Pin:"+ String(trigPinFront));
//    Serial.println("Echo Front Pin:" + String(echoPinFront));
//    
//    Serial.println("Trigger Pin Back:"+String(trigPinBack));
//    Serial.println("Echo Back Pin:"+String(echoPinBack));
    pinMode(trigPinFront, OUTPUT); 
    pinMode(echoPinFront, INPUT);
    pinMode(trigPinBack, OUTPUT); 
    pinMode(echoPinBack, INPUT); 
    pinMode(motor, OUTPUT);
    pinMode(alert_led, OUTPUT);

    //wifiConnect(gssid,password);
    //mqttConnect();
    
    brake_failed_status= check_status_of_brake(brake_failed);
    spd=0;
    if (!brake_failed_status){
      for(int i =0; i<100; i++)
      {
          spd +=1;
          analogWrite(motor,spd);
          Serial.println(spd);
          delay(100);
       }
    }
    Serial.println(spd);
  
    Serial.println();
    Serial.println();
    delay(1000);

}

void loop() {
    while (!brake_failed_status){
       
        delay(100);
        while(spd > working_speed){
            distFromFront = get_distance(trigPinFront, echoPinFront);
            delay(100);
            distFromBack = get_distance(trigPinBack, echoPinBack);
            delay(100);
            while (isnan(distFromFront) || isnan(distFromFront))
            {
              Serial.println("Your Sensor aren't working Properly, Connect Your Sensor Properly");
              delay(1000);
            }
            if(distFromFront < very_near){
                  spd = 0;
                  Serial.println("The Vehicle Ahead You is too closer, Stopping your vehicle and dropping Speed down to " + String(spd) );
            }
            else if (distFromFront<= near){
                if (distFromBack <= near)
                {
                    spd = 0;
                    Serial.println("The Vehicle ahaed You and behind You are approaching near, So decrasing speed down to" +String(spd));
                }
                else
                {
                    spd -= 20;
                    Serial.println("The Vehicle ahaed You and behind You are approaching near, So decrasing speed down to"+ String(spd));
                }
            }
            else if(distFromFront<=mid && distFromFront>near){

               if (distFromBack<near){
                  spd += 20;
                  Serial.println("The Vehicle behind you is approaching towards You so increasing speed by "+ String(spd));
               }
               else{
                
                  spd-=5; 
               }
              
            }
            else if (distFromFront<=far && distFromFront>mid) {
               if (distFromBack<near){
                  spd += 30;
                  Serial.println("The Vehicle behind you is approaching towards You so increasing speed by " + String(spd));
               }
               else{
                  spd+=10; 
               }
            }
            //PublishData(spd);
//            if (!client.loop()) {
//                mqttConnect();
//            }
            delay(100);
            analogWrite(motor,spd);
        }
        brake_failed_status = check_status_of_brake(brake_failed);
    }
    Serial.println("Brake Failed Call Mechanic");
    delay(100);
    while(brake_failed_status){
        digitalWrite(alert_led,HIGH);
        delay(100);
        digitalWrite(alert_led,LOW);
        delay(100);
    }
    
  
            
}
   

int get_distance(int t_pin, int e_pin){
    int distance;
    int duration;
    Serial.println("T_pin:"+String(t_pin));
    Serial.println("T_pin:"+String(e_pin));
    digitalWrite(t_pin, LOW);
    delay(2);
    digitalWrite(t_pin, HIGH);
    delay(10);
    digitalWrite(t_pin, LOW);
    duration = pulseIn(e_pin, HIGH);
    distance= duration*0.034/2;
    Serial.print("Distance: ");
    Serial.println(distance);
    return distance;
 
}

boolean check_status_of_brake(boolean brake){
    
    if(brake_failed){  
        post_brake_status();
        spd=0;
        return true; 
    }
    else{
        return false;
    }
}

void wifiConnect(const char *ssid,const char *passw) {
    
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, passw);
  
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void post_brake_status()
{
    char *msg = "Brake Failed Call Your Mechanic";
    char *auth = "226551A1d4JIDN405b4dae6b";
    Serial.println();
    Serial.println();
    Serial.print("connecting to ");
    Serial.println(host);
    
    String postData = "mobile=9017920586&status=True";
    HTTPClient http; 
    http.begin("http://beast-app.eu-gb.mybluemix.net/send-car-status");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header
    int httpCode = http.POST(postData);   //Send the request
    String payload = http.getString();    //Get the response payload
 
    Serial.println(httpCode);   //Print HTTP return code
    Serial.println(payload);    //Print request response payload
 
    http.end(); 

}


void mqttConnect() {
  if (!client.connected()) {
    Serial.print("Reconnecting MQTT client to ");
    Serial.println(server);
    while (!client.connect(clientId, authMethod, token)) {
      Serial.print(".");
      delay(500);
    }
    initManagedDevice();
    Serial.println();
  }
}
void initManagedDevice() {
    if (client.subscribe(topic)) {
        Serial.println("subscribe to cmd OK");
    } else {
        Serial.println("subscribe to cmd FAILED");
    }
}



void PublishData(int spd){
   if (!!!client.connected()) {
      Serial.print("Reconnecting client to ");
      Serial.println(server);
      while (!!!client.connect(clientId, authMethod, token)) {
          Serial.print(".");
          delay(500);
      }
      Serial.println();
   }
   String payload = "{\"d\":{\"speed\":";
   payload += spd;
   payload += "}}";
   Serial.print("Sending payload: ");
   Serial.println(payload);    
   if (client.publish(topic, (char*) payload.c_str())) {
      Serial.println("Publish ok");
   }
   else {
      Serial.println("Publish failed");
   }
}
