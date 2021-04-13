#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <led_strip.h>
#include "esp_log.h"
#include <hsv2rgb.h>
#include <math.h>
#include "animcontroller.h"

//Define the type of LED strip, I have the WS2812b, which has the same timings as the WS2812
#define LED_TYPE LED_STRIP_WS2812

//We connect the data IN of the ledstrip to GPIO 15, on the LyraT board this pin is marked as MTDO by the JTAG headers
#define LED_GPIO 15

//We use RMT to send the data, and the ESP has multiple channels, we select the first one, since we dont use any other devices on our RMT channels.
#define LED_CHANNEL RMT_CHANNEL_0

//My LED strip has 256 individual WS2812B RGB LED's.
#define LED_STRIP_LEN 256

//Logtag to use with the esp logging library
static const char *TAG = "AnimController";

//In this function pointer we store our current animation, so we can easily change the running animation. We start it with the OFF animation, which turns all our LED's off
static void (*current_anim)() = &animOff;

//Here we configure our LED strip so we know how we need to communicate, etc.
static led_strip_t strip = {
    .type = LED_TYPE,
    .length = LED_STRIP_LEN,
    .gpio = LED_GPIO,
    .channel = LED_CHANNEL,
    .buf = NULL,     // We dont use a buffer, this would use extra RAM and we dont need this feature.
    .brightness = 30 // The max brightness is 255, but we set it lower so we can comfortly look at the animation without burning our eyes out. This also decreases our current usage.
};

//In this array we store pointers to all the animations, so we can easily call them using their ID.
static void (*animations[])() = {
    &animOff,
    &animBlink,
    &animBreathe,
    &animChase,
    &animFade,
    &animGlitter,
    &animPolice,
    &animPong,
    &animRailway,
    &animRainbow};

//We have one counter that manages all animations. The animation that is currently playing is the only method allowed to manage this value.
static int animCounter = 0;

//Call this before anything else, here we initialise our led strip driver. This initialises our RMT driver.
//Our Init method also creates a task for the loop, which constantly calls our animaitons
void animctl_init()
{
    led_strip_install();
    ESP_ERROR_CHECK(led_strip_init(&strip));
    
    xTaskCreate(animctl_mainLoop, "animctl_mainLoop", configMINIMAL_STACK_SIZE * 5, NULL, 5, NULL);
}

//Reset our animationcounter, and change our current animation to a different one.
void animctl_setAnim(int anim)
{
    animCounter = 0;
    current_anim = animations[anim];
}

//This is our main loop, it just calls our animations over and over again, the animations are in charge of the delay.
//This is so some animations can easily run faster that others.
void animctl_mainLoop(void *pvParameters)
{
    while (1)
    {
        current_anim();
    }
}

//No animation, just sets the whole strip to black.
void animOff()
{
    //Black color
    rgb_t blackRGB = {
        .r = 0,
        .g = 0,
        .b = 0};

    //Fill the strip with black, to display everything.
    led_strip_fill(&strip, 0, strip.length, blackRGB);
    //Actually write it to the strip.
    led_strip_flush(&strip);

    //Wait for 500ms
    vTaskDelay(pdMS_TO_TICKS(500));
}

//Blink Red, Green and Blue
void animBlink()
{
    //Our animation counter is counting from 0 to 5, so on 0, 2 and 4 we display a color, otherwise it is black.
    rgb_t colorRGB = {
        .r = 255 * (animCounter == 0),
        .g = 255 * (animCounter == 2),
        .b = 255 * (animCounter == 4)};

    led_strip_fill(&strip, 0, strip.length, colorRGB); //Fill the strip with our color
    led_strip_flush(&strip);                           //Display it on the strip

    //Increase the animation counter, if it reaches 5, we reset the counter
    if ((animCounter++) > 4)
    {
        animCounter = 0;
    }

    //Wait for 500ms for the next frame
    vTaskDelay(pdMS_TO_TICKS(500));
}

//Fade out, and fade back in
void animBreathe()
{
    //Sine wave function, this goes from 155 to 255 to create the breathing animation
    int value = 100.0 * sin(animCounter / 50.0) + 155.0;

    //Set the brightness of our white color to the value of the sine wave.
    rgb_t colorRGB = {
        .r = value,
        .g = value,
        .b = value};

    led_strip_fill(&strip, 0, strip.length, colorRGB); //Fill the strip with the color
    led_strip_flush(&strip);                           //Write to the strip

    //Increase animation counter, reset after 314
    //314 is pi * 10 so we get a repeating sine wave (not perfect, but close enough).
    if ((animCounter++) > 314)
    {
        animCounter = 0;
    }

    //Wait for 10ms for the next frame
    vTaskDelay(pdMS_TO_TICKS(10));
}

//Fill the led-strip pixel for pixel with red. Then fill it again with black
void animChase()
{
    //Here we define the color to sweep with, we use red if our counter is below 256, black if it isn't.
    rgb_t colorRGB = {
        .r = 255 * (animCounter < 256),
        .g = 0,
        .b = 0};

    led_strip_set_pixel(&strip, animCounter % 256, colorRGB); //Set that pixel to the color
    led_strip_flush(&strip);                                  //Write the pixel to the strip

    //Increase animation counter, reset after 512
    // 512 is 256 / 2
    if ((animCounter++) > 512)
    {
        animCounter = 0;
    }

    //Wait for 20ms for the next frame
    vTaskDelay(pdMS_TO_TICKS(20));
}

//Same as breathe, but fully fade to black
void animFade()
{
    //Same sine wave as breathe, but instead it goes from 0 to 255
    int value = 127.5 * sin(animCounter / 50.0) + 127.5;

    //Set the brightness of our white color to the value of the sine wave.
    rgb_t colorRGB = {
        .r = value,
        .g = value,
        .b = value};

    led_strip_fill(&strip, 0, strip.length, colorRGB); //Fill the strip with the color
    led_strip_flush(&strip);                           //Write to the strip

    //Increase animation counter, reset after 314
    //314 is pi * 100
    if ((animCounter++) > 314)
    {
        animCounter = 0;
    }

    //Wait for 10ms for the next frame
    vTaskDelay(pdMS_TO_TICKS(10));
}

//Randomly blink leds on the strip
void animGlitter()
{
    //Define a white color
    rgb_t whiteRGB = {
        .r = 255,
        .g = 255,
        .b = 255};

    //Define a black color
    rgb_t blackRGB = {
        .r = 0,
        .g = 0,
        .b = 0};

    //Randomly generate 5 pixels to blink
    int pixelsToActivate[] = {
        esp_random() % 256,
        esp_random() % 256,
        esp_random() % 256,
        esp_random() % 256,
        esp_random() % 256};

    //Turn the random pixels on
    for (int i = 0; i < 5; i++)
    {
        led_strip_set_pixel(&strip, pixelsToActivate[i], whiteRGB);
    }
    led_strip_flush(&strip);       //write to strip
    vTaskDelay(pdMS_TO_TICKS(10)); //wait 10ms

    //Turn the random pixels off again
    for (int i = 0; i < 5; i++)
    {
        led_strip_set_pixel(&strip, pixelsToActivate[i], blackRGB);
    }

    led_strip_flush(&strip);       //write to strip
    vTaskDelay(pdMS_TO_TICKS(50)); //Wait 50ms for the next animation frame
}

//Make part of the strip blue, and part red. Switch these colors every second
void animPolice()
{
    //Define a blue color
    rgb_t blueRGB = {
        .r = 0,
        .g = 0,
        .b = 255};

    //Define a red color
    rgb_t redRGB = {
        .r = 255,
        .g = 0,
        .b = 0};

    //Loop though every pixel on our strip
    for (int i = 0; i < 256; i++)
    {
        //Check if this pixel should be red or blue
        //We add the animationcounter because it will be either 0 or 1 depending on which parts should be red or blue.
        if (((i / 64) + animCounter) % 2)
        {
            //Set that pixel blue
            led_strip_set_pixel(&strip, i, blueRGB);
        }
        else
        {
            //Set that pixel red
            led_strip_set_pixel(&strip, i, redRGB);
        }
    }

    // Write our instructions to the led strip.
    led_strip_flush(&strip);

    //Swap between 0 and 1 every loop
    if ((animCounter++) >= 1)
    {
        animCounter = 0;
    }

    //Wait for 1 second
    vTaskDelay(pdMS_TO_TICKS(1000));
}

//Make one pixel red at a time, which starts at 0 and travels to the end of the strip. With a cosine function
void animPong()
{
    //Cosine function to determine the pixel to actually light up.
    //Goes from 0 to 256
    int value = 128.0 * cos(animCounter / 100.0) + 128.0;

    //Define a red color
    rgb_t redRGB = {
        .r = 255,
        .g = 0,
        .b = 0};

    //Define a black color
    rgb_t blackRGB = {
        .r = 0,
        .g = 0,
        .b = 0};

    //We set the trailing and leading two pixels off so we dont leave any pixels on,
    led_strip_set_pixel(&strip, value + 1, blackRGB);
    led_strip_set_pixel(&strip, value + 2, blackRGB);
    led_strip_set_pixel(&strip, value, redRGB);
    led_strip_set_pixel(&strip, value - 2, blackRGB);
    led_strip_set_pixel(&strip, value - 1, blackRGB);
    led_strip_flush(&strip); //Write it to our strip.

    //Increase animation counter, reset after 628
    //628 is 2 * pi * 100
    if ((animCounter++) > 628)
    {
        animCounter = 0;
    }

    vTaskDelay(pdMS_TO_TICKS(10));
}

//Osillate all the even and odd pixels on or off, every second
void animRailway()
{
    //Define a black color
    rgb_t blackRGB = {
        .r = 0,
        .g = 0,
        .b = 0};

    //Define a red color
    rgb_t redRGB = {
        .r = 255,
        .g = 0,
        .b = 0};

    //Loop though every pixel
    for (int i = 0; i < 256; i++)
    {
        //Determine if this pixel should be on or off
        //We add the animation counter to the check so we can oscillate it every second
        if ((i + animCounter) % 2)
        {
            led_strip_set_pixel(&strip, i, blackRGB); //Set that pixel black
        }
        else
        {
            led_strip_set_pixel(&strip, i, redRGB); //Set that pixel red
        }
    }

    led_strip_flush(&strip); //Write to the strip

    //Oscillate between 0 and 1 every second
    if ((animCounter++) >= 1)
    {
        animCounter = 0;
    }

    //Wait 1000ms
    vTaskDelay(pdMS_TO_TICKS(1000));
}

//Loop though every color in the rainbow that we can display on the RGB leds.
void animRainbow()
{
    //To get a rainbow of colors we use a library to convert HSV colors to RGB colors,
    //because we can easily increase the hue. The saturation and value are always at max to only get the vibrant colors
    hsv_t hsvValue = {
        .h = (uint8_t)(animCounter),
        .s = 255,
        .v = 255};

    //Here we can convert the hsv value to an rgb value based on a rainbow.
    rgb_t colorRGB = hsv2rgb_rainbow(hsvValue);

    //Fill this color on the entire strip
    led_strip_fill(&strip, 0, strip.length, colorRGB);
    led_strip_flush(&strip); //Write to strip

    //Wait for 10ms
    vTaskDelay(pdMS_TO_TICKS(10));

    //Increase our animationcounter resets after 255
    if ((animCounter++) > 255)
    {
        animCounter = 0;
    }
}