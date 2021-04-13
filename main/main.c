#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <led_strip.h>
#include "esp_log.h"
#include <hsv2rgb.h>
#include "animcontroller.h"

//Logtag to use with esp_log
static const char* TAG = "Main";

void app_main()
{
    //Initialise our animation controller
    animctl_init();

    //Set our animation to rainbow
    animctl_setAnim(ANIM_RAINBOW);
}

