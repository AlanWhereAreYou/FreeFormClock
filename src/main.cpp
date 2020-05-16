// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include "RTClib.h"
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

#include "TM1637Display.h"
// Define the connections pins:
#define CLK 10
#define DIO 11
// Create display object of type TM1637Display:
TM1637Display display = TM1637Display(CLK, DIO);
// Create array that turns all segments on:
const uint8_t data[] = {0xff, 0xff, 0xff, 0xff};
// Create array that turns all segments off:
const uint8_t blank[] = {0x00, 0x00, 0x00, 0x00};
// You can set the individual segments per digit to spell words or create other symbols:
const uint8_t done[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
  SEG_C | SEG_E | SEG_G,                           // n
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
};

#define Pin_Hour_Up 3
#define Pin_Hour_Down 2
#define Pin_10Min_Up 5
#define Pin_10Min_Down 4
#define Pin_1Min_Up 7
#define Pin_1Min_Down 6
#define Pin_Brightness 8
#define Pin_Spare 9

#define DEBUG

struct button
{
  bool raw_old = 0;
  bool raw_new = 0;
  bool value_debounced = 0;
  bool falling_edge_latch = 0;
  bool rising_edge_latch = 0;
};

void ProcessButton(button *b, bool NewValue)
{
    b->raw_old = b->raw_new;
    b->raw_new = NewValue;

    if((b->raw_old == b->raw_new) && (b->raw_new != b->value_debounced))
    {
      // Value has settled, and the new value doesn't match the last debounced value.
      //Capture the debounced value
      b->value_debounced = b->raw_new;

      // Detect the edge
      if(b->raw_new==1)
      {
        b->rising_edge_latch = true;
        //Serial.println("Rising Edge");
      }
      else
      {
        b->falling_edge_latch = true;
        //Serial.println("Falling Edge");
      } 
    }    
}

// General idea
// Check buttons in an interrupt
// Main loop will see if an unprocessed button press has occured
// If so, process the button press
// If time change: Add/subtract time and save to RTC zeroing the seconds.
// On startup read time from RTC
// Read time from RTC every main loop

void ShowTime(DateTime value)
{
  static uint8_t TimeData[4]; 

    // Decode into digits
    int h10;
    int h1;
    int m10;
    int m1;
    int seconds; // If we are in an even second

    h10 = value.hour() / 10;
    h1 = value.hour() - h10*10;
    m10 = value.minute() / 10;
    m1 = value.minute() - m10*10;
    seconds = value.second();

    TimeData[0] = display.encodeDigit(h10);
    TimeData[1] = display.encodeDigit(h1) | (SEG_DP * (seconds/2) ); // Show the : every even second
    TimeData[2] = display.encodeDigit(m10);
    TimeData[3] = display.encodeDigit(m1);


    display.setSegments(TimeData);
}

void setup () {
    Serial.begin(9600);
    Serial.println("Starting");
    
    // Clear the display:
    display.setBrightness(0x0f);
    Serial.println("All segments");
    display.setSegments(data);
    delay(1000);
    display.clear();
    display.showNumberDec(1);

    pinMode(Pin_Hour_Up, INPUT_PULLUP);
    pinMode(Pin_Hour_Down, INPUT_PULLUP);
    pinMode(Pin_10Min_Up, INPUT_PULLUP);
    pinMode(Pin_10Min_Down, INPUT_PULLUP);
    pinMode(Pin_1Min_Up, INPUT_PULLUP);
    pinMode(Pin_1Min_Down, INPUT_PULLUP);
    pinMode(Pin_Brightness, INPUT_PULLUP);
    pinMode(Pin_Spare, INPUT_PULLUP);

    display.showNumberDec(2);
    if (! rtc.begin()) {
      Serial.println("Couldn't find RTC");
      while (1);
    }

    display.showNumberDec(3);
    if (rtc.lostPower()) {
      Serial.println("RTC lost power, lets set the time!");
      // If the RTC have lost power it will sets the RTC to the date & time this sketch was compiled in the following line
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      // This line sets the RTC with an explicit date & time, for example to set
      // January 21, 2014 at 3am you would call:
      // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }

    // If you need to set the time of the uncomment line 34 or 37
    // following line sets the RTC to the date & time this sketch was compiled
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
}

void loop () 
{
  static DateTime now;
  now = rtc.now();

  ShowTime(now);

  static button button_Hour_Up;
  ProcessButton(&button_Hour_Up,digitalRead(Pin_Hour_Up));
  if(button_Hour_Up.falling_edge_latch)
  {
    Serial.println("Hour Up Falling Edge");
    button_Hour_Up.falling_edge_latch=false;
  }
  if(button_Hour_Up.rising_edge_latch)
  {
    //Serial.println("Hour Up Rising Edge");
    button_Hour_Up.rising_edge_latch=false;
  }
 
  static button button_Hour_Down;
  ProcessButton(&button_Hour_Down,digitalRead(Pin_Hour_Down));
  if(button_Hour_Down.falling_edge_latch)
  {
    Serial.println("Hour Down Falling Edge");
    button_Hour_Down.falling_edge_latch=false;
  }
  if(button_Hour_Down.rising_edge_latch)
  {
    //Serial.println("Hour Down Rising Edge");
    button_Hour_Down.rising_edge_latch=false;
  }

  static button button_10Min_Up;
  ProcessButton(&button_10Min_Up,digitalRead(Pin_10Min_Up));
  if(button_10Min_Up.falling_edge_latch)
  {
    Serial.println("10Min Up Falling Edge");
    button_10Min_Up.falling_edge_latch=false;
  }
  if(button_10Min_Up.rising_edge_latch)
  {
    //Serial.println("10Min Up Rising Edge");
    button_10Min_Up.rising_edge_latch=false;
  }
  
  static button button_10Min_Down;
  ProcessButton(&button_10Min_Down,digitalRead(Pin_10Min_Down));
  if(button_10Min_Down.falling_edge_latch)
  {
    Serial.println("10Min Down Falling Edge");
    button_10Min_Down.falling_edge_latch=false;
  }
  if(button_10Min_Down.rising_edge_latch)
  {
    //Serial.println("10Min Down Rising Edge");
    button_10Min_Down.rising_edge_latch=false;
  }
  
  static button button_1Min_Up;
  ProcessButton(&button_1Min_Up,digitalRead(Pin_1Min_Up));
  if(button_1Min_Up.falling_edge_latch)
  {
    Serial.println("1Min Up Falling Edge");
    button_1Min_Up.falling_edge_latch=false;
  }
  if(button_1Min_Up.rising_edge_latch)
  {
    //Serial.println("1Min Up Rising Edge");
    button_1Min_Up.rising_edge_latch=false;
  }
  
  static button button_1Min_Down;
  ProcessButton(&button_1Min_Down,digitalRead(Pin_1Min_Down));
  if(button_1Min_Down.falling_edge_latch)
  {
    Serial.println("1Min Down Falling Edge");
    button_1Min_Down.falling_edge_latch=false;
  }
  if(button_1Min_Down.rising_edge_latch)
  {
    //Serial.println("1Min Down Rising Edge");
    button_1Min_Down.rising_edge_latch=false;
  }
  
  static button button_Brightness;
  ProcessButton(&button_Brightness,digitalRead(Pin_Brightness));
  if(button_Brightness.falling_edge_latch)
  {
    Serial.println("Brightness Falling Edge");
    button_Brightness.falling_edge_latch=false;
  }
  if(button_Brightness.rising_edge_latch)
  {
    //Serial.println("Brightness Rising Edge");
    button_Brightness.rising_edge_latch=false;
  }
  

    delay(100);
}


