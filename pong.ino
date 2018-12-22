#include <FastLED.h> //strip

//logic
#define MAX_HIT_ZONE 10
#define VIRTUAL_LEDS 800
#define STATE_SIGNUP 0
#define STATE_PLAY 1
#define STATE_SCORE 2
#define MODE_IMMEDIATE 0
#define MODE_HOLD 1
#define SCORE_T 100
int state;
unsigned int state_t;
int mode;
int mode_mute_t;
int local_score_a;
int local_score_b;
int speed;
int hit_zone_a;
int hit_zone_b;
long virtual_ball;
long ball;
int server;
int serve;
int bounce;
unsigned int btn_a_down_t;
unsigned int btn_a_up_t;
unsigned int btn_b_down_t;
unsigned int btn_b_up_t;
int btn_a_hit;
int btn_a_hitting;
unsigned int btn_a_hit_t;
int btn_b_hit;
int btn_b_hitting;
unsigned int btn_b_hit_t;
unsigned int particle_a_t;
unsigned int particle_b_t;

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
#define BTN_MODE_PIN 10
#define BTN_MODE_GND 12

int back(int i)
{
  return STRIP_NUM_LEDS-1-i;
}

void clear_strip()
{
  memmove8(&strip_leds[0], &strip_subleds_zone[0], MAX_HIT_ZONE*sizeof(CRGB));
  memmove8(&strip_leds[MAX_HIT_ZONE], &strip_subleds_body[0], (STRIP_NUM_LEDS-(MAX_HIT_ZONE*2))*sizeof(CRGB));
  memmove8(&strip_leds[STRIP_NUM_LEDS-MAX_HIT_ZONE], &strip_subleds_zone[0], MAX_HIT_ZONE*sizeof(CRGB));
}

void setup()
{
  delay(300); //power-up safety delay

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
  pinMode(BTN_MODE_PIN,INPUT_PULLUP);
  pinMode(BTN_MODE_GND,OUTPUT);
  digitalWrite(BTN_MODE_GND,LOW);

  btn_a_down_t = 0;
  btn_a_up_t = 0;
  btn_b_down_t = 0;
  btn_b_up_t = 0;

  mode = MODE_IMMEDIATE;
  mode_mute_t = 0;
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
      particle_a_t = STRIP_FADE_N;
      particle_b_t = STRIP_FADE_N;
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
        virtual_ball = 0;
        ball = 0;
      }
      else
      {
        server = -1;
        virtual_ball = VIRTUAL_LEDS-1;
        ball = back(0);
      }
      serve = server;
      bounce = 0;
      btn_a_hit = -1;
      btn_a_hitting = 0;
      btn_b_hit = -1;
      btn_b_hitting = 0;
      particle_a_t = STRIP_FADE_N;
      particle_b_t = STRIP_FADE_N;
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
  //get input
  if(!mode_mute_t && !digitalRead(BTN_MODE_PIN)) { mode = (mode+1)%2; mode_mute_t = 10; }
  else if(mode_mute_t) mode_mute_t--;
  if(!digitalRead(BTN_A_PIN)) { btn_a_down_t++; btn_a_up_t = 0; }
  else { btn_a_down_t = 0; btn_a_up_t++; }
  if(!digitalRead(BTN_B_PIN)) { btn_b_down_t++; btn_b_up_t = 0; }
  else { btn_b_down_t = 0; btn_b_up_t++; }

  state_t++;      if(state_t      == 0) state_t = -1; //keep at max
  particle_a_t++; if(particle_a_t == 0) particle_a_t = -1; //keep at max
  particle_b_t++; if(particle_b_t == 0) particle_b_t = -1; //keep at max

  //update
  switch(state)
  {
    case STATE_SIGNUP:
    {
      if(btn_a_down_t && !btn_b_down_t && btn_a_down_t%MAX_HIT_ZONE == 0) particle_a_t = 0;
      if(btn_b_down_t && !btn_a_down_t && btn_b_down_t%MAX_HIT_ZONE == 0) particle_b_t = 0;
      int t = btn_a_down_t;
      if(btn_b_down_t < btn_a_down_t) t = btn_b_down_t;
      if(t >= STRIP_NUM_LEDS/2) set_state(STATE_PLAY);
    }
      break;
    case STATE_PLAY:
    {
      //release
      if(btn_a_up_t) btn_a_hitting = 0;
      if(btn_b_up_t) btn_b_hitting = 0;

      if(mode == MODE_IMMEDIATE || (!btn_a_hitting && !btn_b_hitting))
      {
        //update ball
        virtual_ball += serve*speed;
        int old_ball = ball;
        ball = virtual_ball*STRIP_NUM_LEDS/VIRTUAL_LEDS;
        if(ball >= STRIP_NUM_LEDS) { ball = back(0); set_state(STATE_SCORE); break; }
        else if(ball < 0)          { ball = 0;       set_state(STATE_SCORE); break; }

        //clear hits at midpoint
             if(serve ==  1 && btn_b_hit != -1 && old_ball < STRIP_NUM_LEDS/2 && ball >= STRIP_NUM_LEDS/2) btn_b_hit = -1;
        else if(serve == -1 && btn_a_hit != -1 && old_ball > STRIP_NUM_LEDS/2 && ball <= STRIP_NUM_LEDS/2) btn_a_hit = -1;

        //handle hits
        int bounced = 0;
        if(btn_a_hit == -1 && btn_a_down_t == 1)
        {
          btn_a_hit = ball;
          particle_a_t = 0;
          if(serve == -1)
          {
            if(ball < hit_zone_a)
            {
              btn_a_hitting = 1;
              serve = 1;
              bounced = 1;
            }
          }
        }
        if(btn_b_hit == -1 && btn_b_down_t == 1)
        {
          btn_b_hit = ball;
          particle_b_t = 0;
          if(serve == 1)
          {
            if(ball > back(hit_zone_b))
            {
              btn_b_hitting = 1;
              serve = -1;
              bounced = 1;
            }
          }
        }
        if(bounced)
        {
          bounce++;
          speed = 3+(bounce/3);
          if(serve == 1) //a served
            hit_zone_a = MAX_HIT_ZONE-((bounce+2)/3);
          else //b served
            hit_zone_b = MAX_HIT_ZONE-((bounce+2)/3);
          if(hit_zone_a < 4) hit_zone_a = 4;
          if(hit_zone_b < 4) hit_zone_b = 4;
        }
      }
    }
      break;
    case STATE_SCORE:
    {
      if(state_t > SCORE_T) set_state(STATE_SIGNUP);
    }
      break;
  }

  //draw
  switch(state)
  {
    case STATE_SIGNUP:
    {
      clear_strip();

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
          int t;
          if(btn_a_down_t >= btn_b_down_t) //a first
            t = btn_b_down_t;
          else //b first
            t = btn_a_down_t;
          for(int i = MAX_HIT_ZONE+1; i <= t; i++)
          {
            strip_leds[i] = color_a;
            strip_leds[back(i)] = color_b;
          }
        }
      }

      //particles
      if(particle_a_t < STRIP_FADE_N) strip_leds[     MAX_HIT_ZONE-1+particle_a_t ] = color_particle_fade[particle_a_t];
      if(particle_b_t < STRIP_FADE_N) strip_leds[back(MAX_HIT_ZONE-1+particle_b_t)] = color_particle_fade[particle_b_t];

      FastLED.show();
      FastLED.delay(30);
    }
      break;
    case STATE_PLAY:
    {
      clear_strip();
      //clear for shrunken zones
      for(int i = hit_zone_a; i < MAX_HIT_ZONE; i++)
        strip_leds[i] = color_dark;
      for(int i = hit_zone_b; i < MAX_HIT_ZONE; i++)
        strip_leds[back(i)] = color_dark;

      //draw hits/particles
      if(btn_a_hit != -1)
      {
        if(particle_a_t < STRIP_FADE_N)
        {
          int i;
          i = btn_a_hit+particle_a_t;
          if(i < STRIP_NUM_LEDS) strip_leds[i] = color_particle_fade[particle_a_t];
          i = btn_a_hit-particle_a_t;
          if(i >= 0) strip_leds[i] = color_particle_fade[particle_a_t];
        }
        strip_leds[btn_a_hit] = color_a;
      }
      if(btn_b_hit != -1)
      {
        if(particle_b_t < STRIP_FADE_N)
        {
          int i;
          i = btn_b_hit+particle_b_t;
          if(i < STRIP_NUM_LEDS) strip_leds[i] = color_particle_fade[particle_b_t];
          i = btn_b_hit-particle_b_t;
          if(i >= 0) strip_leds[i] = color_particle_fade[particle_b_t];
        }
        strip_leds[btn_b_hit] = color_b;
      }

      //draw ball
      strip_leds[ball] = color_ball;

      FastLED.show();
      FastLED.delay(1);
    }
      break;
    case STATE_SCORE:
    {
      clear_strip();
      //clear for shrunken zones
      for(int i = hit_zone_a; i < MAX_HIT_ZONE; i++)
        strip_leds[i] = color_dark;
      for(int i = hit_zone_b; i < MAX_HIT_ZONE; i++)
        strip_leds[back(i)] = color_dark;

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

