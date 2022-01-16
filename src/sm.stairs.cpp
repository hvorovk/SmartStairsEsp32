#include "sm.stairs.h"
#include "Effects/IdleEffects/FireEffect.h"
#include "Effects/IdleEffects/StarsEffect.h"


SMStairs::SMStairs(bool debug)
{
    
}

void SMStairs::startMainLoop()
{
    while (true) {
        mainLoop();
    }
}

void SMStairs::makeStep()
{
    mainLoop();
}

void SMStairs::init(std::vector<uint16_t> ledByStairs)
{
    m_effects = {
        new StarsEffect(),
        new FireEffect()
    };
    m_effect = new StarsEffect();   
    m_stairCount = ledByStairs.size();
    m_maxLedInStair = *std::max_element(ledByStairs.begin(), ledByStairs.end());

    m_ledCount = 1;
    for (uint16_t i = 0; i < m_stairCount; i++) {
        m_stairsMatrix.push_back(std::vector<uint16_t>(m_maxLedInStair, -1));
        m_ledCount += ledByStairs[i];
    }

    pinMode(SmartStairsPin::lightNight, INPUT);
    pinMode(SmartStairsPin::topTrigger, OUTPUT);
    pinMode(SmartStairsPin::bottomTrigger, OUTPUT);
    pinMode(SmartStairsPin::topEcho, INPUT);
    pinMode(SmartStairsPin::bottomEcho, INPUT);

    m_leds = new CRGB[m_ledCount];
    FastLED.addLeds<WS2811, SmartStairsPin::ledStrip, BRG>(m_leds, m_ledCount)
                                .setCorrection(TypicalLEDStrip);

    clearLed();

    FastLED.setBrightness(255);

    uint16_t shift = 1;
    uint16_t middleElement = static_cast<uint16_t>(m_maxLedInStair / 2. + 0.5);

    double modificator = m_maxLedInStair % 2 ? 1. : 0.5;

    for (uint16_t i = 0; i < m_stairCount; i++) {
        const uint16_t pixelsInRow = ledByStairs[i];

        uint16_t counter = 0;

        uint16_t shiftArray[m_maxLedInStair];

        for(uint16_t j = 0; j < pixelsInRow; j++, shift++) {
            shiftArray[counter] = shift;
            counter++;
        }

        uint16_t currentStairMiddleElement = static_cast<uint16_t>(pixelsInRow / 2. + modificator);

        uint16_t startPosition = middleElement - currentStairMiddleElement;

        for (uint16_t j = startPosition, k = 0; k < counter; j++, k++) {
            m_stairsMatrix[i][j] = shiftArray[k];
        }
    }

    m_lastLightStatus = lightStatus();
    setFirstLast();
    m_lastLightChangeTime = millis();
    Serial.println("init finished");
}

void SMStairs::mainLoop()
{
    if (millis() - m_lastLightChangeTime > SmartStairsConfig::effectTimeToChange) {
        nextEffect();
        m_lastLightChangeTime = millis();
    }

    if (millis() - m_lastLightChangeTime > 1000) {
        m_lastLightStatus = lightStatus();
        setFirstLast();
        m_lastLightChangeTime = millis();
    }

    SMStairSensorPosition position = distanceSensorsCheck();

    if (position == SMStairSensorPosition::None) {
        if (m_lastLightStatus != LightStatus::night) {
            if (millis() - m_lastEffectTime > m_effect->timeBetweenCalls()) {
                m_effect->fillLedWithProxy(m_stairsMatrix, m_leds, m_ledCount);
                FastLED.show();
                m_lastEffectTime = millis();
                m_firstLastDirtyStatus = true;
            }
        }
       
        return;
    }
    m_effect->refresh();

    if (position == SMStairSensorPosition::BottomSensor) {
        turnStairsFromDown();
        m_lastEffectTime = millis();
        return;
    }

    turnStairsFromUp();
    m_lastEffectTime = millis();
}

void SMStairs::nextEffect()
{
    auto it = find(m_effects.begin(), m_effects.end(), m_effect);


    int currentEffectIndex = it - m_effects.begin();

    currentEffectIndex++;
    if (currentEffectIndex >= m_effects.size()) {
        currentEffectIndex = 0;
    }

    m_effect = m_effects[currentEffectIndex];
    m_effect->refresh();
}
 
void SMStairs::setFirstLast()
{
    if (!m_firstLastDirtyStatus) {
        return; // If led has not changed it's status so we don't need to update it as well
    }

    if (m_lastLightStatus == LightStatus::night) {
        FastLED.clear(false);
        for (uint16_t i = 0; i < m_maxLedInStair; i++) {
            setPixel(0, i, m_nightStandColor);
            setPixel(m_stairCount -1, i, m_nightStandColor);
        }
        
        FastLED.show();
        m_firstLastDirtyStatus = false;
        m_effect->refresh();
    }
}

void SMStairs::turnStairsFromUp()
{
    for (uint16_t i = 0; i < m_stairCount; i++) { // Run through all stairs
        turnOnStair(i);
        delay(SmartStairsConfig::delayBetweenStairs);
    }

    delay (SmartStairsConfig::delayBetweenOnOff);

    for (uint16_t i = 0; i < m_stairCount; i++) { // Run through all stairs
        turnOffStair(i);
        delay(SmartStairsConfig::delayBetweenStairs);
    }
}

void SMStairs::turnStairsFromDown()
{
    for (int8_t i = m_stairCount; i > 0; i--) { // Run through all stairs
        turnOnStair(i - 1);
        delay(SmartStairsConfig::delayBetweenStairs);
    }

    delay (SmartStairsConfig::delayBetweenOnOff);

    for (int8_t i = m_stairCount; i > 0; i--) { // Run through all stairs
        turnOffStair(i - 1);
        delay(SmartStairsConfig::delayBetweenStairs);
    }
}

void SMStairs::turnOnStair(uint16_t stairIndex)
{
    //! TODO: differend logic for filing led
    if (m_currentStairEffect == SMStairLightEffect::CenterFill) {
        CRGB color =  m_nightActiveColor;

        if (m_lastLightStatus != LightStatus::night) {
            color = m_eveningActiveColor[stairIndex];
        }
        
        int resultSpeed = SmartStairsConfig::stairsLightSpeed / (m_maxLedInStair / 2.) - 3;

        int leftPointer = m_maxLedInStair / 2;
        int rightPointer = leftPointer + 1;

        if (m_maxLedInStair % 2) {
            leftPointer = m_maxLedInStair / 2 + 1;
            rightPointer = leftPointer;
        } 

        while (leftPointer > -1 || rightPointer < m_maxLedInStair) {
            setPixel(stairIndex, leftPointer, color);
            setPixel(stairIndex, rightPointer, color);
            FastLED.show();
            m_firstLastDirtyStatus = true;
            leftPointer--;
            rightPointer++;

            delay(resultSpeed);
        }
    }
}

void SMStairs::turnOffStair(uint16_t stairIndex)
{
    //! TODO: differend logic for filing led
    if (m_currentStairEffect == SMStairLightEffect::CenterFill) {
        CRGB color = {0, 0, 0};

        if (stairIndex == 0 || stairIndex == m_stairCount -1) {
            if (m_lastLightStatus == LightStatus::night) {
                color =  m_nightStandColor;
            } else {
                color = m_eveningStandColor;
            }
        }
        
        int resultSpeed = SmartStairsConfig::stairsFadeSpeed / (m_maxLedInStair / 2.) - 3;

        int leftPointer = 0;
        int rightPointer = m_maxLedInStair - 1;

        while (rightPointer - leftPointer >= -1) {
            
            setPixel(stairIndex, leftPointer, color);
            setPixel(stairIndex, rightPointer, color);

            FastLED.show();
            m_firstLastDirtyStatus = true;
            leftPointer++;
            rightPointer--;

            delay(resultSpeed);
        }
    }
}

LightStatus SMStairs::lightStatus()
{
    if (digitalRead(SmartStairsPin::lightNight)) {
        return LightStatus::night;
    }
    return LightStatus::day;
}

void SMStairs::clearLed()
{
    FastLED.clear(true);
    m_firstLastDirtyStatus = true;
}

void SMStairs::setPixel(uint16_t stair, uint16_t pos, CRGB color) {
    if ( pos >= m_maxLedInStair || m_stairsMatrix[stair][pos] == -1 ) {
        return;
    }

    m_leds[m_stairsMatrix[stair][pos]] = color;
}

SMStairSensorPosition SMStairs::distanceSensorsCheck()
{
    if (compareDistance(SmartStairsPin::topTrigger, SmartStairsPin::topEcho, SmartStairsConfig::upWidth)) {
        return SMStairSensorPosition::TopSensor;
    } else if (compareDistance(SmartStairsPin::bottomTrigger, SmartStairsPin::bottomEcho, SmartStairsConfig::downWidth)) {
        return SMStairSensorPosition::BottomSensor;
    };

    return SMStairSensorPosition::None;
}

bool SMStairs::compareDistance(uint16_t trigPin, uint16_t echoPin, uint16_t distance)
{
    // Prepare
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    // Start measure
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    int currentDistance = pulseIn(echoPin, HIGH, 5800) / 58;

    if (currentDistance == 0) {
        return 0;
    }

    return currentDistance < distance;
}