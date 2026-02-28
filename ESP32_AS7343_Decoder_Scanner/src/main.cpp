/* ---------------------------------------------------------------------------
ESP32_AS7343_Decoder

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
// Modify the following line according to your project name
// Do not forget to update the platformio.ini file to point to the exported 
// Edge Impulse Arduino library zip file
#include <ESP32-AS7343_Decoder_inferencing.h>

AS7343 as;

// Bluetooth device name (max length: 15 characters)
BleKeyboard bleKeyboard("ESP32_Decoder", "ESP32", 100);
// Parameters: Bluetooth name, manufacturer name, battery level (%)

//HardwareSerial mySerial1(1);
HardwareSerial mySerial2(2);

// SSD1306 OLED using software I2C (clock=22, data=21)
U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/ 22, /* data=*/ 21, /* reset=*/ U8X8_PIN_NONE);  

// WARNING: Some ESP32 pins are input-only. Do not modify pins arbitrarily.
String ReceivedData = ""; // Stores raw serial data
String ReceivedData2 = ""; // First 40 characters of received data
String strSHA1 = "9de0d0219b7dc81366d556ec427cf1e0bf9d1876"; // SHA1 hash of the real information, ciphertext

uint16_t value[16];  // Stores 16 spectral channel values

bool MessageReceived = false;
bool Encrypted = true;

void GetSerialStuff(void);
void OledDisplay(String DisplayContent);
void OledDisplay2(String DisplayContent);
void DisplayWelcome(void);

float features[52]; // Feature buffer for Edge Impulse classifier
/**
 * @brief      Copy raw feature data in out_ptr
 *             Function called by inference library
 *
 * @param[in]  offset   The offset
 * @param[in]  length   The length
 * @param      out_ptr  The out pointer
 *
 * @return     0
 */
int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
    memcpy(out_ptr, features + offset, length * sizeof(float));
    return 0;
}

void setup() 
{
  Serial.begin(115200); // Serial0 for debugging
  delay(10);
  Wire.begin(21,22); // Initialize I2C
//  Serial.print("Wire initialize finished.\r\n");

  if (as.begin() != true)
    {
      //  Serial.print("AS7343 initialize error.\r\n");
        while (1)
            ;
    }
    else
      //  Serial.print("AS7343 initialize register finished.\r\n");

    // Optional sensor configuration
    // as.SetIntegrationTime(29, 599);
    // as.SetGain(AS7343_GAIN_256);
    // LED controlled
    as.ControlLed(false,1);
    // as.setLEDCurrent(0);

  //mySerial1.begin(115200,SERIAL_8N1,5,4); // RX, TX，5,4
  mySerial2.begin(57600,SERIAL_8N1,16,17);//RX, TX，16,17,57600, 8N1
  
  // Clear serial buffer
  //while (Serial.read() >= 0) {}
  //while (mySerial1.read() >= 0) {}
  while (mySerial2.read() >= 0) {}
  
  pinMode(2, OUTPUT);  // Initialize the LED_BUILTIN pin as an output, on-board LED
  digitalWrite(2, LOW); 

  pinMode(32, OUTPUT); // External 365 nm UVA LED
  digitalWrite(32, LOW); 

  pinMode(33, OUTPUT); // MD500 trigger pin
  digitalWrite(33, HIGH); // LOW triggers scanning

  pinMode(4, INPUT); // Analog button input
  analogReadResolution(10); // 10-bit resolution for 4-bit button input


  u8g2.begin();           // Initialize OLED
  u8g2.enableUTF8Print(); // Enable UTF-8 printing
//  Serial.println("OLED initialize finished");

  bleKeyboard.begin();// Initialize Bluetooth keyboard
  delay(500);

  if(bleKeyboard.isConnected() == 1) {
//  Serial.println("Bluetooth connected");
    digitalWrite(2, HIGH); // On-board LED ON when Bluetooth connected
  } else {
//  Serial.println("Bluetooth not connected");
  }
  
  DisplayWelcome();// Display welcome message on OLED

}

void loop() // Main loop
{
  int valueIO = analogRead(4);
  // int Index = 0;
   String strValueIO = String(valueIO);
  // bleKeyboard.println(strValueIO);
  // OledDisplay2(strValueIO);
  if (valueIO < 830 && valueIO > 700) {
    digitalWrite(32, HIGH);
  // Serial.println("Encrypted");
    delay(50);
  } else if (valueIO < 890 && valueIO > 830) {  
    OledDisplay2("                           Decrypting...          Please wait"); 
    digitalWrite(32, HIGH);
    memset(features, 0, sizeof(features));
    for (int i = 0; i < 4; i++) {  // Collect 4 sets of features
      as.ReadAllChannels(value);  // Read all 16 channels
      features[i * 13 + 0] = value[8];
      features[i * 13 + 1] = value[0];
      features[i * 13 + 2] = value[1];
      features[i * 13 + 3] = value[11];
      features[i * 13 + 4] = value[2];
      features[i * 13 + 5] = value[3];
      features[i * 13 + 6] = value[4];
      features[i * 13 + 7] = value[12];
      features[i * 13 + 8] = value[10];
      features[i * 13 + 9] = value[5];
      features[i * 13 + 10] = value[6];
      features[i * 13 + 11] = value[7];
      features[i * 13 + 12] = value[9];
      digitalWrite(2, LOW);
      delay(125);
      digitalWrite(2, HIGH);
      delay(125);
    }
    digitalWrite(32, LOW);
  //ei_printf("Edge Impulse standalone inferencing\n");

  if (sizeof(features) / sizeof(float) != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
  //    ei_printf("The size of your 'features' array is not correct. Expected %lu items, but had %lu\n",
  //    EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, sizeof(features) / sizeof(float));
        delay(2000);
        return;
    }

    ei_impulse_result_t result = { 0 };

    // the features are stored into flash, and we don't want to load everything into RAM
    signal_t features_signal;
    features_signal.total_length = sizeof(features) / sizeof(features[0]);
    features_signal.get_data = &raw_feature_get_data;

    // invoke the impulse
    EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false); // debug
  //  ei_printf("run_classifier returned: %d\n", res);

    if (res != 0) return;

    // human-readable predictions
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
  //      ei_printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
    }
    #if EI_CLASSIFIER_HAS_ANOMALY == 1
  //      ei_printf("    anomaly score: %.3f\n", result.anomaly);
    #endif

  if (result.classification[1].value > 0.9) {
      //  ei_printf("    result: unencrypted\n");
        Encrypted = false;
        OledDisplay2(" Decryption successful     Please scan           the QR code           o(￣V￣)o");
        for (int i = 0; i < 16; i++) {
          digitalWrite(2, LOW);
          delay(50);
          digitalWrite(2, HIGH);
          delay(50);
        }
    } else {
      //  ei_printf("    result: encrypted\n");
        Encrypted = true;
        OledDisplay2("    Decryption failed     Please try again         o(T^T)o");
        digitalWrite(2, LOW);
        delay(2000);
        digitalWrite(2, HIGH);
    }

    delay(3000);

  } else if (valueIO < 980 && valueIO > 900) {
    //  Serial.println("Start Scanning");
    //  Serial.println(Encrypted);
      digitalWrite(33, LOW);
      GetSerialStuff();// Process serial data received from QR code scanner
    } else if (valueIO > 1000) {
      Encrypted = true; // Reset to encrypted state
      DisplayWelcome(); // Show welcome message on OLED
    } else {
      digitalWrite(32, LOW);
      digitalWrite(33, HIGH);
    }

  delay(300);
}

// Print function for Edge Impulse standalone inferencing
void ei_printf(const char *format, ...) {
    static char print_buf[1024] = { 0 };

    va_list args;
    va_start(args, format);
    int r = vsnprintf(print_buf, sizeof(print_buf), format, args);
    va_end(args);

    if (r > 0) {
        Serial.write(print_buf);
    }
}

// Process serial data received from QR code scanner
void GetSerialStuff(void) 
{
  while(mySerial2.available() > 0) { // Process serial data byte by byte
    
    ReceivedData += char(mySerial2.read());  // Append received byte to ReceivedData string
    delay(2); // Delay to allow next byte to be received
  }
  ReceivedData2 = ReceivedData.substring(0, 40);
  if(ReceivedData.length() > 0) {
    if (Encrypted == true) {
      OledDisplay(ReceivedData);
    //  Serial.println(ReceivedData);
    } else {
      if (ReceivedData2 == strSHA1) { 
        OledDisplay("https://www.fudan.edu.cn/"); // plain text
      }
    }
    MessageReceived = true;
  }
  
  if(bleKeyboard.isConnected() == 1) {
    digitalWrite(2, HIGH); // Bluetooth connection successful, turn on LED
    if (MessageReceived == true) {
      if (Encrypted == true) {
        //bleKeyboard.println(ReceivedData);// Bluetooth keyboard wireless output to computer (if connected)
        MessageReceived = false;
      } else {
        if (ReceivedData2 == strSHA1) {
          //bleKeyboard.print("https://www.fudan.edu.cn/");// Bluetooth keyboard wireless output to computer (if connected)
          //bleKeyboard.print("\r\n");
        }
        MessageReceived = false;
      }
    }
  } else {
	  digitalWrite(2, LOW); // Bluetooth connection failed, turn off LED
	//  Serial.println("BlueTooth connect failed!");
  }
    
  delay(100);  // Delay to allow next byte to be received
  ReceivedData = "";     
  
}

#if 1//显示测试
void OledDisplay(String DisplayContent)
{
  u8g2.setFont(u8g2_font_wqy12_t_gb2312); // Set font to wqy12_t_gb2312, which supports all Chinese characters
  u8g2.firstPage();
  do 
  {
    u8g2.setCursor(23, 15);
    u8g2.print("Scanning Result"); 
    
    if(DisplayContent.length() >= 20) 
    {            
      String SubStr0 = DisplayContent.substring(0, 20);// Display first 20 characters on first line
      String SubStr1 = DisplayContent.substring(20, 40);// Display next 20 characters on second line
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

//欢迎界面
void DisplayWelcome(void)
{
  u8g2.setFont(u8g2_font_wqy12_t_gb2312); // Set font to wqy12_t_gb2312, which supports all Chinese characters
  u8g2.firstPage();
  do 
  {
    u8g2.setCursor(23, 15);
    u8g2.print("Welcome to Use");// Display welcome message

    u8g2.setCursor(16, 32);
    u8g2.print("Intelligent Spectrally"); 

    u8g2.setCursor(9, 48);
    u8g2.print("Encrypted QR Scanner"); 
    
   } 
  while (u8g2.nextPage());
}
#endif

#if 1//显示测试
void OledDisplay2(String DisplayContent)
{
  u8g2.setFont(u8g2_font_wqy12_t_gb2312); // Set font to wqy12_t_gb2312, which supports all Chinese characters
  u8g2.firstPage();
  do 
  {
    //u8g2.setCursor(23, 15);
    //u8g2.print("Scanning Result"); 
    if(DisplayContent.length() >= 20) 
    {            
      String SubStr0 = DisplayContent.substring(0, 22);// Display first 22 characters on first line
      String SubStr1 = DisplayContent.substring(22, 44);// Display next 22 characters on second line
      String SubStr2 = DisplayContent.substring(44, 66);
      String SubStr3 = DisplayContent.substring(66, 88);
            
      u8g2.setCursor(0, 15);
      u8g2.print(SubStr0); 
      u8g2.setCursor(0, 30);
      u8g2.print(SubStr1); 
      u8g2.setCursor(0, 45);
      u8g2.print(SubStr2);
      u8g2.setCursor(0, 60);
      u8g2.print(SubStr3); 
     
    }
    else
    {
      u8g2.setCursor(0, 15);
      u8g2.print(DisplayContent); 
    }
  } 
  while (u8g2.nextPage());// Refresh display page
}  
#endif