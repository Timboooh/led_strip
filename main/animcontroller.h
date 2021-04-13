#ifndef ANIMCONTROLLER_H
#define ANIMCONTROLLER_H

//Enum with all the animations to easily change animations, also makes it easy to expand.
typedef enum
{
    ANIM_OFF = 0,
    ANIM_BLINK = 1,
    ANIM_BREATHE = 2,
    ANIM_CHASE = 3,
    ANIM_FADE = 4,
    ANIM_GLITTER = 5,
    ANIM_POLICE = 6,
    ANIM_PONG = 7,
    ANIM_RAILWAY = 8,
    ANIM_RAINBOW = 9,
} animation_t;

void animctl_init();
void animctl_setAnim(int anim);
void animctl_mainLoop(void *pvParameters);

void animOff();
void animBlink();
void animBreathe();
void animChase();
void animFade();
void animGlitter();
void animPolice();
void animPong();
void animRailway();
void animRainbow();

#endif