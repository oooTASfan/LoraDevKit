#ifndef _Fabrick_H
#define _Fabrick_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <PubSubClient.h>

// Pins Assignment
const int debug_mode   =  1; //  (1=debug mode)
const int pinMoisture    = A0; //  Sound sensor pin assignment
const int pinLight    = A1; //  Light sensor pin assignment
const int pinLoraTx    =  3; //  LoRa Tx pin assignment
const int pinLoraRx    =  2; //  LoRa Rx pin assignment

// Class
class Fabrick
{
  public:
    Fabrick();
    void   lora_begin(SoftwareSerial *lora_ss);
    void   lora_dk_begin(SoftwareSerial *lora_ss, Stream *debug_s);
    String get_moisture_data();
	String get_light_data();
	
    void   lora_send_moisture(String device_id);  
	void   lora_send_light(String device_id);	
    void   lora_send(String device_id, int frame_cntr, int data_ch, int data_typ, String data, int data_len_in_hex);
    String adding_zeros(String data, int len);
    void   check_mqtt_connect(String fabrick_usr, String fabrick_pass);
    
  private:
    SoftwareSerial *_lora_ss;
    SoftwareSerial *_pm25_ss;  
    SoftwareSerial *_wifi_ss;
    PubSubClient   *_mqtt_client;
    Stream         *_debug_s;
}; 

extern Fabrick fabrick;

#endif
