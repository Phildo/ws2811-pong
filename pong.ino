#include <FastLED.h> //strip

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
int hit_zone_a;
int hit_zone_b;
long virtual_ball;
long ball;
int server;
int serve;
int hit_quality;
int bounce;
int btn_a_down_t;
int btn_a_up_t;
int btn_b_down_t;
int btn_b_up_t;
int btn_a_hit;
int btn_a_hit_t;
int btn_b_hit;
int btn_b_hit_t;

//strip
#define STRIP_LED_PIN     3
#define STRIP_NUM_LEDS    100
#define STRIP_BRIGHTNESS  64 //0-128
#define STRIP_LED_TYPE    WS2811
#define STRIP_COLOR_ORDER BRG
#define STRIP_FADE_N 10
CRGB color_a;
CRGB color_b;
CRGB color_ball;
CRGB color_zone;
CRGB color_dark;
CRGB color_particle_fade[STRIP_FADE_N];
CRGB strip_leds[STRIP_NUM_LEDS];
CRGB strip_subleds_zone[MAX_HIT_ZONE];
CRGB strip_subleds_body[STRIP_NUM_LEDS-(MAX_HIT_ZONE*2)];

//btns
#define BTN_A_PIN 8
#define BTN_B_PIN 9

void clear_strip()
{
  memmove8(&strip_leds[0], &strip_subleds_zone[0], MAX_HIT_ZONE*sizeof(CRGB));
  memmove8(&strip_leds[MAX_HIT_ZONE], &strip_subleds_body[0], (STRIP_NUM_LEDS-(MAX_HIT_ZONE*2))*sizeof(CRGB));
  memmove8(&strip_leds[STRIP_NUM_LEDS-MAX_HIT_ZONE], &strip_subleds_zone[0], MAX_HIT_ZONE*sizeof(CRGB));
}

void setup()
{
  delay(3000); //power-up safety delay

  //strip
  color_a = CRGB::Blue;
  color_b = CRGB::Red;
  color_ball = CRGB::White;
  color_zone = CRGB::Yellow;
  color_zone.r /= 2;
  color_zone.g /= 2;
  color_zone.b /= 2;
  color_dark = CRGB::Black;
  for(int i = 0; i < STRIP_FADE_N; i++)
  {
    int l = 255-(255*(i+1)/(STRIP_FADE_N+1));
    color_particle_fade[i] = CRGB(l,l,l);
  }
  for(int i = 0; i < MAX_HIT_ZONE; i++) strip_subleds_zone[i] = color_zone;
  for(int i = 0; i < STRIP_NUM_LEDS-(MAX_HIT_ZONE*2); i++) strip_subleds_body[i] = color_dark;
  clear_strip();
  FastLED.addLeds<STRIP_LED_TYPE, STRIP_LED_PIN, STRIP_COLOR_ORDER>(strip_leds, STRIP_NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(STRIP_BRIGHTNESS);

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
      hit_zone_a = MAX_HIT_ZONE;
      hit_zone_b = MAX_HIT_ZONE;
      if(btn_a_down_t >= btn_b_down_t)
      {
        server = 1;
        virtual_ball = 0;
        ball = 0;
      }
      else
      {
        server = -1;
        virtual_ball = VIRTUAL_LEDS-1;
        ball = STRIP_NUM_LEDS-1;
      }
      serve = server;
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
}

void loop()
{
  //get input
  if(!digitalRead(BTN_A_PIN)) { btn_a_down_t++; btn_a_up_t = 0; }
  else { btn_a_down_t = 0; btn_a_up_t++; }
  if(!digitalRead(BTN_B_PIN)) { btn_b_down_t++; btn_b_up_t = 0; }
  else { btn_b_down_t = 0; btn_b_up_t++; }

  mode_t++; if(mode_t == 0) mode_t--; //keep at max

  //update
  switch(mode)
  {
    case MODE_SIGNUP:
    {
      if(btn_a_down_t > MAX_HIT_ZONE && btn_b_down_t > MAX_HIT_ZONE)
      {
        int t = btn_a_down_t;
        if(btn_b_down_t < btn_a_down_t) t = btn_b_down_t;
        if(t >= STRIP_NUM_LEDS/2) set_mode(MODE_PLAY);
      }
    }
      break;
    case MODE_PLAY:
    {
      //update ball
      virtual_ball += serve*(2+(bounce/3));
      if(server == 1) //a served
      {
        hit_zone_a = MAX_HIT_ZONE-((bounce+2)/3);
        hit_zone_b = MAX_HIT_ZONE-((bounce+1)/3);
      }
      else //b served
      {
        hit_zone_a = MAX_HIT_ZONE-((bounce+1)/3);
        hit_zone_b = MAX_HIT_ZONE-((bounce+2)/3);
      }
      if(hit_zone_a < 3) hit_zone_a = 3;
      if(hit_zone_b < 3) hit_zone_b = 3;

      int old_ball = ball;
      ball = virtual_ball*STRIP_NUM_LEDS/VIRTUAL_LEDS;
      if(ball >= STRIP_NUM_LEDS) { ball = STRIP_NUM_LEDS-1; set_mode(MODE_SCORE); break; }
      else if(ball < 0)          { ball = 0;                set_mode(MODE_SCORE); break; }

      //clear hits at midpoint
           if(serve ==  1 && btn_b_hit != -1 && old_ball < STRIP_NUM_LEDS/2 && ball >= STRIP_NUM_LEDS/2) btn_b_hit = -1;
      else if(serve == -1 && btn_a_hit != -1 && old_ball > STRIP_NUM_LEDS/2 && ball <= STRIP_NUM_LEDS/2) btn_a_hit = -1;

      //handle hits
      if(btn_a_hit == -1 && btn_a_down_t == 1)
      {
         btn_a_hit = ball;
         if(serve == -1)
         {
           if(ball < hit_zone_a)
           {
             hit_quality = hit_zone_a-ball;
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
          if(ball > STRIP_NUM_LEDS-1-hit_zone_b)
          {
            hit_quality = hit_zone_b-(STRIP_NUM_LEDS-1-ball);
            bounce++;
            serve = -1;
          }
        }
      }
    }
      break;
    case MODE_SCORE:
    {
      if(mode_t > 100) set_mode(MODE_SIGNUP);
    }
      break;
  }

  //draw
  switch(mode)
  {
    case MODE_SIGNUP:
    {
      clear_strip();

      if(btn_a_down_t || btn_b_down_t) //at least one down
      {
        //pulse
        if(btn_a_down_t >= btn_b_down_t) //a first
        {
          strip_leds[                             (btn_a_down_t+MAX_HIT_ZONE-1)%MAX_HIT_ZONE ] = color_a;
          strip_leds[STRIP_NUM_LEDS-MAX_HIT_ZONE+((btn_b_down_t+MAX_HIT_ZONE-1)%MAX_HIT_ZONE)] = color_a;
        }
        else //b first
        {
          strip_leds[  MAX_HIT_ZONE-1- (btn_a_down_t+MAX_HIT_ZONE-1)%MAX_HIT_ZONE ] = color_b;
          strip_leds[STRIP_NUM_LEDS-1-((btn_b_down_t+MAX_HIT_ZONE-1)%MAX_HIT_ZONE)] = color_b;
        }

        //static
        if(btn_a_down_t)
        {
          strip_leds[0] = color_a;
          strip_leds[STRIP_NUM_LEDS-1-MAX_HIT_ZONE] = color_a;
        }
        else if(btn_b_down_t)
        {
          strip_leds[MAX_HIT_ZONE] = color_b;
          strip_leds[STRIP_NUM_LEDS-1] = color_b;
        }

        //fill zone
        if(btn_a_down_t && btn_b_down_t) //both buttons down
        {
          int t;
          if(btn_a_down_t >= btn_b_down_t) //a first
            t = btn_b_down_t;
          else //b first
            t = btn_a_down_t;
          for(int i = MAX_HIT_ZONE+1; i <= t; i++)
          {
            strip_leds[i] = color_a;
            strip_leds[STRIP_NUM_LEDS-1-i] = color_b;
          }
        }

        //particles

      }

      FastLED.show();
      FastLED.delay(30);
    }
      break;
    case MODE_PLAY:
    {
      clear_strip();
      //clear for shrunken zones
      for(int i = hit_zone_a; i < MAX_HIT_ZONE; i++)
        strip_leds[i] = color_dark;
      for(int i = hit_zone_b; i < MAX_HIT_ZONE; i++)
        strip_leds[STRIP_NUM_LEDS-1-i] = color_dark;

      //draw hits
      if(btn_a_hit != -1) strip_leds[btn_a_hit] = color_a;
      if(btn_b_hit != -1) strip_leds[btn_b_hit] = color_b;
      //draw ball
      strip_leds[ball] = color_ball;

      FastLED.show();
      FastLED.delay(1);
    }
      break;
    case MODE_SCORE:
    {
      clear_strip();
      //clear for shrunken zones
      for(int i = hit_zone_a; i < MAX_HIT_ZONE; i++)
        strip_leds[i] = color_dark;
      for(int i = hit_zone_b; i < MAX_HIT_ZONE; i++)
        strip_leds[STRIP_NUM_LEDS-1-i] = color_dark;

      if(serve == 1) //a scored
      {
        strip_leds[mode_t%hit_zone_a] = color_a;
        strip_leds[STRIP_NUM_LEDS-1-hit_zone_b+(mode_t%hit_zone_b)] = color_a;
      }
      else if(serve == -1) //b scored
      {
        strip_leds[hit_zone_a-1-(mode_t%hit_zone_a)] = color_b;
        strip_leds[STRIP_NUM_LEDS-1-(mode_t%hit_zone_b)] = color_b;
      }

      //draw hits
      if(btn_a_hit != -1) strip_leds[btn_a_hit] = color_a;
      if(btn_b_hit != -1) strip_leds[btn_b_hit] = color_b;
      //draw ball
      strip_leds[ball] = color_ball;

      FastLED.show();
      FastLED.delay(10);
    }
      break;
  }

}

