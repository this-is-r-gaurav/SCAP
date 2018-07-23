
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


char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char topic[] = "iot-2/evt/send-speed/fmt/json";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;



const char* gssid     = "Gionee M2";
const char* password = "15013029";
const char* host = "beast-app.eu-gb.mybluemix.net";
//

//Sensor Pin Number and Node MCU conncetion
const int trigPinFront = D1;  
const int echoPinFront = D2;
const int trigPinBack= D3;  
const int echoPinBack = D4; 
int motor = D6;
int alert_led = D7;
char* _id = "fa2db5080fe437cb6affcbeab0d0640e";


//other variables defining states of car
boolean brake_failed=true, brake_failed_status;
int distFromFront,distFromBack;
int very_near = 5, near = 10, mid = 50, far =100;
int spd,speed_after_which_sensor_operates, max_range;


//Method Declaration
int get_distance(int, int);
boolean check_status_of_brake(boolean);
void post_brake_status();
void wifiConnect(const char *,const char *);
WiFiClient wifiClient;
PubSubClient client(server, 1883, wifiClient);

void setup() {
    Serial.begin(115200);
    delay(1000);
    pinMode(trigPinFront, OUTPUT); 
    pinMode(echoPinFront, INPUT);
    pinMode(trigPinBack, OUTPUT); 
    pinMode(echoPinBack, INPUT); 
    pinMode(motor, OUTPUT);
    pinMode(alert_led, OUTPUT);
    
    brake_failed_status= check_status_of_brake(brake_failed);
    wifiConnect(gssid,password);
    mqttConnect();
    delay(10);
    speed_after_which_sensor_operates = 50;
    max_range = 150;
    spd=0;
    if (!brake_failed_status){
      for(int i =0; i<100; i++)
      {
          spd +=1;
          analogWrite(motor,spd*2);
          delay(100);
       }
    }
    else{
        post_brake_status();  
    }
    Serial.println(spd);

}

void loop() {
    while (!brake_failed_status){
       
        delay(100);
        while(spd > speed_after_which_sensor_operates){
            distFromFront = get_distance(trigPinFront, echoPinFront);
            delay(100);
            distFromBack = get_distance(trigPinBack, echoPinBack);
            delay(100);
            while (isnan(distFromFront) || isnan(distFromBack))
            {
              Serial.println("Your Sensor aren't working Properly, Connect Your Sensor Properly");
              delay(1000);
            }
            if(distFromFront < very_near){
                  spd -= 12;
                  Serial.println("The Vehicle Ahead You is too closer, Stopping your vehicle and dropping Speed down to " + String(spd) );
            }
            else if (distFromFront<= near){
                if (distFromBack <= near)
                {
                    spd -= 10;
                    Serial.println("The Vehicle ahead You and behind You are approaching near, So decrasing speed down to " +String(spd));
                }
                else
                {
                    spd -= 8;
                    Serial.println("The Vehicle ahaed You and behind You are approaching near, So decrasing speed down to "+ String(spd));
                }
            }
            else if(distFromFront<=mid && distFromFront>near){

               if (distFromBack<near){
                  spd += 10;
                  Serial.println("The Vehicle behind you is approaching towards You so increasing speed by "+ String(spd));
               }
               else{
                
                  spd+=7; 
               }
              
            }
            else if (distFromFront<=far && distFromFront>mid) {
               if (distFromBack<near){
                  spd += 12;
                  Serial.println("The Vehicle behind you is approaching towards You so increasing speed by " + String(spd));
               }
               else{
                  spd+=10; 
               }
            }
            else{
              //spd=60;
              
            }
            if (spd<0)
            {
                spd =0;
            }
            if (spd > max_range){
              
              spd = max_range;
            }
            PublishData(spd,distFromFront,distFromBack, _id);
            if (!client.loop()) {
                mqttConnect();
                delay(100);
            }
            analogWrite(motor,spd*4);
        }
        delay(100);
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
    digitalWrite(t_pin, LOW);
    delay(2);
    digitalWrite(t_pin, HIGH);
    delay(10);
    digitalWrite(t_pin, LOW);
    duration = pulseIn(e_pin, HIGH);
    distance= duration*0.034/2;
    return distance;
 
}

boolean check_status_of_brake(boolean brake){
    
    if(brake_failed){  
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
        delay(100);
        Serial.print(".");
        if (brake_failed_status){
                  digitalWrite(alert_led,HIGH);
        delay(100);
        digitalWrite(alert_led,LOW);
        delay(100);
          }
    }

    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void post_brake_status()
{
    Serial.print("Connecting to ");
    Serial.println(host);
    
    String postData = "_id="+String(_id);
    HTTPClient http; 
    http.begin("http://beast-app.eu-gb.mybluemix.net/send-brake-status");
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
void PublishData(int spd,int frontDist,int backDist,char *id){
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
   payload += ",\"frontDist\":";
   payload += frontDist;
   payload += ",\"backDist\":";
   payload += backDist;
   payload += ",\"_id\":\"";
   payload += id;
   
   payload += "\"}}";
   Serial.print("Sending payload: ");
   Serial.println(payload);    
   if (client.publish(topic, (char*) payload.c_str())) {
      Serial.println("Speed :"+ String(spd));
   }
   else {
      Serial.println("Publish failed");
   }
}
