#ifndef AUDIO
#define AUDIO

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace ptlmuh006{

    template<typename S> class Audio{
        private:
            int sampleRate, bitCount, numChannels;
            std::string filename;
            std::vector<S> data;

        public:
            //TODO: add parameterised constructors too
            //defualt constructor
            Audio(int r, int b, int c): sampleRate(r), bitCount(b), numChannels(c){}

            //copy constructor
            Audio(const Audio& other){
               sampleRate = other.sampleRate;
               bitCount = other.bitCount;
               numChannels = other.numChannels;

               //TODO: make this a deep copy
               data = other.data;
            }

            //move constructor
            Audio(Audio&& other){
                sampleRate = other.sampleRate;
                bitCount = other.bitCount;
                numChannels = other.numChannels;

                //TODO: make this a deep copy
                data = other.data;

                other.data.resize(0);
            }

            //destructor
            ~Audio(){}

            //copy assignment
            Audio& operator=(const Audio& other){
                sampleRate = other.sampleRate;
                bitCount = other.bitCount;
                numChannels = other.numChannels;

               //copy over the data;
               data = other.data;

               return *this;
            }

            //move assignment
            Audio& operator=(Audio&& other){
                sampleRate = other.sampleRate;
                bitCount = other.bitCount;
                numChannels = other.numChannels;

                //move over the data
                data = other.data;

                other.data = nullptr;

                return *this;
            }
    };

}
#endif // AUDIO

