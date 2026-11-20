/* ---------------------------------------------------------------------------
Bluetooth Wireless USB QR Scanner (Three-Class Spectral Decoder)
Program overview:
  1) The ESP32 receives scan data from a QR code scanner module over UART and displays it on an OLED screen.
  2) The decoded data can also be forwarded through UART0 and UART1 (UART1 forwarding is disabled by default).
  3) Data can be sent wirelessly over Bluetooth to a computer or smartphone. Here Bluetooth acts as a
     virtual keyboard: no driver is required, and scan results are typed directly at the current cursor
     position (e.g., into Word, Excel, or any text field).
To use Bluetooth, enable it on the host device and pair with the device named "SpectraQR".
Laptops and smartphones usually include Bluetooth; some desktop computers need a separate USB Bluetooth
adapter (any low-cost Bluetooth 4.0+ dongle works).

Communication details:
UART2 (RX, TX on ESP32 GPIO16 and GPIO17) receives data from the scanner module in polling mode at
57600 baud, 8N1. The data can be forwarded through UART1 (RX, TX on GPIO5 and GPIO4) at 9600 baud, 8N1;
it can be received on a computer through a USB-to-UART converter.
The display is a 0.96-inch OLED (SSD1306 driver) using the widely used U8g2 graphics library, which
supports UTF-8 Chinese/English text as well as all ASCII characters.

Usage:
  1. Connect a white Type-C USB cable to the computer, then compile and upload this sketch to the ESP32.
  2. Wiring:
     +----------------------------+            +--------------------+
     |                            |            |                    |
     |                       5V   |------------| 5V                 |
     |  ESP32            TX2-17   |------------| RXD      QR scanner |
     | (UART2: scanner)  RX2-16   |------------| TXD      module    |
     |                      GND   |------------| GND                |
     |                            |            |                    |
     +----------------------------+            +--------------------+
     UART1: RX, TX on GPIO5 and GPIO4.
     UART0: debug output (also used for flashing over the white USB cable; not recommended for other use).

  3. The scanner module defaults to motion-triggered scanning; a button can also trigger or stop a scan.
  4. Generate a QR code with any online generator and scan it.
  5. Observe the decoding result on the OLED display.
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
#include <ESP32-AS7343_Decoder_3class_inferencing.h>

AS7343 as;
// (Maximum length is 15 characters; anything beyond that will be truncated.)
//BleKeyboard bleKeyboard("Bluetooth Device Name", "Bluetooth Device Manufacturer", 100);
BleKeyboard bleKeyboard("SpectraQR", "Qiancheng", 100); // Device name, manufacturer, and battery level (optional, default 100%)

//HardwareSerial mySerial1(1); // The ESP32 has three hardware UARTs: Serial (UART0), mySerial1 (UART1), and mySerial2 (UART2)
HardwareSerial mySerial2(2);
U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/ 22, /* data=*/ 21, /* reset=*/ U8X8_PIN_NONE);  
// Some GPIO pins are input-only; do not change the pin assignments without consulting the ESP32 datasheet
String ReceivedData = ""; // Buffer for data received from the UART
String ReceivedData2 = ""; 
String strSHA1 = "9de0d0219b7dc81366d556ec427cf1e0bf9d1876"; // SHA-1 hash of the plaintext information
uint16_t value[16];  // Buffer for the 16 spectral channel readings

bool MessageReceived = false;
bool Encrypted = true;
int VerifyStage = 0; // Two-stage verification: 0 = waiting for stage 1, 1 = stage 1 passed, waiting for stage 2

void GetSerialStuff(void);
void OledDisplay(String DisplayContent);
void OledDisplay2(String DisplayContent);
void DisplayWelcome(void);
bool CollectAndClassify(ei_impulse_result_t *result);
void GetBestClassification(ei_impulse_result_t *result, const char **bestLabel, float *bestValue);

float features[208];
//const float features[208] = {
    // 55, 1, 2, 26, 24, 23, 8, 23, 15, 12, 2, 0, 1, 68, 1, 3, 31, 28, 29, 10, 28, 18, 15, 3, 0, 1, 55, 0, 2, 26, 24, 23, 8, 23, 15, 11, 2, 0, 1, 67, 1, 2, 30, 28, 29, 10, 27, 18, 14, 3, 0, 1
    // copy raw features here (for example from the 'Live classification' page)
    // see https://docs.edgeimpulse.com/docs/running-your-impulse-arduino
//};

/**
 * @brief      Copy raw feature data into out_ptr.
 *             This function is called by the inference library.
 *
 * @param[in]  offset   The offset
 * @param[in]  length   The length
 * @param      out_ptr  The output pointer
 *
 * @return     0
 */
int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
    memcpy(out_ptr, features + offset, length * sizeof(float));
    return 0;
}

/**
 * @brief      Acquire 16 x 13-channel spectral features and run the three-class NN classifier.
 *             Both stages of the two-stage verification use this function.
 *
 * @param[out] result  Classification results are written to this structure.
 *
 * @return     true = classification succeeded; false = invalid data length or inference failure.
 */
bool CollectAndClassify(ei_impulse_result_t *result)
{
    memset(features, 0, sizeof(features));
    for (int i = 0; i < 16; i++) {  // 16 acquisitions x 13 channels = 208 features
      as.ReadAllChannels(value);  // Read all channels
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

    if (sizeof(features) / sizeof(float) != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
        return false;
    }

    signal_t features_signal;
    features_signal.total_length = sizeof(features) / sizeof(features[0]);
    features_signal.get_data = &raw_feature_get_data;

    EI_IMPULSE_ERROR res = run_classifier(&features_signal, result, false);
    if (res != 0) return false;

    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        ei_printf("    %s: %.5f\n", result->classification[ix].label, result->classification[ix].value);
    }
    return true;
}

/**
 * @brief      Select the class with the highest confidence from the classification results.
 */
void GetBestClassification(ei_impulse_result_t *result, const char **bestLabel, float *bestValue)
{
    *bestLabel = result->classification[0].label;
    *bestValue = result->classification[0].value;
    for (size_t ix = 1; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        if (result->classification[ix].value > *bestValue) {
            *bestValue = result->classification[ix].value;
            *bestLabel = result->classification[ix].label;
        }
    }
}

void setup() 
{
  Serial.begin(115200); // UART0 baud rate for debug output
  delay(10);
  Wire.begin(21,22);
//  Serial.print("Wire initialize finished.\r\n");

  if (as.begin() != true)
    {
      //  Serial.print("AS7343 initialize error.\r\n");
        while (1)
            ;
    }
    else
      //  Serial.print("AS7343 initialize register finished.\r\n");

    // Configure sensor settings if needed
    // as.SetIntegrationTime(29, 599);
    // as.SetGain(AS7343_GAIN_256);
    // LED controlled
    as.ControlLed(false,1);
    // as.setLEDCurrent(0);

  //mySerial1.begin(115200,SERIAL_8N1,5,4); // RX, TX on GPIO5/4; reserved for debugging scan results (some modules only support 9600 baud)
  mySerial2.begin(57600,SERIAL_8N1,16,17);// RX, TX on GPIO16/17; receives QR code data from the scanner module
  
  //while (Serial.read() >= 0) {} // Flush the UART buffer to avoid stale data
  //while (mySerial1.read() >= 0) {} // Flush the UART buffer to avoid stale data
  while (mySerial2.read() >= 0) {} // Flush the UART buffer to avoid stale data
  
  pinMode(2, OUTPUT);  // Initialize the LED_BUILTIN pin as an output, D2
  digitalWrite(2, LOW);  // Onboard blue LED
  
  pinMode(32, OUTPUT); // Configure GPIO32 as an output
  digitalWrite(32, LOW); // External 365 nm UV LED on the AS7343 breakout

  pinMode(33, OUTPUT);
  digitalWrite(33, HIGH); // MD500 scanner trigger pin (active low)

  pinMode(4, INPUT);
  analogReadResolution(10); // 10-bit ADC resolution for the four-button analog ladder


  u8g2.begin();           // Initialize the display
  u8g2.enableUTF8Print(); // Enable UTF-8 text output
//  Serial.println("OLED initialize finished");

  bleKeyboard.begin();// Start the Bluetooth keyboard service; connected hosts receive keystrokes at the cursor position
  delay(500);
  if(bleKeyboard.isConnected() == 1) {
//  Serial.println("Bluetooth connected");
    digitalWrite(2, HIGH); // Bluetooth connected: turn on the onboard LED
  } else {
//  Serial.println("Bluetooth not connected");
  }
  
  DisplayWelcome();// Show the welcome screen

}

void loop() // Main loop, equivalent to while() in main()
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
    // ============ Two-stage spectral verification (the same button advances one stage per press) ============
    // The material emits a spectrum only under UV excitation, so the UV LED must be on for both acquisitions.
    // VerifyStage == 0: this press runs stage 1; the spectrum must be classified as Encrypted.
    // VerifyStage == 1: this press runs stage 2; the spectrum must be classified as Decrypted to grant access.

    if (VerifyStage == 0) {
      // ---- Stage 1: acquire the pristine crystal spectrum under UV excitation ----
      OledDisplay2("Stage 1/2: UV on|Collecting pristine|crystal spectrum...|Please wait"); 
      digitalWrite(32, HIGH);  // Turn on the 365 nm UV LED to excite the material
      delay(500);              // Wait for UV excitation to stabilize

      ei_impulse_result_t result1 = { 0 };
      bool ok1 = CollectAndClassify(&result1);
      digitalWrite(32, LOW);   // Acquisition complete: turn off the UV LED

      if (ok1) {
        const char* bestLabel1 = nullptr;
        float bestValue1 = 0;
        GetBestClassification(&result1, &bestLabel1, &bestValue1);
        ei_printf("    stage1 result: %s (%.3f)\n", bestLabel1, bestValue1);

        if (strcmp(bestLabel1, "Encrypted") == 0 && bestValue1 > 0.7) {
          // Stage 1 passed: advance to stage 2
          VerifyStage = 1;
          OledDisplay2("Stage 1 passed!|Please apply stimulus|then press again|(o^^)o");
        } else if (strcmp(bestLabel1, "Background") == 0) {
          // No valid sample spectrum detected
          Encrypted = true;
          OledDisplay2("Access denied|No sample detected|Please retry|o(T^T)o");
          digitalWrite(2, LOW);
          delay(2000);
          digitalWrite(2, HIGH);
        } else {
          // Classified as Decrypted: the pristine crystal is not in the encrypted state; deny access
          Encrypted = true;
          OledDisplay2("Access denied|Crystal is not|in encrypted state|o(T^T)o");
          digitalWrite(2, LOW);
          delay(2000);
          digitalWrite(2, HIGH);
        }
      } else {
        Encrypted = true;
        OledDisplay2("Access denied|Classification error|Please try again|o(T^T)o");
        digitalWrite(2, LOW);
        delay(2000);
        digitalWrite(2, HIGH);
      }
    } else {
      // ---- Stage 2: acquire the spectrum after stimulus under UV excitation ----
      OledDisplay2("Stage 2/2: UV on|Collecting after|stimulus...|Please wait"); 
      digitalWrite(32, HIGH);  // Turn the UV LED on again to excite the material
      delay(500);              // Wait for UV excitation to stabilize

      ei_impulse_result_t result2 = { 0 };
      bool ok2 = CollectAndClassify(&result2);
      digitalWrite(32, LOW);   // Acquisition complete: turn off the UV LED

      if (ok2) {
        const char* bestLabel2 = nullptr;
        float bestValue2 = 0;
        GetBestClassification(&result2, &bestLabel2, &bestValue2);
        ei_printf("    stage2 result: %s (%.3f)\n", bestLabel2, bestValue2);

        if (strcmp(bestLabel2, "Decrypted") == 0 && bestValue2 > 0.7) {
          // Spectral fingerprint changed from Encrypted to Decrypted: access granted
          VerifyStage = 0;   // Verification complete: return to the idle state
          Encrypted = false;
          OledDisplay2("Access granted!|Decryption successful|Please scan QR|o(^V^)o");
          for (int i = 0; i < 16; i++) {
            digitalWrite(2, LOW);
            delay(50);
            digitalWrite(2, HIGH);
            delay(50);
          }
        } else {
          // Background or Encrypted: stage 2 failed; restart the whole verification flow
          VerifyStage = 0;
          Encrypted = true;
          OledDisplay2("Access denied|Stimulus check failed|Please try again|o(T^T)o");
          digitalWrite(2, LOW);
          delay(2000);
          digitalWrite(2, HIGH);
        }
      } else {
        VerifyStage = 0;
        Encrypted = true;
        OledDisplay2("Access denied|Classification error|Please try again|o(T^T)o");
        digitalWrite(2, LOW);
        delay(2000);
        digitalWrite(2, HIGH);
      }
    }

    digitalWrite(32, LOW); // Safety: make sure the UV LED is off when leaving this flow
    delay(3000);

  } else if (valueIO < 980 && valueIO > 900) {
    //  Serial.println("Start Scanning");
    //  Serial.println(Encrypted);
      digitalWrite(33, LOW);
      GetSerialStuff();// Process data received from the QR scanner module
    } else if (valueIO > 1000) {
      Encrypted = true; // Reset to the encrypted state
      VerifyStage = 0;  // Reset verification progress to stage 1
      DisplayWelcome(); // Restore the welcome screen
    } else {
      digitalWrite(32, LOW);
      digitalWrite(33, HIGH);
    }

  delay(300);
}

// Full print helper from Edge Impulse standalone inferencing
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

// Read data from the UART
void GetSerialStuff(void) 
{
  while(mySerial2.available() > 0) { // Loop while data is available on the UART
    
    ReceivedData += char(mySerial2.read());  // Append each received byte to the buffer
    delay(2); // Short delay so the UART buffer can receive the next byte; without it data may be lost
  }
  ReceivedData2 = ReceivedData.substring(0, 40);
  if(ReceivedData.length() > 0) {
    if (Encrypted == true) {
      OledDisplay(ReceivedData);
    //  Serial.println(ReceivedData);
    } else {
      if (ReceivedData2 == strSHA1) { 
        OledDisplay("https://www.fudan.edu.cn/");
      }
    }
    MessageReceived = true;
  }
  
  if(bleKeyboard.isConnected() == 1) {
    digitalWrite(2, HIGH); // Bluetooth connected: turn on the onboard LED
    if (MessageReceived == true) {
      if (Encrypted == true) {
        //bleKeyboard.println(ReceivedData);// Output scan data wirelessly to the host (if connected)
        MessageReceived = false;
      } else {
        if (ReceivedData2 == strSHA1) {
          //bleKeyboard.print("https://www.fudan.edu.cn/");// Output the URL wirelessly to the host (if connected)
          //bleKeyboard.print("\r\n");
        }
        MessageReceived = false;
      }
    }
  } else {
	  digitalWrite(2, LOW); // Bluetooth disconnected: turn off the onboard LED
	//  Serial.println("BlueTooth connect failed!");
  }
    
  delay(100);  // Interval between consecutive scans
  ReceivedData = "";     
  
}

#if 1 // Display helpers

// Trim leading and trailing spaces (required before centering, otherwise the text is offset)
String TrimStr(const String& s)
{
  int start = 0;
  while (start < (int)s.length() && s[start] == ' ') start++;
  int end = s.length();
  while (end > start && s[end - 1] == ' ') end--;
  return s.substring(start, end);
}

// Print a line horizontally centered at row y (the x position is computed from the actual pixel width)
void OledPrintCentered(u8g2_uint_t y, const String& text)
{
  String s = TrimStr(text);
  if (s.length() == 0) return;
  u8g2_uint_t w = u8g2.getUTF8Width(s.c_str());   // Actual pixel width of the text
  u8g2_uint_t x = (u8g2.getDisplayWidth() > w) ? (u8g2.getDisplayWidth() - w) / 2 : 0;
  u8g2.setCursor(x, y);
  u8g2.print(s);
}

void OledDisplay(String DisplayContent)
{
  u8g2.setFont(u8g2_font_wqy12_t_gb2312); // The GB2312 font is recommended for full CJK glyph coverage
  u8g2.firstPage();
  do 
  {
    OledPrintCentered(15, "Scanning Result"); 
    
    if(DisplayContent.length() >= 20) 
    {            
      String SubStr0 = DisplayContent.substring(0, 20);// At most 20 characters per line with this font;
      String SubStr1 = DisplayContent.substring(20, 40);// longer content continues on the following lines;
      String SubStr2 = DisplayContent.substring(40, 60);
            
      OledPrintCentered(30, SubStr0);
      OledPrintCentered(45, SubStr1); 
      OledPrintCentered(60, SubStr2); 
     
    }
    else
    {
      OledPrintCentered(30, DisplayContent); 
    }
  } 
  while (u8g2.nextPage());// Standard U8g2 page loop to continuously refresh the display
}  
#endif

// Welcome screen
void DisplayWelcome(void)
{
  u8g2.setFont(u8g2_font_wqy12_t_gb2312); // The GB2312 font is recommended for full CJK glyph coverage
  u8g2.firstPage();
  do 
  {
    OledPrintCentered(15, "Welcome to Use");// Replace with your own title if desired 

    OledPrintCentered(32, "Intelligent Spectrally"); 

    OledPrintCentered(48, "Encrypted QR Scanner"); 
    
   } 
  while (u8g2.nextPage());
}
#endif

#if 1 // Display helpers
void OledDisplay2(String DisplayContent)
{
  u8g2.setFont(u8g2_font_wqy12_t_gb2312); // The GB2312 font is recommended for full CJK glyph coverage
  u8g2.firstPage();
  do 
  {
    if (DisplayContent.indexOf('|') >= 0) {
      // Semantic line breaks: '|' marks a line break; up to four centered lines are supported
      String lines[4];
      int idx = 0, start = 0;
      while (idx < 4) {
        int p = DisplayContent.indexOf('|', start);
        lines[idx] = (p < 0) ? DisplayContent.substring(start) : DisplayContent.substring(start, p);
        idx++;
        if (p < 0) break;
        start = p + 1;
      }
      u8g2_uint_t ys[4] = {15, 30, 45, 60};
      for (int i = 0; i < idx; i++) {
        OledPrintCentered(ys[i], lines[i]);
      }
    }
    else if(DisplayContent.length() >= 20) 
    {            
      // Fallback mode: split into fixed-size chunks when no '|' marker is present
      String SubStr0 = DisplayContent.substring(0, 22);// At most 20 characters per line with this font;
      String SubStr1 = DisplayContent.substring(22, 44);// longer content continues on the following lines;
      String SubStr2 = DisplayContent.substring(44, 66);
      String SubStr3 = DisplayContent.substring(66, 88);
            
      OledPrintCentered(15, SubStr0); 
      OledPrintCentered(30, SubStr1); 
      OledPrintCentered(45, SubStr2);
      OledPrintCentered(60, SubStr3); 
     
    }
    else
    {
      OledPrintCentered(15, DisplayContent); 
    }
  } 
  while (u8g2.nextPage());// Standard U8g2 page loop to continuously refresh the display
}  
#endif
