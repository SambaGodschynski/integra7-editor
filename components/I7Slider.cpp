#include "I7Slider.h"

I7Slider::I7Slider()
{

}

void I7Slider::i7setControlLimits(i7::UInt min, i7::UInt max)
{
    setRange(min, max, 1);
}

void I7Slider::i7setValue(ControlerValueType v)
{
    setValue(v);
}

void I7Slider::valueChanged()
{
    i7onValueChanged((i7::UInt)getValue());
}