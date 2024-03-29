namespace SmartStairsPin {
    // Ultra sonic sensors pin
    const int topEcho = 32;
    const int bottomEcho = 33;
    const int topTrigger = 25;
    const int bottomTrigger = 26;
    // Led strip pin
    const int ledStrip = 13;
    // Light sensors pin
    const int lightNight = 14;
}

namespace SmartStairsConfig {
    const int delayBetweenStairs = 0;
    const int stairsLightSpeed = 300;
    const int stairsFadeSpeed = 300;
    const int delayBetweenOnOff = 4000;

    const int upWidth = 60;
    const int downWidth = 70; 

    const int effectTimeToChange = 1000 * 60 * 60;
}

enum LightStatus {
    day,
    evening,
    night
};