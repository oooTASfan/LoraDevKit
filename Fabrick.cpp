#include "Fabrick.h"

Fabrick::Fabrick()
{
  // Nothing here
}

//***********************************
// Start LoRa Module
//***********************************
void Fabrick::lora_begin(SoftwareSerial *lora_ss)
{
  // Software serial for LoRa module	
  _lora_ss = lora_ss;
  _lora_ss->begin(9600);
}

//***********************************
// Start LoRa DK
//***********************************
void Fabrick::lora_dk_begin(SoftwareSerial *lora_ss, Stream *debug_s)
{
  // Debug Serial
  _debug_s = debug_s;	
	
  // Software serial for LoRa module	
  _lora_ss = lora_ss;
  _lora_ss->begin(9600);
}

//***********************************
// Get Moisture Data
//***********************************
String Fabrick::get_moisture_data()
{
  // Get the average value of the sound data
  //float sum        = 0;
  //for(int i=0; i<32; i++){
     int sum = analogRead(pinMoisture);
  //}
  //int moisture_data = int(sum / 32);  
	
  //sum = constrain(sum,250,1023);
  sum = map(sum,0,1023,14000,0);
  // Convert to string 
  String moisture_data_s = String(sum, HEX);
  
  // Debug print
  if (debug_mode==1)
  {
  	_debug_s->print(F("Moisture Data: ")); 
  	_debug_s->println(sum);
  	//_debug_s->print("Sound data in HEX: "); 
  	//_debug_s->println(sound_data_s);  	
  }
  
  return moisture_data_s;
}

//***********************************
// Get Light Data
//***********************************
String Fabrick::get_light_data()
{
  // Get the average value of the sound data
  float sum        = 0;
  for(int i=0; i<32; i++){
      sum += analogRead(pinLight);
  }
  int light_data = int(sum / 32);  

  // Convert to string 
  String light_data_s = String(light_data, HEX);
  
  // Debug print
  if (debug_mode==1)
  {
  	_debug_s->print(F("Light Data: ")); 
  	_debug_s->println(light_data);
  	//_debug_s->print("Sound data in HEX: "); 
  	//_debug_s->println(sound_data_s);  	
  }
  
  return light_data_s;
}

//***********************************
// LoRa Send Moisture Data
//***********************************
void Fabrick::lora_send_moisture(String device_id){
  int frame_cntr = 1;     // Frame cntr	
  int data_ch    = 0;     // Data channel
  int data_typ   = 3401;  // Data type
  int data_len   = 4;     // Payload length (it must be even number!)

  // Get the sound data
  String moisture_data = get_moisture_data();

  // Send the data via LoRa
  lora_send(device_id, frame_cntr, data_ch, data_typ, moisture_data, data_len);

  // Duty cycle of ~1 minute
  delay(61000);
}

//***********************************
// LoRa Send Light Data
//***********************************
void Fabrick::lora_send_light(String device_id){
  int frame_cntr = 1;     // Frame cntr	
  int data_ch    = 0;     // Data channel
  int data_typ   = 3301;  // Data type
  int data_len   = 4;     // Payload length (it must be even number!)

  // Get the sound data
  String light_data = get_light_data();

  // Send the data via LoRa
  lora_send(device_id, frame_cntr, data_ch, data_typ, light_data, data_len);

  // Duty cycle of ~1 minute
  delay(61000);
}


//***********************************
// Lora Send
//***********************************
void Fabrick::lora_send(String device_id, int frame_cntr, int data_ch, int data_typ, String data, int data_len_in_hex)
{
  // Please refer to IPSO Smart Objects Document for packet format.  
  
  // Device ID
  String device_id_s = device_id;
  
  // Frame counter
  // Convert to string and ensure length of 2 (in hex) or 1 byte
  String frame_cntr_s = adding_zeros(String(frame_cntr, HEX), 2);
  
  // Data channel
  // Convert to string and ensure length of 2 (in hex) or 1 byte
  String data_ch_s = adding_zeros(String(data_ch, HEX), 2);
  
  // Data type
  // Convert to string and ensure length of 2 (in hex) or 1 byte
  String data_type_s = adding_zeros(String(data_typ-3200, HEX), 2);
  
  // Payload data 
  // Ensure length of data_len_in_hex
  String data_s = adding_zeros(data, data_len_in_hex);
  
  // Tx packet
  String tx_packet = device_id_s + frame_cntr_s + data_ch_s + data_type_s + data_s;
 
  // Length of the Tx packet
  int tx_packet_len = int(tx_packet.length());
  
  // Initialisation
  char at_command[tx_packet_len+12];

  // Send data via LoRa
  String len_in_string = String(tx_packet_len);                                // Convert to string
  String at_command_s ="AT+DTX="+ len_in_string + "," + tx_packet + "\r\n";    // AT command in string
  at_command_s.toCharArray(at_command,tx_packet_len+12);                       // Convert to char
  _lora_ss->write(at_command);                                                 // AT command to LoRa
  
  // Debug mode print
  if (debug_mode){
  	_debug_s->print(F("LoRa send -> "));
    _debug_s->println(at_command_s);
    _debug_s->println(F(" "));
  }
}

//***********************************
// Adding Zeros
//***********************************
String Fabrick::adding_zeros(String data, int len){
  
  // Length of the data 
  int data_len = data.length();
  
  // Adding zeros 
  String data_ii = data;
  if (data_len<len){
      for (int ii = 0; ii < len-data_len; ii++){
          data_ii = "0" + data_ii;
      }
  }

  return data_ii;
}

//***********************************
// Check MQTT Connection
//***********************************
void Fabrick::check_mqtt_connect(String fabrick_usr, String fabrick_pass){
	
  // Convert fabrick username and password from String to Char 
  int fusr_len  = int(fabrick_usr.length());
  int fpass_len = int(fabrick_pass.length());
  char fabrick_usr_c[fusr_len+1];
  char fabrick_pass_c[fpass_len+1];
  fabrick_usr.toCharArray(fabrick_usr_c,fusr_len+1);
  fabrick_pass.toCharArray(fabrick_pass_c,fpass_len+1);
  
  // Loop until we're reconnected
  while (!_mqtt_client->connected()) 
  {
    _debug_s->println(F("Attempting MQTT connection..."));
    // Attempt to connect
    if (_mqtt_client->connect("ESP8266Client", fabrick_usr_c, fabrick_pass_c))
    {
      _debug_s->println(F("Connected"));
    } else {
      _debug_s->print(F("failed, rc="));
      _debug_s->print(_mqtt_client->state());
      _debug_s->println(F(" try again in 5 seconds"));
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  _mqtt_client->loop();
}
