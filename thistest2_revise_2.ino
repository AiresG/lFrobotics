#include <SPI.h>  
#include <WiFi101.h>  
#include<SD.h>  
#include <ArduCAM.h>  
#include <Wire.h>  
//#include <AudioZero.h>  
#include "memorysaver.h"  
  
  
int n=1;  
File myFile;  
File theFile;  
//#include "arduino_secrets.h"  
  
  
boolean currentLineIsBlank = true;  
#define SD_CS 4  
const int SPI_CS = 0;  
ArduCAM myCAM( OV2640, SPI_CS );  
char ssid[]="DAWIS";  
char pass[]="dawis@2017";  
int keyIndex=0;  
WiFiClient client; 
String normalFilename = String(n)+".JPG";   
File normPicture = SD.open(normalFilename);
  
  
int status=WL_IDLE_STATUS;  
WiFiServer server(80);  
  
  
void setup() {  
  Serial.begin(115200);  
  while (!Serial) {  
    ; // wait for serial port to connect. Needed for native USB port only  
  }  
  Serial.print("Initializing SD card...");  
  
  
  if (!SD.begin(4)) {  
    Serial.println("initialization failed!");  
    //return;  
  }  
  Serial.println("initialization done.");  
  
  
  if(WiFi.status()==WL_NO_SHIELD) {  
    Serial.println("WiFi shield not present");  
    while(true);  
  }  
  
  
  while(status!=WL_CONNECTED) {  
    Serial.print("Attempting to connect to Network named: ");  
    Serial.println(ssid);  
  
  
    status=WiFi.begin(ssid,pass);  
    delay(3000);  
  }  
  server.begin();  
  cam_setup();  
  Serial.println("");  
  Serial.println("");  
}  
  
  
void img_POST() {  
  Serial.begin(115200);  
  Serial.println("====================="); 
  while (!Serial) {  
      ; // wait for serial port to connect. Needed for native USB port only  
   }  
  Serial.println("---------------------");  
  //Load Normal Picture  
  while (SD.begin(4));  
  String normalFilename = String(n)+".JPG";  
  File normPicture = SD.open(normalFilename);  
  
  
  // Get the size of the image (frame) taken  
  while (!normPicture.available());  
  long jpglen = normPicture.size();  
  Serial.println("Sending "+String(n)+"th image of");  
  Serial.print(jpglen, DEC);  
  Serial.println(" bytes");  
  
  
  // Prepare request  
  Serial.println("&&&&&&&&&&&&&&&&&&&&&&&&&"); 
  String start_request = "";  
  String end_request = "";  
  start_request = start_request + "\n" + "--AaB03x" + "\n" + "Content-Type: image/jpeg" + "\n" + "Content-Disposition: form-data; name=\"file\"; filename=\"" + normalFilename + "\"" + "\n" + "Content-Transfer-Encoding: binary" + "\n" + "\n";  
  end_request = end_request + "\n" + "--AaB03x--" + "\n";  
  long extra_length;  
  extra_length = start_request.length() + end_request.length();  
  //Serial.println("Extra length:");  
  //Serial.println(extra_length);  
  long len = jpglen + extra_length;  
    
  //Serial.println(start_request);  
  Serial.println("Starting connection to server...");  
  char flask_hostname[]="192.168.254.108";  
  // Connect to the server, please change your IP address !  
  if (client.connect(flask_hostname, 5000)) {  
    Serial.println("Connected");  
    client.print("POST ");  
    client.print("/");  
    client.println(" HTTP/1.1");  
    client.println("Host: " + String(flask_hostname));  
    client.println("Content-Type: multipart/form-data; boundary=AaB03x");  
    client.print("Content-Length: ");  
    client.println(len);  
    client.print(start_request);  
    Serial.println("Connected1");  
  
  
    if (normPicture) {  
      //Serial.println("Connected2");  
      int count = 0;  
      byte clientBuf[128];  
      int clientCount = 0;  
  
  
      while (normPicture.available()) {  
        clientBuf[clientCount] = normPicture.read();  
        clientCount++;  
        count++;  
  
  
        if (clientCount > 127) {  
          //Serial.println(".");  
          client.write(clientBuf, 128);  
          clientCount = 0;  
        }  
      }  
      if (clientCount > 0) client.write(clientBuf, clientCount);  
      Serial.println(String(count));  
      normPicture.close();  
    }  
    //Serial.println("Connected3");  
    client.print(end_request);  
    client.println();  
  
  
    Serial.println("Transmission over");  
  }  
  else {  
    Serial.println("Connection failed");  
  }  
  char sentence[100];  
  Serial.println("HEARING STARTS");  
  Serial.println("Sentence "+String(n)+" is ");  
  while (client.connected()) {  
    int flag=0;  
    int i=0;  
    while (client.available()) {  
      // Read answer  
      char c = client.read();  
      if(c=='$')  
      {  
        flag=flag+1;  
        //Serial.println(flag);  
        //Serial.println(c);  
        continue;  
      }  
      if(flag==1)  
      {  
        Serial.print(c);  
        //sentence[i]=c;  
        //i=i+1;  
        //Serial.print(c);  
      }  
      //Serial.print(c);  
    }  
  }  
  //Serial.println("Sentence "+String(n)+" is ");  
  //Serial.println(sentence);  
  Serial.println("");  
  Serial.println("HEARING ENDS");  
  delay(100);  
  client.stop();  
  //while(true);  
}  
  
  
//void wav_GET()  
//{ Serial.println("");  
//  Serial.println("Starting connection to server...");  
//  char flask_hostname[]="192.168.1.109";  
//  // Connect to the server, please change your IP address !  
//  if (client.connect(flask_hostname, 5000)) {  
//    Serial.println("hello");  
//    client.println("GET /output"+String(n)+".wav HTTP/1.1");  // change resource to get here  
//    client.println("Host: 192.168.1.109");                 // change resource host here  
//    client.println("Connection: close");  
//    client.println();}  
//  else{  
//    Serial.println("Connection failed");  
//  }  
//  Serial.println("Creating file.");  
//  theFile = SD.open("output"+String(n)+".wav", FILE_WRITE);  // change file name to write to here  
//  if (!theFile) {  
//    Serial.println("Could not create file");  
//    while (1);  
//  }  
//  Serial.println("Created");  
//  int x=1;  
//  Serial.println("Loop starts");  
//  while(x!=0)  
//  {  
//    if (client.available()) {  
//    char c = client.read();  
//    //Serial.println("Available");  
//    if (c == '\n' && currentLineIsBlank) {  
//      // end of HTTP header, now save requested file  
//      while (client.connected()) {  
//        // stay in this loop until the file has been received  
//        if (client.available()) {  
//          c = client.read();  // get file byte  
//          theFile.print(c);   // save file byte  
//          //Serial.println("reading");  
//        }  
//      }  
//    }  
//    // detect the end of the incoming HTTP header  
//    if (c == '\n') {  
//      // starting a new line  
//      currentLineIsBlank = true;  
//    }  
//    else if (c != '\r') {  
//      // got a character on the current line  
//      currentLineIsBlank = false;  
//    }  
//  }  
//  
//  
//  // if the server's disconnected, stop the client:  
//  if (!client.connected()) {  
//    Serial.println();  
//    Serial.println("disconnecting.");  
//    client.stop();  
//    theFile.close();  
//    Serial.println("Finished writing to file");  
//      
//    // do nothing forevermore:  
//    //while (true);  
//    x=0;  
//  }  
//  }  
//  Serial.println("loop ends");  
//}  
void cam_setup()  
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
  myCAM.OV2640_set_JPEG_size(OV2640_1024x768);  
  delay(1000);  
}  
  
  
void cam_2_SD(){  
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
  outFile = SD.open(String(n)+".JPG", O_WRITE | O_CREAT | O_TRUNC);  
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
  
  
//void loop() {  
//   for(int i=1;i<6;i++)  
//   {  
//    cam_2_SD();  
//    delay(200);  
//    img_POST();  
//    delay(200);  
////    wav_GET();  
////    delay(200);  
////    play_WAV();  
//    n=n+1;  
//    delay(1000);  
//   }  
//   while(true);  
//} 
void loop() {
   WiFiClient client = server.available();   // listen for incoming clients
   if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
  //==============================
  SD.begin(4);  
  
  long jpglen = normPicture.size(); 
  String start_request = "";  
  String end_request = "";  
  start_request = start_request + "\n" + "--AaB03x" + "\n" + "Content-Type: image/jpeg" + 
                  "\n" + "Content-Disposition: form-data; name=\"file\"; filename=\"" + 
                  normalFilename + "\"" + "\n" + "Content-Transfer-Encoding: binary" + "\n" + "\n";  
  end_request = end_request + "\n" + "--AaB03x--" + "\n";        
  long extra_length;  
  extra_length = start_request.length() + end_request.length();    
  long len = jpglen + extra_length;

  //==============================
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:

            client.print("POST ");  
            client.print("/");  
            client.println("HTTP/1.1 200 OK");  
//            client.println("Host: " + String(flask_hostname));  
            client.println("Content-Type: multipart/form-data; boundary=AaB03x"); 
            client.println();
            client.print("Content-Length: ");  
            client.println(len);  
            client.print(start_request);  
            Serial.println("Connected1");  

            // the content of the HTTP response follows the header:  the content of the HTTP response multipart/form-data; boundary=AaB03x
//            client.print("Click <a href=\"/H\">here</a> turn the LED on pin 9 on<br>");
//            client.print("Click <a href=\"/L\">here</a> turn the LED on pin 9 off<br>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          }
          else {      // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
      if (normPicture) {  
      Serial.println("Connected2");  
      int count = 0;  
      byte clientBuf[128];  
      int clientCount = 0;  
  
  
      while (normPicture.available()) {  
        clientBuf[clientCount] = normPicture.read();  
        clientCount++;  
        count++;  
  
  
        if (clientCount > 127) {  
          //Serial.println(".");  
          client.write(clientBuf, 128);  
          clientCount = 0;  
        }  
      }  
      if (clientCount > 0) client.write(clientBuf, clientCount);  
      Serial.println(String(count));  
      normPicture.close();  
    }
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
} 
  
  

  
  
//void play_WAV()  
//{ AudioZero.begin(44100);  
//  File myFile = SD.open("output"+String(n)+".wav");  
//  if (!myFile) {  
//    // if the file didn't open, print an error and stop  
//    Serial.println("error opening test.wav");  
//    while (true);  
//  }  
//  
//  
//  Serial.print("Playing");  
//    
//  // until the file is not finished    
//  AudioZero.play(myFile);  
//  Serial.println("Finished playing");  
//  delay(300);  
//}  
