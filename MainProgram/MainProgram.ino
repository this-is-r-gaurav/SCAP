#include <ESP8266WiFi.h>

const char* ssid     = "Gionee M2";
const char* password = "15013029";
const char* host = "msg91.com"; //IP of Raspberry Pi


//Sensor Pin Number and Node MCU conncetion
const int trigPinFront = D2;  
const int echoPinFront = D4;
const int trigPinBack= D5;  
const int echoPinBack = D7; 
int motor = D5;


//other variables defining states of car
boolean brake_failed=true, brake_failed_status;
int distFromFront,distFromBack;
int near = 10, mid = 20, far =30; 


//Method Declaration
int get_direction(int, int);
boolean check_status_of_brake(boolean);
void connect_to_wifi(const char *,const char *);

void setup() {
  Serial.begin(115200);
  pinMode(trigPinFront, OUTPUT); 
  pinMode(echoPinFront, INPUT);
  pinMode(trigPinBack, OUTPUT); 
  pinMode(echoPinBack, INPUT); 
  pinMode(motor, OUTPUT);

  delay(100);
  brake_failed_status= check_status_of_brake(brake_failed);
  
  Serial.println();
  Serial.println();
  delay(10);

}

void loop() {
    if (!brake_failed_status){
      distFromFront = get_distance(trigPinFront, echoPinFront);
      delay(100);
      distFromBack = get_distance(trigPinBack, echoPinBack);
      delay(100);

      if(distFromFront<near && distFromBack>far){
          Serial.println("Speed Is 10 Kmph"); 
      }
      else if(distFromFront>near && distFromFront<mid & distFromBack<far && distFromBack>mid){ 
          Serial.println("Speed Is 20Kmph"); 
      }
      else{
          Serial.println("Speed is 30kmph");
      }
  }
  delay(10000);
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
    Serial.print("Distance: ");
    Serial.println(distance);
    return distance;
 
}

boolean check_status_of_brake(boolean brake){
    
    if(brake_failed){  
        connect_to_wifi(ssid, password);
        return true; 
    }
    else{
        return false;
    }
}

void connect_to_wifi(const char *ssid,const char *passw)
{
  
    char *msg = "Brake Failed Call Your Mechanic";
    char *auth = "226551A1d4JIDN405b4dae6b";
    Serial.print("Connecting to ");
    Serial.println(ssid);

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
    Serial.println();
    Serial.println("");
    Serial.print("connecting to ");
    Serial.println(host);
    WiFiClient client;
    if (!client.connect(host,443)) {
    Serial.println("connection failed");
            return;
    }// We now create a URI for the request
    String url = "/sendhttp.php?sender=DCoders&route=4";
    url+="&mobiles=";
    url+="919017920586";
    url+="&authkey=";
    url+=auth;
    url+="&country=0&message=";
    url += msg;
    Serial.print("Requesting URL: ");
    Serial.println(url);

     Serial.println();
 
 Serial.print("Sending payload: ");
 Serial.println(url);
    // This will send the request to the server
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
        }
    }
  
    // Read all the lines of the reply from server and print them to Serial
    while(client.available()){
        String line = client.readStringUntil('\r');
        Serial.print(line);
    }
    Serial.println();
    Serial.println("closing connection");

}
