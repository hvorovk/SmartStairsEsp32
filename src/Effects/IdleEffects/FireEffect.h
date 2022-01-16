#include "FastLED.h"
#include "../AbstractEffect.h"


struct HueConf {
    uint8_t start = 0;
    uint8_t gap = 40;
};

struct SaturationConf {
    uint8_t min = 225;
    uint8_t max = 255;
};

struct BrightConf {
    uint8_t min = 30;
    uint8_t max = 225;
};

class FireEffect: public AbstractEffect
{
    void fillLedWithProxy(const std::vector<std::vector<uint16_t>> &m_stairsMatrix,
                          CRGB *leds,
                          uint16_t n, 
                          uint16_t from = 1) override
    {
        unsigned int counterNum = counter();
        for (uint16_t i = from; i < n; i++) {
            leds[i] = firePixelColor(inoise8(i * fireStep, counterNum));
        }
    }

    uint16_t timeBetweenCalls() override
    {
        return 20;
    }

    HueConf hue = {};
    SaturationConf sat = {};
    BrightConf bright = {};

    uint8_t fireStep = 20;

private:
    CRGB firePixelColor(int noisedValue) {
        return CHSV(
            hue.start + map(noisedValue, 0, 255, 0, hue.gap),
            constrain(map(noisedValue, 0, 255, sat.max, sat.min), 0, 255),
            constrain(map(noisedValue, 0, 255, bright.min, bright.max), 0, 255)
        );
    }

    unsigned int counter() {
        m_counter += 20;

        return m_counter;
    }
    unsigned int m_counter = 0;
};
