#include <FastLED.h> //strip
#include "DigitLedDisplay.h" //sseg

//logic
#define MAX_HIT_ZONE 10
#define VIRTUAL_LEDS 600
#define MODE_SIGNUP 0
#define MODE_PLAY 1
#define MODE_SCORE 2
int mode;
unsigned long mode_t;
int local_score_a;
int local_score_b;
int hit_zone;
long virtual_ball;
long ball;
int serve;
int hit_quality;
int bounce;
int btn_a_down_t;
int btn_a_up_t;
int btn_b_down_t;
int btn_b_up_t;
int btn_a_hit;
int btn_b_hit;

//strip
#define STRIP_LED_PIN     3
#define STRIP_NUM_LEDS    100
#define STRIP_BRIGHTNESS  64 //0-128
#define STRIP_LED_TYPE    WS2811
#define STRIP_COLOR_ORDER BRG
CRGB color_a;
CRGB color_b;
CRGB strip_leds[STRIP_NUM_LEDS];

//sseg
#define SSEG_BRIGHT 10 //1-15
#define SSEG_DIGITS 8
#define SSEG_LOCAL_DIN_PIN 7
#define SSEG_LOCAL_CS_PIN 6
#define SSEG_LOCAL_CLK_PIN 5

//btns
#define BTN_A_PIN 8
#define BTN_B_PIN 9

DigitLedDisplay sseg_local = DigitLedDisplay(SSEG_LOCAL_DIN_PIN, SSEG_LOCAL_CLK_PIN, SSEG_LOCAL_CS_PIN);

void setup()
{
  delay(3000); //power-up safety delay

  //strip
  color_a = CRGB::Blue;
  color_b = CRGB::Red;
  for(int i = 0; i < STRIP_NUM_LEDS; i++) strip_leds[i] = CRGB::Black;
  FastLED.addLeds<STRIP_LED_TYPE, STRIP_LED_PIN, STRIP_COLOR_ORDER>(strip_leds, STRIP_NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(STRIP_BRIGHTNESS);

  /*
  //sseg
  sseg_local.setBright(SSEG_BRIGHT);
  sseg_local.setDigitLimit(SSEG_DIGITS);

  sseg_local.clear();
  sseg_local.printDigit(22222222);
  sseg_local.printDigit(123, 4); //"start from digit 4" (?) 
  sseg_local.write(5, B01100011); //digit 5, values?
  sseg_local.off();
  sseg_local.on();
*/

  //btns
  pinMode(BTN_A_PIN,INPUT_PULLUP);
  pinMode(BTN_B_PIN,INPUT_PULLUP);

  btn_a_down_t = 0;
  btn_a_up_t = 0;
  btn_b_down_t = 0;
  btn_b_up_t = 0;

  set_mode(MODE_SIGNUP);
}

void set_mode(int m)
{
  mode = m;
  mode_t = 0;
  switch(mode)
  {
    case MODE_SIGNUP:
    {
      local_score_a = 0;
      local_score_b = 0;
    }
      break;
    case MODE_PLAY:
    {
      hit_zone = MAX_HIT_ZONE;
      virtual_ball = 0;
      ball = 0;
      serve = 1;
      hit_quality = 0;
      bounce = 0;
      btn_a_hit = -1;
      btn_b_hit = -1;
    }
      break;
    case MODE_SCORE:
    {
    }
      break;
  }
  //clear strip
  for(int i = 0; i < STRIP_NUM_LEDS; i++) strip_leds[i] = CRGB::Black;
}

void loop()
{
  //get input
  if(!digitalRead(BTN_A_PIN)) { btn_a_down_t++; btn_a_up_t = 0; }
  else { btn_a_down_t = 0; btn_a_up_t++; }
  if(!digitalRead(BTN_B_PIN)) { btn_b_down_t++; btn_b_up_t = 0; }
  else { btn_b_down_t = 0; btn_b_up_t++; }

  mode_t++; if(mode_t == 0) mode_t--; //keep at max
  switch(mode)
  {
    case MODE_SIGNUP:
    {
      //clear zones
      for(int i = 0; i < MAX_HIT_ZONE; i++) strip_leds[i] = CRGB::Black;
      for(int i = 0; i < MAX_HIT_ZONE; i++) strip_leds[STRIP_NUM_LEDS-1-i] = CRGB::Black;
      //clear the rest
      if(btn_a_up_t == 1 || btn_b_up_t == 1)
      for(int i = MAX_HIT_ZONE+1; i < STRIP_NUM_LEDS-MAX_HIT_ZONE-1; i++) strip_leds[i] = CRGB::Black;
      //draw gates
      strip_leds[MAX_HIT_ZONE] = CRGB::Yellow;
      strip_leds[STRIP_NUM_LEDS-1-MAX_HIT_ZONE] = CRGB::Yellow;

      if(btn_a_down_t > MAX_HIT_ZONE && btn_b_down_t > MAX_HIT_ZONE)
      {
        int t = btn_a_down_t;
        if(btn_b_down_t < btn_a_down_t) t = btn_b_down_t;
        if(t >= STRIP_NUM_LEDS/2)
        {
          set_mode(MODE_PLAY);
          break;
        }
        strip_leds[t] = color_a;
        strip_leds[STRIP_NUM_LEDS-1-t] = color_b;
      }
      else
      {
        if(btn_a_down_t) strip_leds[btn_a_down_t%MAX_HIT_ZONE] = color_a;
        if(btn_b_down_t) strip_leds[STRIP_NUM_LEDS-1-(btn_b_down_t%MAX_HIT_ZONE)] = color_b;
      }
      FastLED.show();
      FastLED.delay(30);
    }
      break;
    case MODE_PLAY:
    {
      //erase ball
      strip_leds[ball] = CRGB::Black;
      //erase hitzone
      strip_leds[hit_zone] = CRGB::Black;
      strip_leds[STRIP_NUM_LEDS-1-hit_zone] = CRGB::Black;
      //update ball
      int intensity = bounce/6;
      virtual_ball += serve*(2+intensity);
      hit_zone = MAX_HIT_ZONE-intensity;
      if(hit_zone < 3) hit_zone = 3;
      int old_ball = ball;
      ball = virtual_ball*STRIP_NUM_LEDS/VIRTUAL_LEDS;
      if(ball >= STRIP_NUM_LEDS) { ball = STRIP_NUM_LEDS-1; set_mode(MODE_SCORE); break; }
      else if(ball < 0)          { ball = 0;                set_mode(MODE_SCORE); break; }

      //clear hits at midpoint
      if(serve == 1)
      {
        if(btn_b_hit != -1 && old_ball < STRIP_NUM_LEDS/2 && ball >= STRIP_NUM_LEDS/2)
        {
          strip_leds[btn_b_hit] = CRGB::Black;
          btn_b_hit = -1;
        }
      }
      else if(serve == -1)
      {
        if(btn_a_hit != -1 && old_ball > STRIP_NUM_LEDS/2 && ball <= STRIP_NUM_LEDS/2)
        {
          strip_leds[btn_a_hit] = CRGB::Black;
          btn_a_hit = -1;
        }
      }

      //handle hits
      if(btn_a_hit == -1 && btn_a_down_t == 1)
      {
         btn_a_hit = ball;
         if(serve == -1)
         {
           if(ball < hit_zone)
           {
             hit_quality = hit_zone-ball;
             bounce++;
             serve = 1;
           }
         }
      }
      if(btn_b_hit == -1 && btn_b_down_t == 1)
      {
        btn_b_hit = ball;
        if(serve == 1)
        {
          if(ball > STRIP_NUM_LEDS-1-hit_zone)
          {
            hit_quality = hit_zone-(STRIP_NUM_LEDS-1-ball);
            bounce++;
            serve = -1;
          }
        }
      }

      //draw gates
      strip_leds[hit_zone] = CRGB::Yellow;
      strip_leds[STRIP_NUM_LEDS-1-hit_zone] = CRGB::Yellow;
      //draw hits
      if(btn_a_hit != -1) strip_leds[btn_a_hit] = color_a;
      if(btn_b_hit != -1) strip_leds[btn_b_hit] = color_b;
      //draw ball
      strip_leds[ball] = CRGB::White;
      FastLED.show();
      FastLED.delay(1);
    }
      break;
    case MODE_SCORE:
    {
      //clear goal zones
      for(int i = 0; i < hit_zone; i++) strip_leds[i] = CRGB::Black;
      for(int i = 0; i < hit_zone; i++) strip_leds[STRIP_NUM_LEDS-1-i] = CRGB::Black;

      //draw gates
      strip_leds[hit_zone] = CRGB::Yellow;
      strip_leds[STRIP_NUM_LEDS-1-hit_zone] = CRGB::Yellow;
      //draw hits
      if(btn_a_hit != -1) strip_leds[btn_a_hit] = color_a;
      if(btn_b_hit != -1) strip_leds[btn_b_hit] = color_b;
      //draw ball
      strip_leds[ball] = CRGB::White;

      if(serve == 1)
      {
        strip_leds[mode_t%hit_zone] = color_a;
        strip_leds[STRIP_NUM_LEDS-1-hit_zone+(mode_t%hit_zone)] = color_a;
      }
      else if(serve == -1)
      {
        strip_leds[hit_zone-1-(mode_t%hit_zone)] = color_b;
        strip_leds[STRIP_NUM_LEDS-1-(mode_t%hit_zone)] = color_b;
      }

      if(mode_t > 100) set_mode(MODE_SIGNUP);
      FastLED.show();
      FastLED.delay(10);
    }
      break;
  }

}

