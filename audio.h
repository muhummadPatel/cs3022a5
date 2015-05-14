/*
 * This header file contains the definition of the templated Audio class.
 * The Audio class has been temnplated to allow for .raw audio files that have
 * different sample sizes. The Audio class has a single template parameter S.
 * The Audio class can handle both mono and stereo files. Stereo files are 
 * handled by a partial specialisation (for the case where the parameter given 
 * is a std::pair<S, S>). The Audio files can be manipulated through operators 
 * and transformations that have been implemented as requested in the assignment
 * specification.
 *
 * Muhummad Patel
 * 14-May-2015
 */

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

    //Templated Audio class. The code implemented here will handle the case of
    //mono audio files.
    template<typename S> class Audio{
        private:
            int sampleRate, bitCount, numChannels;
            std::string filename;
            std::vector<S> data;

        private:
            //nested Functor to aid in normalizing the audio file
            class normFunctor{
                public:
                    double normFact;
                    
                    normFunctor(float desiredRMS, float currentRMS): normFact(desiredRMS / currentRMS){}
                    
                    //overloaded ()
                    S operator()(S inputAmp){
                        double outAmp = inputAmp * normFact;

                        //clamp the output amplitude to the max of the type S
                        if(outAmp > std::numeric_limits<S>::max()){
                            outAmp = std::numeric_limits<S>::max();
                        }else if(outAmp < std::numeric_limits<S>::min()){
                            outAmp = std::numeric_limits<S>::min();
                        }

                        return (S)outAmp;
                    }
            };

        public:
            //constructor
            Audio(int r = 44100, int b = 16, int c = 1): 
                sampleRate(r),
                bitCount(b),
                numChannels(c){}
            
            //useful constructor to construct + read in file
            Audio(std::string infilename, int r = 44100, int b = 16, int c = 1):
                sampleRate(r),
                bitCount(b),
                numChannels(c){
                    read(infilename);
                }
            
            //constructor to be used for testing purposes. Allows for 
            //construction using a buffer
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

               data = other.data;
            }

            //move constructor
            Audio(Audio&& other){
                sampleRate = other.sampleRate;
                bitCount = other.bitCount;
                numChannels = other.numChannels;

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
            
            //basic accessor functions to be used in testing
            int getSampleRate(){ return sampleRate; }
            int getBitCount(){ return bitCount; }
            int getNumChannels(){ return numChannels; }
            std::vector<S> getData(){ return data; }

            //function to read in a raw audio file
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

                //reserve space for all the samples and read them in
                data.resize(numSamples);
                for(int i = 0; i < numSamples; i++){
                    char buff[sizeof(S)];
                    infile.read(buff, sizeof(S));
                    data[i] = *(S *)(buff);
                }
                
                infile.close();
            }

            //save the samples in this objects buffer to a .raw audio file
            void save(std::string filename){
                std::ofstream outfile(filename, std::ios::out | std::ios::binary);
                outfile.write((char*)(&(data[0])), sizeof(S) * data.size());
                outfile.close();
            }

            //concatenation operator
            Audio<S> operator|(const Audio& rhs) const{
                Audio<S> cat = *this;
                
                //add all the rhs samples to the end of this data buffer
                for(std::size_t i = 0; i < rhs.data.size(); i++){
                    cat.data.push_back(rhs.data[i]);
                }
                return cat;
            }

            //volume factor operator
            Audio operator*(const std::pair<float, float> factor) const{
                float monoFact = factor.first;
                Audio factored = *this;
                
                //multiply every sample by the factor
                for(std::size_t i = 0; i < data.size(); i++){
                    factored.data[i] *= monoFact;
                }

                return factored;
            }
            
            //per sample add operator
            Audio operator+(const Audio& rhs) const{
                //Use the longer file as the base to add to
                Audio sum = *this;
                Audio other = rhs;
                if(this->data.size() < rhs.data.size()){
                    sum = rhs;
                    other = *this;
                }
                
                //add each sample together
                for(std::size_t i = 0; i < other.data.size(); i++){
                    double testSum = sum.data[i] + other.data[i];
                    
                    //clamp to the range of type S
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
                std::vector<S> buffer;

                //discard all samples outside the required range for aud1
                auto startIt = aud1.data.begin() + (range1.first - 1);
                auto endIt = aud1.data.begin() + range1.second;
                std::copy(startIt, endIt, std::back_inserter(buffer));
                aud1.data = std::move(buffer);

                //discard all samples outside the reuired range for aud2
                startIt = aud2.data.begin() + (range2.first - 1);
                endIt = aud2.data.begin() + range2.second;
                std::copy(startIt, endIt, std::back_inserter(buffer));
                aud2.data = std::move(buffer);
                buffer.resize(0);

                //add the reduced clips and return
                Audio sum = aud1 + aud2;

                return sum;
            }

            //compute RMS transformation
            std::pair<float, float> computeRMS(){
                float sumOfSq = 0;
                int numSamples = 0;

                sumOfSq = std::accumulate(data.begin(), data.end(), sumOfSq,
                    [&numSamples](float sumOfSq, S x){ 
                        numSamples++;
                        return sumOfSq + (x * x);
                    }
                );

                std::pair<float, float> rms(std::sqrt(sumOfSq / numSamples), 0.0f);
                return rms;
            }

            //normalisation transformation
            Audio normalized(std::pair<float, float> requiredRMS) const{
                Audio norm = *this;
                float currentRMS = norm.computeRMS().first;
                norm.data.resize(0);
                
                //Use the normFunctor to normalize the data
                std::transform(data.begin(), data.end(), std::back_inserter(norm.data), normFunctor(requiredRMS.first, currentRMS));

                return norm;
            }
    };
    
    //Templated Audio class. The code implemented here will handle the case of
    //stereo audio files.
    template<typename S> class Audio <std::pair<S, S>> {
        private:
            int sampleRate, bitCount, numChannels;
            std::string filename;
            std::vector<std::pair<S, S>> data;
        
        private:
            //nested Functor to normalise stereo files
            class normFunctor{
                public:
                    std::pair<double, double> normFact;
                    
                    normFunctor(std::pair<float, float> desiredRMS, std::pair<float, float> currentRMS): 
                        normFact(std::pair<float, float>(desiredRMS.first / currentRMS.first, desiredRMS.second / currentRMS.second)){}
                        
                    std::pair<S, S> operator()(std::pair<S, S> inputAmp){
                        //Compute and clamp the normalised left channel
                        double outAmpL = inputAmp.first * normFact.first;
                        if(outAmpL > std::numeric_limits<S>::max()){
                            outAmpL = std::numeric_limits<S>::max();
                        }else if(outAmpL < std::numeric_limits<S>::min()){
                            outAmpL = std::numeric_limits<S>::min();
                        }
                        
                        //compute and clamp the normalised right channel
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
            //constructor
            Audio(int r = 44100, int b = 16, int c = 2): 
                sampleRate(r),
                bitCount(b),
                numChannels(c){}
            
            //more useful constructor that also reads in a file
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

               data = other.data;
            }

            //move constructor
            Audio(Audio&& other){
                sampleRate = other.sampleRate;
                bitCount = other.bitCount;
                numChannels = other.numChannels;

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
            
            //Reads in the samples from the specified raw audio file
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
                
                //reserve space for the samples and read them in
                data.resize(numSamples);
                for(int i = 0; i < numSamples; i++){
                    //read in left channel
                    char lBuff[sizeof(S)];
                    infile.read(lBuff, sizeof(S));
                    data[i].first = *(S *)(lBuff);
                    
                    //read in the right channel
                    char rBuff[sizeof(S)];
                    infile.read(rBuff, sizeof(S));
                    data[i].second = *(S *)(rBuff);
                }

                infile.close();
            }

            //Writes the data from the buffer to the file with the sepcified filename.
            void save(std::string filename){
                std::ofstream outfile(filename, std::ios::out | std::ios::binary);

                //write out each set of left and right samples one at a time
                int index = 0;
                std::for_each(data.begin(), data.end(), [&outfile, &index](std::pair<S, S> sample){ 
                    outfile.write((char*)(&(sample.first)), sizeof(S));
                    outfile.write((char*)(&(sample.second)), sizeof(S));
                    index++;
                });

                outfile.close();
            }
            
            //accessor functions used for testing purposes
            int getSampleRate(){ return sampleRate; }
            int getBitCount(){ return bitCount; }
            int getNumChannels(){ return numChannels; }
            std::vector<std::pair<S, S>> getData(){ return data; }
            
            //concatenation operator
            Audio operator|(const Audio& rhs) const{
                Audio cat = *this;
                for(std::size_t i = 0; i < rhs.data.size(); i++){
                    cat.data.push_back(rhs.data[i]);
                }

                return cat;
            }
            
            //volume factor operator
            Audio operator*(const std::pair<float, float> factor) const{
            
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
                    //Add and clamp the left channel sample
                    double testSum1 = sum.data[i].first + other.data[i].first;
                    if(testSum1 > std::numeric_limits<S>::max()){
                        testSum1 = std::numeric_limits<S>::max();
                    }else if (testSum1 < std::numeric_limits<S>::min()){
                        testSum1 = std::numeric_limits<S>::min();
                    }
                    sum.data[i].first = (S)testSum1;
                    
                    //add and clamp the right channel sample
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
                std::vector<std::pair<S, S>> buffer;

                //resize Aud1 to the requested range discarding irrelevant data
                auto startIt = aud1.data.begin() + (range1.first - 1);
                auto endIt = aud1.data.begin() + range1.second;
                std::copy(startIt, endIt, std::back_inserter(buffer));
                aud1.data = std::move(buffer);
                buffer.resize(0);

                //resize Aud2 to the requested range discarding irrelevant data
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
                //compute the sum of the squares of the right channel samples
                float sumOfSqL = 0;
                int numSamples = 0;
                sumOfSqL = std::accumulate(data.begin(), data.end(), sumOfSqL, 
                    [&numSamples](float sumOfSqL, std::pair<S, S> x){ 
                        numSamples++;
                        return sumOfSqL + (x.first * x.first);
                    }
                );
                
                //compute the sum of the squares of the right channel samples
                float sumOfSqR = 0;
                numSamples = 0;
                sumOfSqR = std::accumulate(data.begin(), data.end(), sumOfSqR, 
                    [&numSamples](float sumOfSqR, std::pair<S, S> x){ 
                        numSamples++;
                        return sumOfSqR + (x.first * x.first);
                    }
                );
                
                //compute the right and left channel RMS's
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

