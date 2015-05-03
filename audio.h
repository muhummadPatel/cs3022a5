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

            void read(std::string filename){
                std::ifstream infile(filename, std::ios::in | std::ios::binary);

                if(!infile){
                    std::cout << "Error opening file!" << std::endl;
                    std::exit(1);
                }

                //get length of infile
                infile.seekg(0, infile.end);
                int infileLength = infile.tellg();
                infile.seekg(0, infile.beg);
                std::cout << "length of " << filename << ": " << infileLength << std::endl;

                int numSamples = infileLength / (sizeof(S) * numChannels);
                std::cout << "numSamples: " << numSamples << std::endl;

                data.resize(numSamples);
                for(int i = 0; i < numSamples; i++){
                    char buff[sizeof(S)];
                    infile.read(buff, sizeof(S));
                    data[i] = *(S *)(buff);
                }

//                for(int i = 0 ; i < 10; i++){
//                    std::cout << data[numSamples - (i+1)] << std::endl;
//                }

                infile.close();
            }

            void save(std::string filename){
                std::ofstream outfile(filename, std::ios::out | std::ios::binary);

                outfile.write((char*)(&(data[0])), sizeof(S) * data.size());

                outfile.close();
            }

    };

}
#endif // AUDIO

