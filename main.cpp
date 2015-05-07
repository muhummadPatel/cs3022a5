#include <cstdint>
#include <iostream>
#include <string>

#include "audio.h"

using namespace ptlmuh006;

int main()
{
    std::cout << "Running main" << std::endl;

    Audio<std::pair<int16_t, int16_t>> a(44100, 16, 2);
    a.read("frogs18sec_44100_signed_16bit_stereo.raw");
    a.save("STEEEERRRRREEOOOO.raw");
    
//    for(auto it = y.begin(); it != y.end(); ++it){
//        std::cout << *it << std::endl;
//    }
}
