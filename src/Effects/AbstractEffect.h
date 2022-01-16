#ifndef ABSTRACTEFFECT
#define ABSTRACTEFFECT
class AbstractEffect
{
public:
    virtual void fillLedWithProxy(const std::vector<std::vector<uint16_t>> &m_stairsMatrix,
                                  CRGB *leds,
                                  uint16_t n, 
                                  uint16_t from = 1) = 0;

    virtual uint16_t timeBetweenCalls() = 0;

    virtual void refresh() {
        
    }
};
#endif
