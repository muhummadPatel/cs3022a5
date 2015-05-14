#ifndef AUDIO
#define AUDIO

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <vector>

namespace ptlmuh006{

    template<typename S> class Audio{
        private:
            int sampleRate, bitCount, numChannels;
            std::string filename;
            std::vector<S> data;

        private:
            //nested Functor
            class normFunctor{
                public:
                    double normFact;
                    normFunctor(float desiredRMS, float currentRMS): normFact(desiredRMS / currentRMS){}
                    
                    S operator()(S inputAmp){
                        double outAmp = inputAmp * normFact;

                        if(outAmp > std::numeric_limits<S>::max()){
                            outAmp = std::numeric_limits<S>::max();
                        }else if(outAmp < std::numeric_limits<S>::min()){
                            outAmp = std::numeric_limits<S>::min();
                        }

                        return (S)outAmp;
                    }
            };

        public:
            //TODO: add parameterised constructors too
            //defualt constructor
            Audio(int r = 44100, int b = 16, int c = 1): 
                sampleRate(r),
                bitCount(b),
                numChannels(c){}
            
            Audio(std::string infilename, int r = 44100, int b = 16, int c = 2):
                sampleRate(r),
                bitCount(b),
                numChannels(c){
                    read(infilename);
                }
            
            //constructor to be used for testing purposes
            Audio(int r, int b, int c, std::vector<S> d):
                sampleRate(r),
                bitCount(b),
                numChannels(c),
                data(d){}

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
                data = std::move(other.data);

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
                data = std::move(other.data);

                other.data.resize(0);

                return *this;
            }
            
            int getSampleRate(){ return sampleRate; }
            int getBitCount(){ return bitCount; }
            int getNumChannels(){ return numChannels; }
            std::vector<S> getData(){ return data; }

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

                int numSamples = infileLength / (sizeof(S) * numChannels);

                data.resize(numSamples);
                for(int i = 0; i < numSamples; i++){
                    char buff[sizeof(S)];
                    infile.read(buff, sizeof(S));
                    data[i] = *(S *)(buff);
                }
                
                infile.close();
            }

            void save(std::string filename){
                std::ofstream outfile(filename, std::ios::out | std::ios::binary);

                outfile.write((char*)(&(data[0])), sizeof(S) * data.size());

                outfile.close();
            }

            //concatenation operator
            Audio<S> operator|(const Audio& rhs) const{
                //TODO: check that the files are compatible before catting them

                Audio<S> cat = *this;
                for(std::size_t i = 0; i < rhs.data.size(); i++){
                    cat.data.push_back(rhs.data[i]);
                }

                return cat;
            }

            //volume factor operator
            Audio operator*(const std::pair<float, float> factor) const{
                //TODO: check the factors given are in range [0.0f, 1.0f]
                float monoFact = factor.first;

                Audio factored = *this;
                for(std::size_t i = 0; i < data.size(); i++){
                    factored.data[i] *= monoFact;
                }

                return factored;
            }
            
            //per sample add operator
            Audio operator+(const Audio& rhs) const{
                Audio sum = *this;
                Audio other = rhs;
                if(this->data.size() < rhs.data.size()){
                    sum = rhs;
                    other = *this;
                }
                
                for(std::size_t i = 0; i < other.data.size(); i++){
                    
                    
                    double testSum = sum.data[i] + other.data[i];
                    if(testSum > std::numeric_limits<S>::max()){
                        testSum = std::numeric_limits<S>::max();
                    }else if (testSum < std::numeric_limits<S>::min()){
                        testSum = std::numeric_limits<S>::min();
                    }
                    
                    sum.data[i] = (S)testSum;
                }
                
                return sum;
            }
            
            //cut operator
            Audio operator^(const std::pair<int, int> range) const{
                Audio cut = *this;
                
                auto rangeStart = cut.data.begin() + (range.first - 1);
                auto rangeEnd = cut.data.begin() + (range.second);
                cut.data.erase(rangeStart, rangeEnd);
                
                return cut;
            }
            
            //reverse transformation
            Audio reverse() const{
                Audio rev = *this;
                
                std::reverse(rev.data.begin(), rev.data.end());
                
                return rev;
            }

            //ranged add transformation
            static Audio rangedAdd(Audio aud1, const std::pair<int, int> range1, Audio aud2, const std::pair<int, int> range2){
                //TODO: check sample reanges given have same length
                //TODO: try to change this to use std::copy

                std::vector<S> buffer;

                auto startIt = aud1.data.begin() + (range1.first - 1);
                auto endIt = aud1.data.begin() + range1.second;
                std::copy(startIt, endIt, std::back_inserter(buffer));
                aud1.data = std::move(buffer);

                startIt = aud2.data.begin() + (range2.first - 1);
                endIt = aud2.data.begin() + range2.second;
                std::copy(startIt, endIt, std::back_inserter(buffer));
                aud2.data = std::move(buffer);
                buffer.resize(0);

                Audio sum = aud1 + aud2;

                return sum;
            }

            //compute RMS transformation
            std::pair<float, float> computeRMS(){
                float sumOfSq = 0;
                int numSamples = 0;

                sumOfSq = std::accumulate(data.begin(), data.end(), sumOfSq, [&numSamples](float sumOfSq, S x){ numSamples++; return sumOfSq + (x * x);});

                std::pair<float, float> rms(std::sqrt(sumOfSq / numSamples), 0.0f);
                return rms;
            }

            //normalisation transformation
            Audio normalized(std::pair<float, float> requiredRMS) const{
                Audio norm = *this;
                float currentRMS = norm.computeRMS().first;

                norm.data.resize(0);
                std::transform(data.begin(), data.end(), std::back_inserter(norm.data), normFunctor(requiredRMS.first, currentRMS));

                return norm;
            }
    };
    
    template<typename S> class Audio<std::pair<S, S> >{
        private:
            int sampleRate, bitCount, numChannels;
            std::string filename;
            std::vector<std::pair<S, S>> data;
        
        private:
            //nested Functor
            class normFunctor{
                public:
                    std::pair<double, double> normFact;
                    normFunctor(std::pair<float, float> desiredRMS, std::pair<float, float> currentRMS): 
                        normFact(std::pair<float, float>(desiredRMS.first / currentRMS.first, desiredRMS.second / currentRMS.second)){}
                        
                    std::pair<S, S> operator()(std::pair<S, S> inputAmp){
                        double outAmpL = inputAmp.first * normFact.first;
                        if(outAmpL > std::numeric_limits<S>::max()){
                            outAmpL = std::numeric_limits<S>::max();
                        }else if(outAmpL < std::numeric_limits<S>::min()){
                            outAmpL = std::numeric_limits<S>::min();
                        }
                        
                        double outAmpR = inputAmp.second * normFact.second;
                        if(outAmpR > std::numeric_limits<S>::max()){
                            outAmpR = std::numeric_limits<S>::max();
                        }else if(outAmpR < std::numeric_limits<S>::min()){
                            outAmpR = std::numeric_limits<S>::min();
                        }
                        
                        return std::pair<S, S>((S)outAmpL, (S)outAmpR);
                    }
            };
        
        public:
            //TODO: add parameterised constructors too
            //defualt constructor
            Audio(int r = 44100, int b = 16, int c = 2): 
                sampleRate(r),
                bitCount(b),
                numChannels(c){}
            
            Audio(std::string infilename, int r = 44100, int b = 16, int c = 2): 
                sampleRate(r),
                bitCount(b),
                numChannels(c){
                    read(infilename);
                }
            
            //constructor to be used for testing purposes
            Audio(int r, int b, int c, std::vector<std::pair<S, S>> d):
                sampleRate(r),
                bitCount(b),
                numChannels(c),
                data(d){}

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
                data = std::move(other.data);

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
                data = std::move(other.data);

                other.data.resize(0);

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

                int numSamples = infileLength / (sizeof(S) * numChannels);

                data.resize(numSamples);
                for(int i = 0; i < numSamples; i++){
                    char lBuff[sizeof(S)];
                    infile.read(lBuff, sizeof(S));
                    data[i].first = *(S *)(lBuff);
                    
                    char rBuff[sizeof(S)];
                    infile.read(rBuff, sizeof(S));
                    data[i].second = *(S *)(rBuff);
                }

                infile.close();
            }

            void save(std::string filename){
                std::ofstream outfile(filename, std::ios::out | std::ios::binary);

                //outfile.write((char*)(&(data[0].first)), sizeof(S));
                int index = 0;
                std::for_each(data.begin(), data.end(), [&outfile, &index](std::pair<S, S> sample){ 
                    outfile.write((char*)(&(sample.first)), sizeof(S));
                    outfile.write((char*)(&(sample.second)), sizeof(S));
                    index++;
                });

                outfile.close();
            }
            
            int getSampleRate(){ return sampleRate; }
            int getBitCount(){ return bitCount; }
            int getNumChannels(){ return numChannels; }
            std::vector<std::pair<S, S>> getData(){ return data; }
            
            //concatenation operator
            Audio operator|(const Audio& rhs) const{
                //TODO: check that the files are compatible before catting them

                Audio cat = *this;
                for(std::size_t i = 0; i < rhs.data.size(); i++){
                    cat.data.push_back(rhs.data[i]);
                }

                return cat;
            }
            
            //volume factor operator
            Audio operator*(const std::pair<float, float> factor) const{
                //TODO: check the factors given are in range [0.0f, 1.0f]                

                Audio factored = *this;
                for(std::size_t i = 0; i < data.size(); i++){
                    factored.data[i].first *= factor.first;
                    factored.data[i].second *= factor.second;
                }

                return factored;
            }
            
            //per sample add operator
            Audio operator+(const Audio& rhs) const{
                Audio sum = *this;
                Audio other = rhs;
                if(this->data.size() < rhs.data.size()){
                    sum = rhs;
                    other = *this;
                }
                
                for(std::size_t i = 0; i < other.data.size(); i++){
                    
                    double testSum1 = sum.data[i].first + other.data[i].first;
                    if(testSum1 > std::numeric_limits<S>::max()){
                        testSum1 = std::numeric_limits<S>::max();
                    }else if (testSum1 < std::numeric_limits<S>::min()){
                        testSum1 = std::numeric_limits<S>::min();
                    }
                    sum.data[i].first = (S)testSum1;
                    
                    double testSum2 = sum.data[i].second + other.data[i].second;
                    if(testSum2 > std::numeric_limits<S>::max()){
                        testSum2 = std::numeric_limits<S>::max();
                    }else if (testSum2 < std::numeric_limits<S>::min()){
                        testSum2 = std::numeric_limits<S>::min();
                    }
                    sum.data[i].second = (S)testSum2;
                }
                
                return sum;
            }
            
            //TODO: Do cut and add and ranged add need to be here???
            //cut operator
            Audio operator^(const std::pair<int, int> range) const{
                Audio cut = *this;
                
                auto rangeStart = cut.data.begin() + (range.first - 1);
                auto rangeEnd = cut.data.begin() + (range.second);
                cut.data.erase(rangeStart, rangeEnd);
                
                return cut;
            }
            
            //reverse transformation
            Audio reverse() const{
                Audio rev = *this;
                
                std::reverse(rev.data.begin(), rev.data.end());
                
                return rev;
            }

            //ranged add transformation
            static Audio rangedAdd(Audio aud1, const std::pair<int, int> range1, Audio aud2, const std::pair<int, int> range2){
                //TODO: check sample reanges given have same length
                //TODO: try to change this to use std::copy

                std::vector<std::pair<S, S>> buffer;

                auto startIt = aud1.data.begin() + (range1.first - 1);
                auto endIt = aud1.data.begin() + range1.second;
                std::copy(startIt, endIt, std::back_inserter(buffer));
                aud1.data = std::move(buffer);
                buffer.resize(0);

                startIt = aud2.data.begin() + (range2.first - 1);
                endIt = aud2.data.begin() + range2.second;
                std::copy(startIt, endIt, std::back_inserter(buffer));
                aud2.data = std::move(buffer);
                buffer.resize(0);

                Audio sum = aud1 + aud2;

                return sum;
            }

            //compute RMS transformation
            std::pair<float, float> computeRMS(){
                float sumOfSqL = 0;
                int numSamples = 0;

                sumOfSqL = std::accumulate(data.begin(), data.end(), sumOfSqL, 
                    [&numSamples](float sumOfSqL, std::pair<S, S> x){ 
                        numSamples++;
                        return sumOfSqL + (x.first * x.first);
                    }
                );
                
                float sumOfSqR = 0;
                numSamples = 0;

                sumOfSqR = std::accumulate(data.begin(), data.end(), sumOfSqR, 
                    [&numSamples](float sumOfSqR, std::pair<S, S> x){ 
                        numSamples++;
                        return sumOfSqR + (x.first * x.first);
                    }
                );
                
                float RMSLeft = std::sqrt(sumOfSqL / numSamples);
                float RMSRight = std::sqrt(sumOfSqR / numSamples);
                
                return std::pair<float, float>(RMSLeft, RMSRight);
            }
            
            //normalisation transformation
            Audio normalized(std::pair<float, float> requiredRMS) const{
                Audio norm = *this;
                std::pair<float, float> currentRMS = norm.computeRMS();

                norm.data.resize(0);
                std::transform(data.begin(), data.end(), std::back_inserter(norm.data), normFunctor(requiredRMS, currentRMS));

                return norm;
            }
    };
}
#endif // AUDIO

