/* ---------------------------------------------------------------------------
ESP32_AS7343_Decoder trainning

Program Description:
  1) Uses the ESP32 to receive QR code data from a barcode scanning module
     via serial communication and displays the result on an OLED screen.
  2) Forwards the scanned data through Serial0 and Serial1.
  3) Supports Bluetooth wireless transmission to a computer or mobile device.
     Bluetooth works as a virtual wireless keyboard, requiring no driver.
     The scanned data will be sent directly to the cursor position.

Bluetooth Instructions:
  Before using Bluetooth, enable Bluetooth on your PC or phone and connect
  to a device named "ESP32_Decoder".
  Most laptops and phones have built-in Bluetooth.
  Desktop PCs may require an external Bluetooth adapter.

Communication Details:
  - Serial2 (RX, TX mapped to ESP32 pins 16,17) is used to receive
    QR scanner data in polling mode.
    Default: 57600, 8N1.
  - Serial1 (RX, TX mapped to ESP32 pins 5,4) forwards the scanned data.
    The forwarded data can be received on a PC via a USB-to-serial module.
    Default: 9600, 8N1.
  - The display is a 1.54" OLED (SSD1306 driver),
    controlled using the widely-used u8g2 graphics library.
    Supports UTF-8 Chinese/English display and full ASCII characters.

Usage Instructions:
  1) Connect the white Type-C USB cable to your PC.
     Compile and upload this firmware to the ESP32.
  2) Connect hardware as follows:

     +--------------------------+              +--------------------+
     |                          |              |                    |
     |                     5V   |--------------| 5V                 |
     |  ESP32            TX2-17 |--------------| RXD     QR Scanner |
     | (Serial2: scanner) RX2-16|--------------| TXD     Module     |
     |                    GND   |--------------| GND                |
     |                          |              |                    |
     +--------------------------+              +--------------------+

     Serial1: RX, TX mapped to pins 5,4
     Serial0: Used for debugging (USB serial, also used for flashing firmware)

  3) Observe scanning results on the OLED screen.
 ---------------------------------------------------------------------------*/
#if 1
#include <BleKeyboard.h>
#include <Arduino.h>
#include <Wire.h>
#include <AS7343.h>
#include <U8g2lib.h>
#include <u8g2_wqy.h>

AS7343 as;
//. (Max lenght is 15 characters, anything beyond that will be truncated.)
//BleKeyboard bleKeyboard("Bluetooth Device Name", "Bluetooth Device Manufacturer", 100);
BleKeyboard bleKeyboard("ESP32_Decoder", "ESP32", 100);

// ESP32 has 3 hardware UARTs: UART0=Serial, UART1, UART2
//HardwareSerial mySerial1(1);
HardwareSerial mySerial2(2);
U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/ 22, /* data=*/ 21, /* reset=*/ U8X8_PIN_NONE);  

// Some special IO pins can only be set as input mode. Do not change them or refer to ESP32 datasheet.
String ReceivedData = ""; // Stores the received data from the scanner
uint16_t value[16];  // Stores the 16 channel values from AS7343

bool MessageReceived = false;

void GetSerialStuff(void);
void OledDisplay(String DisplayContent);
void DisplayWelcome(void);

void setup() 
{
  Serial.begin(115200); // Debug serial port, used for flashing firmware and debugging
  delay(10);
  Wire.begin(21,22);
  Serial.print("Wire initialize finished.\r\n");

  if (as.begin() != true)
    {
        Serial.print("AS7343 initialize error.\r\n");
        //while (1)
            ;
    }
    else
        Serial.print("AS7343 initialize register finished.\r\n");

    // Set AS7343 integration time and gain
    // as.SetIntegrationTime(29, 599);
    // as.SetGain(AS7343_GAIN_256);
    // LED controlled
    as.ControlLed(false,1);
    // as.setLEDCurrent(0);

  //mySerial1.begin(115200,SERIAL_8N1,5,4); // RX, TX，5,4
  mySerial2.begin(57600,SERIAL_8N1,16,17);//RX, TX，16,17
  
  // Clear serial buffer to prevent interference
  //while (Serial.read() >= 0) {} 
  //while (mySerial1.read() >= 0) {}
  while (mySerial2.read() >= 0) {} 
  
  pinMode(2, OUTPUT);  // Initialize the LED_BUILTIN pin as an output,D 2
  digitalWrite(2, LOW); 
  
  pinMode(32, OUTPUT); // D32 set as output mode
  digitalWrite(32, LOW);

  pinMode(33, OUTPUT);
  digitalWrite(33, HIGH);

  pinMode(4, INPUT);
  analogReadResolution(10);

  u8g2.begin();           // Initialize OLED display
  u8g2.enableUTF8Print(); // Enable UTF-8 character display
  Serial.println("OLED initialize finished");

  bleKeyboard.begin();// Initialize Bluetooth keyboard
  
  DisplayWelcome();// Display welcome message on OLED

}



void loop() // Main loop
{
  int valueIO = analogRead(4);
      as.ReadAllChannels(value);  // Read all 16 channel values from AS7343
      String str = String(value[8]) + "," + String(value[0]) + "," + String(value[1]) + "," + String(value[11]) + "," + String(value[2]) + "," + String(value[3]) + "," + String(value[4]) + "," + String(value[12]) + "," + String(value[10]) + "," + String(value[5]) + "," + String(value[6]) + "," + String(value[7]) + "," + String(value[9]);
      Serial.println(str); // Used for sending to the Edge Impulse platform via the Data-forwarder tool
      digitalWrite(32, LOW);
      digitalWrite(33, HIGH);
  //  }
    
  delay(50);
}


// Function to read data from serial port
void GetSerialStuff(void) 
{
  while(mySerial2.available() > 0) { // Read all available data from serial port
    
    ReceivedData += char(mySerial2.read());  // Append received character to string
    delay(2); // Short delay to allow buffer to fill
  }

  if(ReceivedData.length() > 0) {
    OledDisplay(ReceivedData);
	  MessageReceived = true;
	
  }
  
 
  if(bleKeyboard.isConnected() == 1) {
	  digitalWrite(2, HIGH); // Bluetooth connection is active, turn on LED
    if (MessageReceived == true) {
      bleKeyboard.print(ReceivedData);// Bluetooth keyboard outputs received data to computer (if connected)
      bleKeyboard.print("\r\n");
      MessageReceived = false;
    }
  } else {
	  digitalWrite(2, LOW); // Bluetooth connection is inactive, turn off LED
	  Serial.println("BlueTooth connect failed!");
  }
    
  delay(100);  // Delay between reads to allow buffer to fill
  ReceivedData = "";     
  
}


#if 1// OLED display function
void OledDisplay(String DisplayContent)
{
  u8g2.setFont(u8g2_font_wqy12_t_gb2312); // Set font to wqy12_t_gb2312, which supports all Chinese characters
  u8g2.firstPage();
  do 
  {
    u8g2.setCursor(41, 15);
    u8g2.print("Scanning Result"); // Display decoded result title on OLED
    
    if(DisplayContent.length() >= 20) 
    {            
      String SubStr0 = DisplayContent.substring(0, 20);// Split the string into three parts, each part contains 20 characters
      String SubStr1 = DisplayContent.substring(20, 40);
      String SubStr2 = DisplayContent.substring(40, 60);
            
      u8g2.setCursor(0, 30);
      u8g2.print(SubStr0); 
      u8g2.setCursor(0, 45);
      u8g2.print(SubStr1); 
      u8g2.setCursor(0, 60);
      u8g2.print(SubStr2); 
     
    }
    else
    {
      u8g2.setCursor(0, 30);
      u8g2.print(DisplayContent); 
    }
  } 
  while (u8g2.nextPage());// Refresh display page
}  
#endif

// Function to display welcome message on OLED
void DisplayWelcome(void)
{
  u8g2.setFont(u8g2_font_wqy12_t_gb2312); // Set font to wqy12_t_gb2312, which supports all Chinese characters
  u8g2.firstPage();
  do 
  {
    u8g2.setCursor(15, 15);
    u8g2.print("Welcome to train");// Display title on OLED

    u8g2.setCursor(23, 32);
    u8g2.print("Intelligent Spectrally"); 

    u8g2.setCursor(9, 48);
    u8g2.print("Encrypted QR Scanner"); 
    
   } 
  while (u8g2.nextPage());
}

#endif