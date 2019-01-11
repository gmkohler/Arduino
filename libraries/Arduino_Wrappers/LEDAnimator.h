#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "LoggingWrapper.h"

#ifndef WRAPPER_LED_ANIMATOR
#define WRAPPER_LED_ANIMATOR

#define ANIMATION_BLINK 1
#define ANIMATION_LOOP 2

class AnimData {
    public:
    uint8_t anim;
    uint8_t data;

    AnimData(uint8_t animation, uint8_t data) : anim(animation), data(data) {}
};

class LEDAnimator {
    private:
    uint8_t ptr = 0;
    
    uint8_t anim_size;
    uint32_t anim_color = 0;

    AnimData *animation;
    Adafruit_NeoPixel *strip;
    Logger *logger;

    public:
    LEDAnimator(
        Adafruit_NeoPixel *strip, 
        Logger *logger,
        AnimData *anim,
        uint8_t length
    ) : strip(strip), anim_size(length), logger(logger) {
        anim_size = length;
        animation = (AnimData *) malloc(sizeof(AnimData) * length);
        memcpy(animation, anim, length * sizeof(AnimData));
    }

    ~LEDAnimator() { free(animation); }

    void set_animation_color(uint32_t color) { anim_color = color; }

    void reset() { ptr = 0; }

    void step() {
        if (ptr >= anim_size) reset();

        AnimData data = animation[ptr++];
        switch (data.anim) {
            case ANIMATION_BLINK:
                animate_led_color(anim_color, 0);
                delay(data.data);
                animate_led_color(0, 0);
                break;
            case ANIMATION_LOOP:
                animate_loop(anim_color, data.data, 75);
                break;
        }
    }

    void animate_led_color(uint32_t hex, uint8_t wait) {
        animate_led_color(hex, 0, wait, false);
    }

    void animate_led_color(uint32_t hex, uint8_t start, uint8_t wait, bool interpolate) {
        for (int k = 0; k < strip->numPixels(); ++k) {
            strip->setPixelColor(get_pixel_idx(start - k), hex);
            strip->show();
            
            if (interpolate) delay(interpolate_delay(k, strip->numPixels(), wait));
            else delay(wait);
        }
    }

    void animate_loop(uint32_t hex, uint8_t start, uint8_t wait) {
        animate_led_color(hex, start, wait, true);
        animate_led_color(0, start - 1, wait, true);
    }

    private:
    uint8_t get_pixel_idx(int raw) {
        uint8_t px = strip->numPixels();
        if (raw < 0) {
            return px - (abs(raw) % px);
        }
        return raw % px;
    }

    uint8_t interpolate_delay(int step, int max, uint8_t wait) {
        double progress = (double) step / max; // [0, 1]
        double offset_progress = 0.5 - progress;
        return (uint8_t) abs(pow(offset_progress * 2, 2) * wait);
    }
};

#endif