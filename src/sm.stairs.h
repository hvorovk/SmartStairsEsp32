#ifndef SM_STAIRS_H
#define SM_STAIRS_H

#define FASTLED_ALLOW_INTERRUPTS 0
#include "FastLED.h"

#include "sm.stairsconstants.h"

enum class SMStairLightEffect {
    CenterFill,
    CenterFillDiffColors
};

enum class SMStairSensorPosition {
    TopSensor,
    BottomSensor,
    None
};

class SMStairs
{
public:
    SMStairs(bool debug = false);

    void init(std::vector<int> ledByStairs);

    // It is blocking operations will block thread forever
    void startMainLoop();
    // Should made only one step(check sensors and possible turn on stairs)
    void makeStep();

private:
    // Fill the whole stairs to black(0,0,0)
    void clearLed();

    // Set color to pixel by given coordinates
    void setPixel(int stair, int pos, CRGB color);

    /*
    * Infinite loop with checking ultrasonic and light sensors  
    */
    void mainLoop();

    void turnStairsFromUp();
    void turnStairsFromDown();

    void turnOnStair(int stairIndex);
    void turnOffStair(int stairIndex);

    // Set color to first/last stairs
    void setFirstLast();

    // Getter 
    SMStairSensorPosition distanceSensorsCheck();

    // Current light status (night/day)
    LightStatus lightStatus();

    CRGB *m_leds; // Raw led storage

    SMStairLightEffect m_currentStairEffect = SMStairLightEffect::CenterFill;
    LightStatus m_lastLightStatus;
    // Proxy for simple access to leds 
    std::vector<std::vector<int>> m_stairsMatrix;

    int m_stairCount = 0;
    int m_maxLedInStair = 0;

    CRGB m_nightStandColor = CRGB(10, 5, 2);
    CRGB m_nightActiveColor = CRGB(15, 8 , 2);

    CRGB m_eveningStandColor = CRGB(40, 25, 15);
    CRGB m_eveningActiveColor[15] = {
        CRGB(220, 0 , 0),
        CRGB(220, 50 , 0),
        CRGB(200, 80 , 0),
        CRGB(200, 110 , 0),
        CRGB(190, 140 , 0),
        CRGB(50, 140 , 0),
        CRGB(0, 160 , 0),
        CRGB(0, 160 , 20),
        CRGB(0, 140 , 50),
        CRGB(0, 120 , 80),
        CRGB(0, 80 , 140),
        CRGB(0, 20 , 160),
        CRGB(0, 0 , 160),
        CRGB(30, 0 , 160),
        CRGB(60, 0 , 160)
    };

    int m_lastLightChangeTime = 0;

    // Measure distance and compare it with given
    bool compareDistance(int trigPin, int echoPin, int distance);
};

#endif // SM_STAIRS_H