/*
  Web Server

 A simple web server that shows the value of the analog input pins.
 using an Arduino Wiznet Ethernet shield.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)

 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe

 */

#include <SPI.h>
#include <Ethernet.h>
#include <Servo.h>
#include <SdFat.h>
#include <SdFatUtil.h>
#include <SdVolume.h>

char inData[11]; // Allocate some space for the string
char inChar; // Where to store the character read
byte charIndex = 0; // Index into array; where to store the character
int serialRead;

bool gotData = false;

int timesWentBy = 0;

Servo servo;
long prev=0;  //saves last time (in millisec) the servo arm was moved down
long current=0;  //current time in millisec
unsigned char servoangle=0;  //current servo angle
#define COUNTTIME 2000 //defines time that the arm waits between tick-downs (in millisec)
#define MOV 12 //defines how much the arm moves up when a page is loaded

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 80);

Sd2Card card;
SdVolume volume;
SdFile root;
SdFile file;


#define error(s) error_P(PSTR(s))
void error_P(const char* str) {
  PgmPrint("error: ");
  SerialPrintln_P(str);
  if (card.errorCode()) {
    PgmPrint("SD error: ");
    Serial.print(card.errorCode(), HEX);
    Serial.print(',');
    Serial.println(card.errorData(), HEX);
  }
  while(1);
}

char rootFileName[] = "index.htm";

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

bool gotInfo = false;
int infoType;

int LED = 15;

void setup() {
  servo.attach(9);  //sets servo to pin 9.  Choose your favorite PWM pin (except 10 or 11)
  servo.write(0);
  
  pinMode(LED, OUTPUT);

  SdInit(9600);
}

#define BUFSIZ 100  //Buffer size for getting data
char clientline[BUFSIZ];  //string that will contain command data
char *filename;

void loop() {
  int index = 0; // reset the clientline index
  // listen for incoming clients
  int image = 0;
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if(index<BUFSIZ)  //Only add data if the buffer isn't full.
        {
          clientline[index]=c;
          index++;
        }
        clientline[index] = 0;
        filename = 0;
        Serial.print("Request: ");
        Serial.println(clientline);
        Serial.println("\n");
        if (strstr(clientline, "GET / ") != 0) {
          filename = rootFileName;
        }
        if (strstr(clientline, "GET /") != 0) {
          if (!filename) filename = clientline + 5;
          (strstr(clientline, " HTTP"))[0] = 0;
          Serial.println(filename);
          if (! file.open(&root, filename, O_READ)) {
            client.println("HTTP/1.1 404 Not Found");
            client.println("Content-Type: text/html");
            client.println();
            client.println("<h2>File Not Found!</h2>");
            break;
          }
          Serial.println("Opened!");
          client.println("HTTP/1.1 200 OK");
          if (strstr(filename, ".htm") != 0){
            client.println("Content-Type: text/html");
            servoangle+=MOV;  //Moves servo amount specified by MOV when html file is opened
          }
          else if (strstr(filename, ".css") != 0)
            client.println("Content-Type: text/css");
          else if (strstr(filename, ".png") != 0)
            client.println("Content-Type: image/png");
          else if (strstr(filename, ".jpg") != 0)
            client.println("Content-Type: image/jpeg");
          else if (strstr(filename, ".gif") != 0)
            client.println("Content-Type: image/gif");
          else if (strstr(filename, ".3gp") != 0)
            client.println("Content-Type: video/mpeg");
          else if (strstr(filename, ".pdf") != 0)
            client.println("Content-Type: application/pdf");
          else if (strstr(filename, ".js") != 0)
            client.println("Content-Type: application/x-javascript");
          else if (strstr(filename, ".xml") != 0)
            client.println("Content-Type: application/xml");
          else
            client.println("Content-Type: text");
          client.println();
          int16_t c;
          while ((c = file.read()) >= 0) {
              client.write((char)c);
          }
          file.close();
        }
        else {
          client.println("HTTP/1.1 404 Not Found");
          client.println("Content-Type: text/html");
          client.println();
          client.println("<h2>File Not Found!</h2>");
        }
        break;
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
  

  while(Serial.available() > 0) {
    if (charIndex < 10) {
      inChar = Serial.read(); // Read a character
      if (inChar == '\n') {
        inData[charIndex] = 0;   // Null terminate the string
        gotData = true;
        break;
      } else {
        inData[charIndex] = inChar; // Store it
        charIndex++;                // Increment where to write next
      }
    }
  }
  if(gotData) {
    gotData = false;
    charIndex = 0;
    int servoValue = atoi(inData);
    Serial.print("Servo Value: ");
    Serial.print(servoValue);
    servo.write(servoValue);
    for (int i = 0; i <= 10; i++) inData[i] = 0;
    servoValue = '\0';
  }
  
}

void blink(int intervalMs, int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED, HIGH);
    delay(intervalMs);
    digitalWrite(LED, LOW);
    delay(intervalMs);
  }
}

void StartServer() {
  // start the Ethernet connection and the server:
  Serial.println("Starting Server");
  if(Ethernet.begin(mac)) {
    Serial.println("Opening Port");
    server.begin();
    Serial.print("Successfully obtained DHCP IP address at ");
    Serial.println(Ethernet.localIP());
    blink(100, 4);
  } else {
    Serial.println("Failed to obtain DHCP IP address.");
    Ethernet.begin(mac, ip);
    server.begin();
    Serial.print("Server now online at ");
    Serial.println(Ethernet.localIP());
  }
}

void SdInit(int baud) {
  Serial.begin(baud);
  PgmPrint("Free RAM: ");
  Serial.println(FreeRam());
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  if (!card.init(SPI_FULL_SPEED, 4)) error("card.init failed!");
  if (!volume.init(&card)) error("vol.init failed!");
  PgmPrint("Volume is FAT");
  Serial.println(volume.fatType(),DEC);
  Serial.println();
  if (!root.openRoot(&volume)) error("openRoot failed");
  PgmPrintln("Files found in root:");
  root.ls(LS_DATE | LS_SIZE);
  Serial.println();
  PgmPrintln("Files found in all dirs:");
  root.ls(LS_R);
  Serial.println();
  PgmPrintln("Done");
}
