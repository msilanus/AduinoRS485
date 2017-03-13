/*******************************************************************
 * 
 *   RS485_Slave : Transmission / Reception sur un bus RS485 
 *   Ecoute du bus et réponse quand si c'est pour moi !
 *   
 *   Auteur : MS
 *   Date : 13/03/2017
 *   Rev : 0
 *   
 *   Basé sur YourDuino SoftwareSerialExample1
 *   https://arduino-info.wikispaces.com/SoftwareSerialRS485Example
 *   terry@yourduino.com
 *   
 *   Utilise la lib SoftwareSerial pour créer un port série virtuel
 *   Un module de convertion TTL/RS485 (Velleman MM107)
 *   https://www.velleman.eu/products/view/?id=431912&country=us&lang=fr
 *   
 *   Connexion :      
 *     Arduino    ->  Cenvertisseur TTL/RS485       
 *      10 (Rx)   ->    Tx (Jaune) 
 *      11 (Tx)   ->    Rx (Rouge)
 *      3 (ctr)   ->    RTS(Noir)
 *      5V        ->    5V (Rouge)
 *      GND       ->    GND (Noir)
 *      
 *     Bus :
 *      Escaves   ->    Maitre
 *       A (Rouge)->    A (Rouge)
 *       B (Jaune)->    B (Jaune)
 *        
 *     Résistance 120 Ohm à connecter entre A et B à chaque extrémité du bus 
 *     
 *  Protocol :
 *    Attente du questionnement : <slaveAddr>;<Command>;<CRC>\r\n
 *    
 *    Si <slaveAddr> = <mySlaveAdd> : réponse
 *      
 *    Affichage dans le terminal du controle d'intégrité (CRC correct)
 *    et de l'adresse de l'esclave interrogé
 *    
 *    Réponse : <mySlaveAddr>;<temp>;<hum>;<kg> ... (all you want) ... ;<CRC>\r\n
 *      
 *****************************************************************************************/


#include <SoftwareSerial.h>

#define SSerialRX        10  //Serial Receive pin
#define SSerialTX        11  //Serial Transmit pin

#define SSerialTxControl 3   //RS485 Direction control

#define RS485Transmit    HIGH
#define RS485Receive     LOW

#define Pin13LED         13   //To show activity

#define slaveAddress     "1"  //This slave has address number 1



SoftwareSerial RS485Serial(SSerialRX, SSerialTX); // RX, TX


String messageFromSerial;
String  messageFromRS485;
String messageToSend;

String calculCRC(String message)
{
  char crc = 0;
  for (int i = 0; i < message.length(); i++)
  {
    /* Debug calcul CRC

    Serial.print("message[");
    Serial.print(i);
    Serial.print("] = 0x");
    Serial.print(message[i], HEX);
    Serial.print(" - ");   */

    crc ^= message[i];
    //Serial.println(crc, HEX);

  }
  String CRC = String(crc, HEX);

  return CRC;
}

bool testCRC(String message)
{
  //Dont take the last char (\r)
  String giveCRC = message.substring(message.lastIndexOf(";") + 1, message.length() - 1); 
  //String giveCRC = message.substring(message.lastIndexOf(";") + 1); 
  String frame = message.substring(0, message.lastIndexOf(";") + 1);
  String foundCRC = calculCRC(frame);
  /* Debug
  Serial.print("CRC on frame : ");
  Serial.print(giveCRC);
  Serial.print(" in ");
  Serial.print(foundCRC.length());
  Serial.println(" char");
  Serial.print("Calcul CRC   : ");
  Serial.print(foundCRC);
  Serial.print(" in ");
  Serial.print(foundCRC.length());
  Serial.println(" char");
  */
  if (foundCRC == giveCRC)
  {
    //Debug
    //Serial.println("OK");
    return true;
  }
  else
  {
    //Debug
    //Serial.println("NOK");
    return false;
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.println("RS485 communication on hives Network : Start");
  Serial.print("Slave hive address : ");
  Serial.println(slaveAddress);
  Serial.println("-----------------------------------------");
  Serial.println();


  pinMode(Pin13LED, OUTPUT);
  pinMode(SSerialTxControl, OUTPUT);

  digitalWrite(SSerialTxControl, RS485Receive);  // Init Transceiver
  
  RS485Serial.begin(4800);   
}


void loop()
{
  
  if (RS485Serial.available() > 0) // If there is available data from RS485 bus
  {
    digitalWrite(Pin13LED, HIGH);  // Show activity ON
    messageFromRS485 = RS485Serial.readStringUntil('\n');    // Read frame from RS485 bus
    Serial.println(messageFromRS485);        // Show message in serial monitor
    delay(10);
    digitalWrite(Pin13LED, LOW);  //  Show activity OFF
    if (testCRC(messageFromRS485))
    {
      Serial.print("Transmission OK -> it's for ");  // No error transmission

      String address = messageFromRS485.substring(0, messageFromRS485.indexOf(';')); // Read slave address
      Serial.print(address); // Show slave address on serial monitor
      if (address == slaveAddress) // It's for me ?
      {
        Serial.println(" slave : It's me !");  // yes
        delay(10);
        digitalWrite(Pin13LED, HIGH);  // Show activity ON
        digitalWrite(SSerialTxControl, RS485Transmit);  // Enable RS485 Transmit 
        /************************************************************************
         * Here you have to prepare your message response to send
         ************************************************************************/
        messageToSend = "1;25;76.3;123.54;e3"; // It's an exemple response message to send
        RS485Serial.println(messageToSend);  
        
        delay(10);
        digitalWrite(Pin13LED, LOW);  //  Show activity OFF
        digitalWrite(SSerialTxControl, RS485Receive);  // Disable RS485 Transmit
      }
      else Serial.println(" : Not for me !"); 
    }
    else Serial.println("Transmission Error");

  }

}

