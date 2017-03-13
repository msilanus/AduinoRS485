/*******************************************************************
 * 
 *   RS485_Master : Transmission / Reception sur un bus RS485 
 *   Interrogation des 30 escalves potentiels
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
 *    L'interrogation d'un esclave se fait au moyen de la commande '?' :
 *    
 *    <SlaveAddr>;<Command>;<CRC>\r\n
 *    
 *    Exemple : Pour interroger l'esclave 11
 *      11;?;3f\r\n
 *      
 *  Affichage de la réponse sur le moniteur série
 *  Traitement de la réponse à la charge du développeur
 *      
 *****************************************************************************************/

#include <SoftwareSerial.h>

#define SSerialRX        10  //Serial Receive pin
#define SSerialTX        11  //Serial Transmit pin

#define SSerialTxControl 3   //RS485 Direction control

#define RS485Transmit    HIGH
#define RS485Receive     LOW

#define Pin13LED         13  //To show activity


SoftwareSerial RS485Serial(SSerialRX, SSerialTX); // RX, TX

/*-----( Declare Variables )-----*/
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
  //Dont take the last char (\r) if data send from a linux terminal
  String giveCRC = message.substring(message.lastIndexOf(";") + 1, message.length() - 1);
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
    Serial.println("OK");
    return true;
  }
  else
  {
    Serial.println("NOK");
    return false;
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.println("RS485 communication on hives Network : Start");
  Serial.print("Master test transmission for 30 slave successively");
  Serial.println();
  Serial.println("-----------------------------------------");
  Serial.println();


  pinMode(Pin13LED, OUTPUT);
  pinMode(SSerialTxControl, OUTPUT);

  digitalWrite(SSerialTxControl, RS485Receive); 

  RS485Serial.begin(4800);

}


void loop()
{
  // Ask all slaves
  for (int slave = 1; slave < 31; slave++)
  {
    digitalWrite(Pin13LED, HIGH);  // Show activity ON
    digitalWrite(SSerialTxControl, RS485Transmit);  // Enable RS485 Transmit
    messageToSend = String(slave)+";?;";
    messageToSend = messageToSend + calculCRC(messageToSend);
    RS485Serial.println(messageToSend);  // Transmit Question 
    delay(10);
    digitalWrite(Pin13LED, LOW);  //  Show activity OFF
    digitalWrite(SSerialTxControl, RS485Receive);  // Disable RS485 Transmit

    if (RS485Serial.available() > 0) // If there is available data from RS485 bus
    {
      digitalWrite(Pin13LED, HIGH);  // Show activity ON
      messageFromRS485 = RS485Serial.readStringUntil('\n');    // Read frame from RS485 bus
      Serial.println(messageFromRS485);  // Show on Serial Monitor
      delay(10);
      digitalWrite(Pin13LED, LOW);  //  Show activity OFF

      /******************************************************************
       * 
       * Here you have to do your response treatment
       * 
       ******************************************************************/

      
    }
    delay(1000);
  }
}

