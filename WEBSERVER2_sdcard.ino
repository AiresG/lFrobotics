/*--------------------------------------------------------------
  Program:      eth_websrv_SD_image

  Description:  Arduino web server that serves up a basic web
                page that displays an image.
  
  Hardware:     Arduino Uno and official Arduino Ethernet
                shield. Should work with other Arduinos and
                compatible Ethernet shields.
                2Gb micro SD card formatted FAT16
                
  Software:     Developed using Arduino 1.0.5 software
                Should be compatible with Arduino 1.0 +
                
                Requires index.htm, page2.htm and pic.jpg to be
                on the micro SD card in the Ethernet shield
                micro SD card socket.
  
  References:   - WebServer example by David A. Mellis and 
                  modified by Tom Igoe
                - SD card examples by David A. Mellis and
                  Tom Igoe
                - Ethernet library documentation:
                  http://arduino.cc/en/Reference/Ethernet
                - SD Card library documentation:
                  http://arduino.cc/en/Reference/SD

  Date:         7 March 2013
  Modified:     17 June 2013
 
  Author:       W.A. Smith, http://startingelectronics.org
--------------------------------------------------------------*/

#include <SPI.h>  
#include <WiFi101.h>  
#include<SD.h>  
#include <ArduCAM.h>  
#include <Wire.h>  
//#include <AudioZero.h>  
#include "memorysaver.h"  

// size of buffer used to capture HTTP requests
#define REQ_BUF_SZ   20
int n=1;  
File myFile;  
File theFile;  

boolean currentLineIsBlank = true;  
#define SD_CS 4  
const int SPI_CS = 0;  
ArduCAM myCAM( OV2640, SPI_CS ); 
IPAddress ip(192, 168 ,254 ,109); // IP address, may need to change depending on network

char ssid[]="DAWIS";  
char pass[]="dawis@2017";

File webFile;
char HTTP_req[REQ_BUF_SZ] = {0}; // buffered HTTP request stored as null terminated string
//char req_index = 0;              // index into HTTP_req buffer
char keyIndex=0; 
WiFiClient client; 
String normalFilename = String(n)+".JPG";  
File normPicture = SD.open(normalFilename);

  
int status=WL_IDLE_STATUS;  
WiFiServer server(80);  

void setup(){//============================================================================= S E T U P
    
    Serial.begin(115200);        // for debugging

     while (!Serial) {  
    ; // wait for serial port to connect. Needed for native USB port only  
  } 
    // initialize SD card
    Serial.println("Initializing SD card...");
    if (!SD.begin(4)) {
        Serial.println("ERROR - SD card initialization failed!");
        return;    // init failed
    }
    Serial.println("SUCCESS - SD card initialized.");
    // check for index.htm file
    if (!SD.exists("index.htm")) {
        Serial.println("ERROR - Can't find index.htm file!");
        return;  // can't find index file
    }
    Serial.println("SUCCESS - Found index.htm file.");
    
    while(status!=WL_CONNECTED) {  
    Serial.print("Attempting to connect to Network named: ");  
    Serial.println(ssid);  
  
  
    status=WiFi.begin(ssid,pass);  
    delay(1000);  
  }  
  server.begin(); 
  cam_setup(); 
  Serial.println("");  
  Serial.println("");
}
void cam_setup()//============================================================================= C A M_S E T U P  
{  
  uint8_t vid, pid;  
  uint8_t temp;  
  Wire.begin();  
  Serial.begin(9600);  
  while (!Serial) {  
      ; // wait for serial port to connect. Needed for native USB port only  
   }  
  Serial.println(F("ArduCAM Start!"));  
  //set the CS as an output:  
  pinMode(SPI_CS,OUTPUT);  
  digitalWrite(SPI_CS, HIGH);  
  // initialize SPI:  
  SPI.begin();  
  //delay(2000);  
  //Reset the CPLD  
  myCAM.write_reg(0x07, 0x80);  
  delay(100);  
  myCAM.write_reg(0x07, 0x00);  
  delay(100);  
    
  while(1){  
    //Check if the ArduCAM SPI bus is OK  
    myCAM.write_reg(ARDUCHIP_TEST1, 0x55);  
    temp = myCAM.read_reg(ARDUCHIP_TEST1);  
    
    if (temp != 0x55){  
      Serial.println(F("SPI interface Error!"));  
      delay(1000);continue;  
    }  
    else{  
      Serial.println(F("SPI interface OK."));break;  
    }  
  }  
  while(1){  
    //Check if the camera module type is OV2640  
    myCAM.wrSensorReg8_8(0xff, 0x01);  
    myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);  
    myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);  
    if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 ))){  
      Serial.println(F("Can't find OV2640 module!"));  
      delay(1000);continue;  
    }  
    else{  
      Serial.println(F("OV2640 detected."));break;  
    }   
  }  
  myCAM.set_format(JPEG);  
  myCAM.InitCAM();  
  myCAM.OV2640_set_JPEG_size(OV2640_1024x768);  //  1024x768   320x240
  delay(2000);  
}  

void cam_2_SD(){ //============================================================================= C A M 2 S D
  Serial.println("");  
  Serial.println("");  
  Serial.println("IMAGE NO. "+String(n));  
  char str[8];  
  byte buf[256];  
  static int i = 0;  
  static int k = 0;  
  uint8_t temp = 0,temp_last=0;  
  uint32_t length = 0;  
  bool is_header = false;  
  File outFile;  
  //Flush the FIFO  
  myCAM.flush_fifo();  
  //Clear the capture done flag  
  myCAM.clear_fifo_flag();  
  //Start capture  
  myCAM.start_capture();  
  Serial.println(("Start Capture"));  
  while(!myCAM.get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK));  
  Serial.println(("Capture Done."));    
  length = myCAM.read_fifo_length();  
  Serial.print(("The fifo length is :"));  
  Serial.println(length, DEC);  
  if (length >= MAX_FIFO_SIZE) //384K  
  {  
    Serial.println(("Over size."));  
    return ;  
  }  
  if (length == 0 ) //0 kb  
  {  
    Serial.println(("Size is 0."));  
    return ;  
  }  
  //Construct a file name  
  //k = k + 1;  
  //itoa(k, str, 10);  
  //strcat(str, ".jpg");  
  //Open the new file  
  outFile = SD.open(String(n)+".JPG", O_WRITE | O_CREAT | O_TRUNC | O_READ);  
  if(!outFile){  
    Serial.println(("File open failed"));  
    return;  
  }  
  myCAM.CS_LOW();  
  myCAM.set_fifo_burst();  
  while ( length-- )  
  {  
    temp_last = temp;  
    temp =  SPI.transfer(0x00);  
    //Read JPEG data from FIFO  
    if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,  
      {  
        buf[i++] = temp;  //save the last  0XD9       
        //Write the remain bytes in the buffer  
        myCAM.CS_HIGH();  
        outFile.write(buf, i);      
        //Close the file  
        outFile.close();  
        Serial.println(("Image No. "+String(n)+" save OK."));  
        is_header = false;  
        i = 0;  
      }    
    if (is_header == true)  
      {   
        //Write image data to buffer if not full  
        if (i < 256)  
        buf[i++] = temp;  
        else  
          {  
            //Write 256 bytes image data to file  
            myCAM.CS_HIGH();  
            outFile.write(buf, 256);  
            i = 0;  
            buf[i++] = temp;  
            myCAM.CS_LOW();  
            myCAM.set_fifo_burst();  
          }          
      }  
    else if ((temp == 0xD8) & (temp_last == 0xFF))  
      {  
        is_header = true;  
        buf[i++] = temp_last;  
        buf[i++] = temp;     
      }   
  }   
}



void loop()//============================================================================= M A I N
{
//     for(int i=1;i<6;i++){  
//        
//        cam_2_SD();   
//        delay(1000);   
//        n=n+1;  
         
    
        //====================================
        WiFiClient client = server.available();  // try to get client

          if (client) {  // got client?
              boolean currentLineIsBlank = true;
              while (client.connected()) {
                  if (client.available()) {   // client data available to read
                      char c = client.read(); // read 1 byte (character) from client
                      // buffer first part of HTTP request in HTTP_req array (string)
                      // leave last element in array as 0 to null terminate string (REQ_BUF_SZ - 1)
                      if (keyIndex < (REQ_BUF_SZ - 1)) {
                          HTTP_req[keyIndex] = c;          // save HTTP request character
                          keyIndex++;
                      }
                      // print HTTP request character to serial monitor
                      Serial.print(c);
                      // last line of client request is blank and ends with \n
                      // respond to client only after last line received
                      if (c == '\n' && currentLineIsBlank) {
                          // open requested web page file
                          if (StrContains(HTTP_req, "GET / ")
                                       || StrContains(HTTP_req, "GET /index.htm")) {
                              client.println("HTTP/1.1 200 OK");
                              client.println("Content-Type: text/html");
                              client.println("Connnection: close");
                              client.println();
                              webFile = SD.open("index.htm");        // open web page file
                          }
                          else if (StrContains(HTTP_req, "GET /page2.htm")) {
                              client.println("HTTP/1.1 200 OK");
                              client.println("Content-Type: text/html");
                              client.println("Connnection: close");
                              client.println();
                              webFile = SD.open("page2.htm");        // open web page file
                          }
                          else if ((HTTP_req, "GET /*.JPG")) {
                              webFile = SD.open("*.JPG");
                              //webFile = normPicture;
                              if (webFile) {
                                  client.println("HTTP/1.1 200 OK");
                                  client.println();
                              }
                          }
                          if (webFile) {
                              while(webFile.available()) {
                                  client.write(webFile.read()); // send web page to client
                              }
                              webFile.close();
                          }
                          // reset buffer index and all buffer elements to 0
                          keyIndex = 0;
                        StrClear(HTTP_req, REQ_BUF_SZ);
                          break;
                      }
                      // every line of text received from the client ends with \r\n
                      if (c == '\n') {
                          // last character on line of received text
                          // starting new line with next character read
                          currentLineIsBlank = true;
                      } 
                      else if (c != '\r') {
                          // a text character was received from client
                          currentLineIsBlank = false;
                      }
                  } // end if (client.available())
              } // end while (client.connected())
              delay(1);      // give the web browser time to receive the data
              client.stop(); // close the connection
          } // end if (client)
//    delay(3000);  
//   }
//   while(true); 
}

// sets every element of str to 0 (clears array)
void StrClear(char *str, char length)
{
    for (int i = 0; i < length; i++) {
        str[i] = 0;
    }
}

// searches for the string sfind in the string str
// returns 1 if string found
// returns 0 if string not found
char StrContains(char *str, char *sfind)
{
    char found = 0;
    char index = 0;
    char len;

    len = strlen(str);
    
    if (strlen(sfind) > len) {
        return 0;
    }
    while (index < len) {
        if (str[index] == sfind[found]) {
            found++;
            if (strlen(sfind) == found) {
                return 1;
            }
        }
        else {
            found = 0;
        }
        index++;
    }

    return 0;
        //==================================== 

}
