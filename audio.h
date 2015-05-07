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
        
        public:
            //nested iterator class
            class iterator{
                //friend the audio class to allow for access to private members
                friend class Audio;
                
                private:
                    S* ptr;
                    iterator(S* p): ptr(p) {} //constructor (only called by Audio::begin())
                    
                public:
                    //copy construct is public
                    iterator( const iterator & rhs) : ptr(rhs.ptr) {}
                    
                    //copy assignment
                    iterator& operator=(const iterator& rhs){
                        ptr = rhs.ptr;
                    }
                    
                    //move assignment
                    iterator& operator=(iterator&& rhs){
                        ptr = rhs.ptr;
                        rhs.ptr = nullptr;
                    }
                    
                    //dereference
                    S& operator*(){
                        return *ptr;
                    }
                    
                    //prefix ++
                    const iterator& operator++(){
                        ptr += 1;
                        return *this;
                    }
                    
                    //prefix --
                    const iterator& operator--(){
                        ptr -= 1;
                        return this;
                    }
                    
                    //equality
                    bool operator==(const iterator& rhs){
                        return (ptr == rhs.ptr);
                    }
                    
                    //inequality
                    bool operator!=(const iterator& rhs){
                        return (ptr != rhs.ptr);
                    }
            };
            
            //iterator to start of sample data
            iterator begin() const{
                S* first = (S*)&(data[0]);
                return iterator(first);
            }
            
            //one past the last valid position
            iterator end() const{
                S* onePastLast = (S*)&(data[data.size()]);
                return iterator(onePastLast);
            }

        public:
            //nested Functor
            class normFunctor{
                public:
                    float normFact;
                    normFunctor(float desiredRMS, float currentRMS): normFact(desiredRMS / currentRMS){}
                    S operator()(S inputAmp){
                        float outAmp = inputAmp * normFact;

                        if(outAmp > std::numeric_limits<S>::max()){
                            outAmp = std::numeric_limits<S>::max();
                        }else if(outAmp < std::numeric_limits<S>::min()){
                            outAmp = std::numeric_limits<S>::min();
                        }

                        return outAmp;
                    }
            };

        public:
            //TODO: add parameterised constructors too
            //defualt constructor
            Audio(int r = 44100, int b = 16, int c = 1): 
                sampleRate(r),
                bitCount(b),
                numChannels(c){}
            
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
            Audio rangedAdd(std::pair<int, int> range1, Audio aud2, std::pair<int, int> range2) const{
                //TODO: check sample reanges given have same length
                //TODO: try to change this to use std::copy

                Audio aud1 = *this ^ range1;
                aud2 = aud2 ^ range2;
                Audio sum = aud1 + aud2;

                return sum;
            }

            //compute RMS transformation
            float computeRMS(){
                float sumOfSq = 0;
                int numSamples = 0;

                std::function<S> rmsLambda = [&numSamples](float sumOfSq, S x){ sumOfSq += x * x; numSamples++;};
                std::accumulate(data.begin(), data.end(), sumOfSq, rmsLambda);

                return std::sqrt(sumOfSq / numSamples);
            }

            //normalisation transformation
            Audio normalized(std::pair<float, float> requiredRMS) const{
                Audio norm = *this;
                float currentRMS = norm.computeRMS();

                norm.data.resize(0);
                std::transform(data.begin(), data.end(), std::back_inserter(norm.data), normFunctor(requiredRMS.first, currentRMS));
            }
    };
    
    template<typename S> class Audio<std::pair<S, S>>{
        private:
            int sampleRate, bitCount, numChannels;
            std::string filename;
            std::vector<std::pair<S, S>> data;
        
        public:
            //TODO: add parameterised constructors too
            //defualt constructor
            Audio(int r = 44100, int b = 16, int c = 2): sampleRate(r), bitCount(b), numChannels(c){}
            
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
                    char lBuff[sizeof(S)];
                    infile.read(lBuff, sizeof(S));
                    data[i].first = *(S *)(lBuff);
                    
                    char rBuff[sizeof(S)];
                    infile.read(rBuff, sizeof(S));
                    data[i].second = *(S *)(rBuff);
                }

//                for(int i = 0 ; i < 10; i++){
//                    std::cout << data[numSamples - (i+1)] << std::endl;
//                }

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
    };
}
#endif // AUDIO

