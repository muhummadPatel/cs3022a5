#include <cstdint>
#include <iostream>
#include <string>

#include <audio.h>

using namespace ptlmuh006;

int main()
{
    std::cout << "Running main" << std::endl;

    Audio<int16_t> a(44100, 16, 1);
    a.read("frogs18sec_44100_signed_16bit_mono.raw");

    Audio<int16_t> b(44100, 16, 1);
    b.read("frogs18sec_44100_signed_16bit_mono.raw");

    Audio<int16_t> c = a | b;
    c.save("aCATb.raw");

    std::pair<float, float> factor(0.5f, 0.0f);
    Audio<int16_t> f = a * factor;
    f.save("aFACThalf.raw");
}
