#include "sm.stairs.h"

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

void SMStairs::init(std::vector<int> ledByStairs)
{
    m_stairCount = ledByStairs.size();
    m_maxLedInStair = *std::max_element(ledByStairs.begin(), ledByStairs.end());

    int ledCount = 1;
    for (int i = 0; i < m_stairCount; i++) {
        m_stairsMatrix.push_back(std::vector<int>(m_maxLedInStair, -1));
        ledCount += ledByStairs[i];
    }

    pinMode(SmartStairsPin::lightFull, INPUT);
    pinMode(SmartStairsPin::lightNight, INPUT);
    pinMode(SmartStairsPin::topTrigger, OUTPUT);
    pinMode(SmartStairsPin::bottomTrigger, OUTPUT);
    pinMode(SmartStairsPin::topEcho, INPUT);
    pinMode(SmartStairsPin::bottomEcho, INPUT);

    m_leds = new CRGB[ledCount];
    FastLED.addLeds<WS2811, SmartStairsPin::ledStrip, BRG>(m_leds, ledCount)
                                .setCorrection(TypicalLEDStrip);

    clearLed();

    int shift = 1;
    int middleElement = static_cast<int>(m_maxLedInStair / 2. + 0.5);

    double modificator = m_maxLedInStair % 2 ? 1. : 0.5;

    for (int i = 0; i < m_stairCount; i++) {
        const int pixelsInRow = ledByStairs[i];

        int counter = 0;

        int shiftArray[m_maxLedInStair];

        for(int j = 0; j < pixelsInRow; j++, shift++) {
            shiftArray[counter] = shift;
            counter++;
        }

        int curentStairMiddleElement = static_cast<int>(pixelsInRow / 2. + modificator);

        int startPosition = middleElement - curentStairMiddleElement;

        for (int j = startPosition, k = 0; k < counter; j++, k++) {
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
    if (millis() - m_lastLightChangeTime > 1000) {
        m_lastLightStatus = lightStatus();
        setFirstLast();
        m_lastLightChangeTime = millis();
    }


    if (m_lastLightStatus == LightStatus::day) {
        return;
    }

    SMStairSensorPosition position = distanceSensorsCheck();

    if (position == SMStairSensorPosition::None) {
        return; // Go to next check
    }

    if (position == SMStairSensorPosition::BottomSensor) {
        turnStairsFromDown();
        return;
    }

    turnStairsFromUp();
}
 
void SMStairs::setFirstLast()
{
    if (m_lastLightStatus == LightStatus::day) {
        clearLed();
    } else if (m_lastLightStatus == LightStatus::evening) {
        for (int i = 0; i < m_maxLedInStair; i++) {
            setPixel(0, i, m_eveningStandColor);
            setPixel(m_stairCount -1, i, m_eveningStandColor);
        }
    } else {
        for (int i = 0; i < m_maxLedInStair; i++) {
            setPixel(0, i, m_nightStandColor);
            setPixel(m_stairCount -1, i, m_nightStandColor);
        }
    }
    FastLED.show();
}

void SMStairs::turnStairsFromUp()
{
    for (int i = 0; i < m_stairCount; i++) { // Run through all stairs
        turnOnStair(i);
        delay(SmartStairsConfig::delayBetweenStairs);
    }

    delay (SmartStairsConfig::delayBetweenOnOff);

    for (int i = 0; i < m_stairCount; i++) { // Run through all stairs
        turnOffStair(i);
        delay(SmartStairsConfig::delayBetweenStairs);
    }
}

void SMStairs::turnStairsFromDown()
{
    for (int i = m_stairCount; i > 0; i--) { // Run through all stairs
        turnOnStair(i - 1);
        delay(SmartStairsConfig::delayBetweenStairs);
    }

    delay (SmartStairsConfig::delayBetweenOnOff);

    for (int i = m_stairCount; i > 0; i--) { // Run through all stairs
        turnOffStair(i - 1);
        delay(SmartStairsConfig::delayBetweenStairs);
    }
}

void SMStairs::turnOnStair(int stairIndex)
{
    if (m_currentStairEffect == SMStairLightEffect::CenterFill) {
        CRGB color =  m_nightActiveColor;

        if (m_lastLightStatus == LightStatus::evening) {
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
            leftPointer--;
            rightPointer++;

            delay(resultSpeed);
        }
    }
}

void SMStairs::turnOffStair(int stairIndex)
{
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
            leftPointer++;
            rightPointer--;

            delay(resultSpeed);
        }
    }
}

LightStatus SMStairs::lightStatus()
{
    bool full = digitalRead(SmartStairsPin::lightFull);;
    if (full && digitalRead(SmartStairsPin::lightNight)) {
        return LightStatus::night;
    }
  
    if (full) {
        return LightStatus::evening;
    }
  
    return LightStatus::day;
}

void SMStairs::clearLed()
{
    FastLED.clear(true);
}

void SMStairs::setPixel(int stair, int pos, CRGB color) {
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

bool SMStairs::compareDistance(int trigPin, int echoPin, int distance)
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