#include <FastLED.h> //strip

//logic
#define MAX_HIT_ZONE 10
#define VIRTUAL_LEDS 800
#define STATE_SIGNUP 0
#define STATE_PLAY 1
#define STATE_SCORE 2
#define SCORE_T 100

int state;
unsigned int state_t;
int local_score_a;
int local_score_b;
int speed;
int hit_zone_a;
int hit_zone_b;
long virtual_ball_p;
long ball_p;
int server;
int serve;
int bounce;
unsigned int btn_a_down_t;
unsigned int btn_a_up_t;
unsigned int btn_b_down_t;
unsigned int btn_b_up_t;
int btn_a_hit_p;
int missile_a_hit_p;
unsigned int btn_a_hit_t;
int btn_b_hit_p;
int missile_b_hit_p;
unsigned int btn_b_hit_t;

//strip
#define STRIP_LED_PIN     3
#define STRIP_NUM_LEDS    100
#define STRIP_BRIGHTNESS  128 //0-128
#define STRIP_LED_TYPE    WS2811
#define STRIP_COLOR_ORDER BRG
#define STRIP_FADE_N 10

CRGB black;
CRGB white;
CRGB red;
CRGB green;
CRGB blue;
CRGB yellow;
CRGB light_blue;
CRGB dark_blue;
CRGB pink;

CRGB color_a;
CRGB color_b;
CRGB color_ball;
CRGB color_zone;
CRGB color_zone_fade[STRIP_FADE_N];
CRGB color_dark;
CRGB color_fade[STRIP_FADE_N];
CRGB strip_leds[STRIP_NUM_LEDS];

//btns
#define BTN_A_PIN 8
#define BTN_B_PIN 9

int back(int i)
{
  return STRIP_NUM_LEDS-1-i;
}

void cache_colors()
{
  for(int i = 0; i < STRIP_FADE_N; i++)
  {
    int l = 255*(i+1)/STRIP_FADE_N;
    color_fade[i] = CRGB(l,l,l);
    int r = color_zone.r*(i+1)/STRIP_FADE_N;
    int g = color_zone.g*(i+1)/STRIP_FADE_N;
    int b = color_zone.b*(i+1)/STRIP_FADE_N;
    color_zone_fade[i] = CRGB(r,g,b);
  }
}

void init_strip()
{
  int i = 0;
  for(; i < MAX_HIT_ZONE; i++)
    strip_leds[i] = color_zone;
  for(; i < STRIP_NUM_LEDS-MAX_HIT_ZONE; i++)
    strip_leds[i] = color_dark;
  for(; i < STRIP_NUM_LEDS; i++)
    strip_leds[i] = color_zone;
}

void draw_pulsed_zones()
{
  //a
  {
    int half = hit_zone_a/2;
    int ceilhalf = (hit_zone_a+1)/2;
    if(btn_a_down_t && btn_a_down_t < STRIP_FADE_N)
    {
      for(int i = 0; i < ceilhalf; i++)
      {
        int f = (STRIP_FADE_N-1-((i*STRIP_FADE_N)/half))*btn_a_down_t/STRIP_FADE_N; //\_/
        strip_leds[i]              = color_zone_fade[f];
        strip_leds[hit_zone_a-1-i] = color_zone_fade[f];
      }
    }
    else
    {
      for(int i = 0; i < ceilhalf; i++)
      {
        int f = STRIP_FADE_N-1-((i*STRIP_FADE_N)/half); //\_/
        strip_leds[i]              = color_zone_fade[f];
        strip_leds[hit_zone_a-1-i] = color_zone_fade[f];
      }
    }
    if(btn_a_down_t < hit_zone_a)
      strip_leds[btn_a_down_t] = color_a;
  }

  //b
  {
    int half = hit_zone_b/2;
    int ceilhalf = (hit_zone_b+1)/2;
    if(btn_b_down_t && btn_b_down_t < STRIP_FADE_N)
    {
      for(int i = 0; i < ceilhalf; i++)
      {
        int f = (STRIP_FADE_N-1-((i*STRIP_FADE_N)/half))*btn_b_down_t/STRIP_FADE_N; //\_/
        strip_leds[back(i)]              = color_zone_fade[f];
        strip_leds[back(hit_zone_b-1-i)] = color_zone_fade[f];
      }
    }
    else
    {
      for(int i = 0; i < ceilhalf; i++)
      {
        int f = STRIP_FADE_N-1-((i*STRIP_FADE_N)/half); //\_/
        strip_leds[back(i)]              = color_zone_fade[f];
        strip_leds[back(hit_zone_b-1-i)] = color_zone_fade[f];
      }
    }
    if(btn_b_down_t < hit_zone_b)
      strip_leds[back(btn_b_down_t)] = color_b;
  }
}

void clear_lane()
{
  int imax = STRIP_NUM_LEDS-hit_zone_b;
  for(int i = hit_zone_a; i < imax; i++)
    strip_leds[i] = color_dark;
}

void setup()
{
  delay(300); //power-up safety delay

  //helpful constants for color-picking
  black      = CRGB(0x00,0x00,0x00); //CRGB::Black;
  white      = CRGB(0xFF,0xFF,0xFF); //CRGB::White;
  red        = CRGB(0xFF,0x00,0x00); //CRGB::Red;
  green      = CRGB(0x00,0xFF,0x00); //CRGB::Green;
  blue       = CRGB(0x00,0x00,0xFF); //CRGB::Blue;
  yellow     = CRGB(0xFF,0xFF,0x00); //CRGB::Yellow;
  light_blue = CRGB(0x4E,0xFD,0xEE);
  dark_blue  = CRGB(0x3C,0x44,0xE8);
  pink       = CRGB(0xF6,0x49,0xC7);

  //strip
  color_a = light_blue;
  color_b = dark_blue;
  color_ball = white;
  color_zone = pink;
  color_zone /= 2;
  color_zone /= 2;
  color_zone /= 2;
  color_dark = black;
  cache_colors();
  init_strip();
  FastLED.addLeds<STRIP_LED_TYPE, STRIP_LED_PIN, STRIP_COLOR_ORDER>(strip_leds, STRIP_NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(STRIP_BRIGHTNESS);

  //btns
  pinMode(BTN_A_PIN,INPUT_PULLUP);
  pinMode(BTN_B_PIN,INPUT_PULLUP);

  btn_a_down_t = 0;
  btn_a_up_t = 0;
  btn_b_down_t = 0;
  btn_b_up_t = 0;

  set_state(STATE_SIGNUP);
}

void set_state(int s)
{
  state = s;
  state_t = 0;
  switch(state)
  {
    case STATE_SIGNUP:
    {
      if(btn_a_down_t >= btn_b_down_t)
      {
        btn_a_down_t = 1;
        btn_b_down_t = 0;
      }
      else
      {
        btn_a_down_t = 0;
        btn_b_down_t = 1;
      }
      local_score_a = 0;
      local_score_b = 0;
    }
      break;
    case STATE_PLAY:
    {
      speed = 3;
      hit_zone_a = MAX_HIT_ZONE;
      hit_zone_b = MAX_HIT_ZONE;
      if(btn_a_down_t >= btn_b_down_t)
      {
        server = 1;
        virtual_ball_p = 0;
        ball_p = 0;
      }
      else
      {
        server = -1;
        virtual_ball_p = VIRTUAL_LEDS-1;
        ball_p = back(0);
      }
      serve = server;
      bounce = 0;
      btn_a_hit_p = -1;
      missile_a_hit_p = -1;
      btn_b_hit_p = -1;
      missile_b_hit_p = -1;
    }
      break;
    case STATE_SCORE:
    {
    }
      break;
  }
}

void loop()
{
  //read buttons
  if(!digitalRead(BTN_A_PIN)) { btn_a_down_t++; btn_a_up_t = 0; }
  else { btn_a_down_t = 0; btn_a_up_t++; }
  if(!digitalRead(BTN_B_PIN)) { btn_b_down_t++; btn_b_up_t = 0; }
  else { btn_b_down_t = 0; btn_b_up_t++; }

  state_t++; if(state_t == 0) state_t = -1; //keep at max

  int delay_t = 30;
  //update
  switch(state)
  {
    case STATE_SIGNUP:
    {
      int t = btn_a_down_t;
      if(btn_b_down_t < btn_a_down_t) t = btn_b_down_t;
      if(t >= STRIP_NUM_LEDS/2) set_state(STATE_PLAY);
      delay_t = 30;
    }
      break;
    case STATE_PLAY:
    {
      //update ball
      virtual_ball_p += serve*speed;
      int old_ball_p = ball_p;
      ball_p = virtual_ball_p*STRIP_NUM_LEDS/VIRTUAL_LEDS;
      if(ball_p >= STRIP_NUM_LEDS) { ball_p = back(0); set_state(STATE_SCORE); break; }
      else if(ball_p < 0)          { ball_p = 0;       set_state(STATE_SCORE); break; }

      //clear hits at midpoint
           if(serve ==  1 && btn_b_hit_p != -1 && old_ball_p < STRIP_NUM_LEDS/2 && ball_p >= STRIP_NUM_LEDS/2) { btn_b_hit_p = -1; missile_b_hit_p = -1; }
      else if(serve == -1 && btn_a_hit_p != -1 && old_ball_p > STRIP_NUM_LEDS/2 && ball_p <= STRIP_NUM_LEDS/2) { btn_a_hit_p = -1; missile_a_hit_p = -1; }

      //handle hits
      int should_bounce = 0;
      if(missile_a_hit_p == -1 && btn_a_down_t == 1)
      {
        btn_a_hit_p = ball_p;
        if(serve == -1)
        {
          if(btn_a_down_t < hit_zone_a && ball_p <= btn_a_down_t)
          {
            missile_a_hit_p = ball_p;
            should_bounce = 1;
          }
        }
      }
      if(btn_b_hit_p == -1 && btn_b_down_t == 1)
      {
        btn_b_hit_p = ball_p;
        if(serve == 1)
        {
          if(btn_b_down_t > back(hit_zone_a) && ball_p >= back(btn_b_down_t))
          {
            missile_b_hit_p = ball_p;
            should_bounce = 1;
          }
        }
      }
      if(should_bounce)
      {
        bounce++;
        speed = 3+(bounce/3);
        if(serve == -1) { serve =  1; hit_zone_a = MAX_HIT_ZONE-((bounce+2)/3); } //a served
        if(serve ==  1) { serve = -1; hit_zone_b = MAX_HIT_ZONE-((bounce+2)/3); } //b served
        if(hit_zone_a < 4) hit_zone_a = 4;
        if(hit_zone_b < 4) hit_zone_b = 4;
      }
      delay_t = 1;
    }
      break;
    case STATE_SCORE:
    {
      if(state_t > SCORE_T) set_state(STATE_SIGNUP);
      delay_t = 10;
    }
      break;
  }

  //draw
  //for(int i = 0; i < 10; i++) //used to test performance
  {
    switch(state)
    {
      case STATE_SIGNUP:
      {
        draw_pulsed_zones();

        if(btn_a_down_t || btn_b_down_t) //at least one down
        {
          //pulse
          if(btn_a_down_t >= btn_b_down_t) //a first
          {
            strip_leds[                             (btn_a_down_t+MAX_HIT_ZONE-1)%MAX_HIT_ZONE ] = color_a;
            strip_leds[STRIP_NUM_LEDS-MAX_HIT_ZONE+((btn_a_down_t+MAX_HIT_ZONE-1)%MAX_HIT_ZONE)] = color_a;
          }
          else //b first
          {
            strip_leds[  MAX_HIT_ZONE-1- (btn_b_down_t+MAX_HIT_ZONE-1)%MAX_HIT_ZONE ] = color_b;
            strip_leds[STRIP_NUM_LEDS-1-((btn_b_down_t+MAX_HIT_ZONE-1)%MAX_HIT_ZONE)] = color_b;
          }

          //static
          if(btn_a_down_t)
          {
            strip_leds[0] = color_a;
            strip_leds[back(MAX_HIT_ZONE)] = color_a;
          }
          if(btn_b_down_t)
          {
            strip_leds[MAX_HIT_ZONE] = color_b;
            strip_leds[back(0)] = color_b;
          }

          //fill zone
          if(btn_a_down_t && btn_b_down_t) //both buttons down
          {
            int t = btn_a_down_t;
            if(btn_b_down_t < btn_a_down_t) t = btn_b_down_t;
            for(int i = MAX_HIT_ZONE+1; i <= t; i++)
            {
              strip_leds[i] = color_a;
              strip_leds[back(i)] = color_b;
            }
          }

        }
      }
        break;
      case STATE_PLAY:
      {
        draw_pulsed_zones();
        clear_lane();

        //draw hits
        if(missile_a_hit_p != -1) strip_leds[missile_a_hit_p] = color_a;
        if(missile_b_hit_p != -1) strip_leds[missile_b_hit_p] = color_b;

        //draw ball
        strip_leds[ball_p] = color_ball;
      }
        break;
      case STATE_SCORE:
      {
        draw_pulsed_zones();
        clear_lane();

        if(serve == 1) //a scored
        {
          strip_leds[state_t%hit_zone_a] = color_a;
          strip_leds[STRIP_NUM_LEDS-hit_zone_b+(state_t%hit_zone_b)] = color_a;
          int body_leds = STRIP_NUM_LEDS-hit_zone_a-hit_zone_b+2;
          int pulse_t = SCORE_T/10;
          int pulse_is = hit_zone_a+(( state_t   %pulse_t)*body_leds/pulse_t);
          int pulse_ie = hit_zone_a+(((state_t+1)%pulse_t)*body_leds/pulse_t);
          if((state_t+1)%pulse_t == 0) pulse_ie = STRIP_NUM_LEDS-hit_zone_b+1;
          if(pulse_is == pulse_ie) pulse_ie++;
          for(int i = pulse_is; i < pulse_ie; i++)
            strip_leds[i] = color_a;
        }
        else if(serve == -1) //b scored
        {
          strip_leds[hit_zone_a-1-(state_t%hit_zone_a)] = color_b;
          strip_leds[STRIP_NUM_LEDS-1-(state_t%hit_zone_b)] = color_b;
          int body_leds = STRIP_NUM_LEDS-hit_zone_a-hit_zone_b+2;
          int pulse_t = SCORE_T/10;
          int pulse_is = hit_zone_b+(( state_t   %pulse_t)*body_leds/pulse_t);
          int pulse_ie = hit_zone_b+(((state_t+1)%pulse_t)*body_leds/pulse_t);
          if((state_t+1)%pulse_t == 0) pulse_ie = STRIP_NUM_LEDS-hit_zone_a+1;
          if(pulse_is == pulse_ie) pulse_ie++;
          for(int i = pulse_is; i < pulse_ie; i++)
            strip_leds[back(i)] = color_b;
        }

        //draw hits
        if(btn_a_hit_p != -1) strip_leds[btn_a_hit_p] = color_a;
        if(btn_b_hit_p != -1) strip_leds[btn_b_hit_p] = color_b;

        //draw ball
        strip_leds[ball_p] = color_ball;
      }
        break;
    }
  }

  //refresh
  FastLED.show();
  FastLED.delay(delay_t);
}

