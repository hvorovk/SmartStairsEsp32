#include "FastLED.h"
#include "../AbstractEffect.h"


namespace {
constexpr uint16_t spacingStars = 5;
}

class StarsEffect: public AbstractEffect
{
     void fillLedWithProxy(const std::vector<std::vector<uint16_t>> &m_stairsMatrix,
                          CRGB *leds,
                          uint16_t n, 
                          uint16_t from = 1) override
    {
        clear(leds, 0, n);

        auto counterNum = counter();
       
        for(uint16_t i = 0; i < m_starsCount; i++) {
            auto currentY = (counterNum - (i * ::spacingStars)) % m_stairsMatrix.size();

            if (currentY == 0) {
                m_positions[i] = randomX(4, 20);
            }
          
            drawOneStar(m_positions[i], currentY, m_stairsMatrix, leds, m_colors[i]);
        }
    }


    uint16_t timeBetweenCalls() override
    {
        return 50;
    }

    void refresh() override {
        for(uint16_t i = 0; i < m_starsCount; i++) {
            m_positions[i] = -1;
        } 
        m_counter = 0;
    }

private:

    uint16_t randomX(uint16_t from, uint16_t to)
    {
        return esp_random() % (to - from) + from;
    }

    void drawOneStar(int16_t x, uint16_t y,
                      const std::vector<std::vector<uint16_t>> &m_stairsMatrix,
                      CRGB *leds, CRGB color)
    {
        if (x == -1 || y >= m_stairsMatrix.size()) {
            return;
        } 

        if (y == m_stairsMatrix.size() - 1) {
            for (auto x : m_stairsMatrix[y]) {
                if (x == -1) continue;

                leds[x] = darker(color, 3);
            }
            return;
        }

        if (y == 0) {
            for (auto x : m_stairsMatrix[y]) {
                if (x == -1) continue;

                leds[x] = darker(color, 2);
            }
            return;
        }

        for (int8_t i = 0; i < m_shelfLength; i++) {
            if (y > i - 1) {
                int8_t centerColor = i == 0 ? 1 : i * 2.5;

                if (m_stairsMatrix[y - i][x] != -1)
                    leds[m_stairsMatrix[y - i][x]] = darker(color, centerColor);
                // If you have led with one diode in pack - uncomment these
                // int8_t sideColor = (i + 1) * 3;
                // if (m_stairsMatrix[y - i][x - 1] != -1)
                //     leds[m_stairsMatrix[y - i][x - 1]] = darker(color, sideColor);
                // if (m_stairsMatrix[y - i][x + 1] != -1)
                //     leds[m_stairsMatrix[y - i][x + 1]] = darker(color, sideColor);
            }
        }
    }

    CRGB darker(const CRGB &color, float factor) {
        CHSV value = rgb2hsv_approximate(color);

        value.v = static_cast<uint16_t>(value.v / factor);

        return CRGB(value);
    }

    unsigned long counter()
    {
        return m_counter++;
    }

    unsigned long m_counter = 0;

    void clear(CRGB *leds, uint16_t start = 0, uint16_t end = 0)
    {
        for (uint16_t i = start; i < end; i++) {
            leds[i] = CRGB(0,0,0);
        }
    }

    CRGB m_colors[4] = {
        CRGB(250, 250, 20),
        CRGB(50, 250, 50),
        CRGB(50, 250, 250),
        CRGB(250, 50, 50)
    };

    int16_t m_positions[4] = {-1, -1, -1, -1};

    uint16_t m_shelfLength = 4;
    uint16_t m_starsCount = 3;
};