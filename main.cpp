#include <cstdint>
#include <iostream>
#include <string>

#include "audio.h"

//TODO: FIX RANGED ADD TO USE SECONDS

void printUsage(){
    using namespace std;
    
    cout << "\nUsage:" << endl;
    cout << "./samp -r sampleRateInHz -b bitcount -c noChannels" << endl;
    cout << "\t [-o outfilename] [<ops>]" << endl;
    cout << "\t sndfile1 [sndfile2]" << endl;
    cout << "\nOptions:___" << endl;
    cout << "\t-add: add sndFile1 and sndFile2" << endl;
    cout << "\t-cut r1 r2: remove samples over range [r1,r2] (inclusive)" << endl;
    cout << "\t-radd r1 r2 s1 s2 : add sndFile1 and sndFile2 over sub-ranges(seconds)" << endl;
    cout << "\t-cat: concatenate soundFile1 and soundFile2" << endl;
    cout << "\t-v r1 r2: volume factor for left/right audio" << endl;
    cout << "\t-rev: reverse sound file" << endl;
    cout << "\t-rms: Prints out the RMS of the soundfile" << endl;
    cout << "\t-norm r1 r2: normalize file for left/right audio" << endl;
    cout << "\n"<< endl;
}

template <typename T> int parseOptions(int sampleRate, int bitCount, int noChannels, int argc, char* argv[], std::string outFilename){
    using namespace ptlmuh006;    
    
    int pos = (outFilename == "out.raw")? 7 : 9;
    std::cout << "pos" << pos << std::endl;
    
    if(std::string(argv[pos]) == "-add"){
        if(argc < (pos+2)+1) return 1;
        
        Audio<T> aud1(argv[pos+1], sampleRate, bitCount, noChannels);
        Audio<T> aud2(argv[pos+2], sampleRate, bitCount, noChannels);
        Audio<T> sum = aud1 + aud2;
        sum.save(outFilename);
        
        return 0;
    }else if(std::string(argv[pos]) == "-cut"){
        if(argc < (pos+3)+1) return 1;
        
        Audio<T> original(argv[pos+3], sampleRate, bitCount, noChannels);
        int r1 = std::stoi(argv[pos+1]);
        int r2 = std::stoi(argv[pos+2]);
        Audio<T> cut = original ^ std::pair<int, int>(r1, r2);
        cut.save(outFilename);
        
        return 0;
    }
}

int main(int argc, char* argv[]){
    using namespace ptlmuh006;
    
    if(argc < 8){
        printUsage();
        return 1;
    }else{
        int sampleRate = std::stoi(argv[2]);
        int bitCount = std::stoi(argv[4]);
        int noChannels = std::stoi(argv[6]);
        
        if(bitCount != 8 && bitCount != 16){
            std::cout << "Invalid bitCount (must be either 8 or 16)" << std::endl;
            return 1;
        }
        
        if(noChannels != 1 && noChannels != 2){
            std::cout << "Invalid noChannels (must be either 1 or 2)" << std::endl;
            return 1;
        }
        
        std::string outFilename = "out.raw";
        if(std::string(argv[7]) == "-o"){
            outFilename = std::string(argv[8]);
        }
        
        int response = 0;
        std::cout << argc << std::endl;
        
        if(noChannels == 1){
            if(bitCount == 8){
                response = parseOptions<int8_t>(sampleRate, bitCount, noChannels, argc, argv, outFilename);
            }else if(bitCount == 16){
                response = parseOptions<int16_t>(sampleRate, bitCount, noChannels, argc, argv, outFilename);
            }
        }else if(noChannels == 2){
            if(bitCount == 8){
                response = parseOptions<std::pair<int8_t, int8_t>>(sampleRate, bitCount, noChannels, argc, argv, outFilename);
            }else if(bitCount == 16){
                response = parseOptions<std::pair<int16_t, int16_t>>(sampleRate, bitCount, noChannels, argc, argv, outFilename);
            }
        }
        
        if(response != 0){
            std::cout << "ERRRORRR" << std::endl;
        }
        
        std::cout << sampleRate << " " << bitCount << " " << noChannels << std::endl;
    }
    
    return 0;
}
