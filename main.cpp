#include <cstdint>
#include <iostream>
#include <string>

#include "audio.h"

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
        
        std::cout << "Files successfully added and saved to " << outFilename << std::endl;
        
        return 0;
    }else if(std::string(argv[pos]) == "-cut"){
        if(argc < (pos+3)+1) return 1;
        
        Audio<T> original(argv[pos+3], sampleRate, bitCount, noChannels);
        int r1 = std::stoi(argv[pos+1]);
        int r2 = std::stoi(argv[pos+2]);
        Audio<T> cut = original ^ std::pair<int, int>(r1, r2);
        cut.save(outFilename);
        
        std::cout << "File successfully cut and saved to " << outFilename << std::endl;
        
        return 0;
    }else if(std::string(argv[pos]) == "-radd"){
        if(argc < (pos+6)+1) return 1;
        
        Audio<T> aud1(argv[pos+5], sampleRate, bitCount, noChannels);
        Audio<T> aud2(argv[pos+6], sampleRate, bitCount, noChannels);
        int r1Start = (int)(std::stof(argv[pos+1]) * bitCount);
        int r1End = (int)(std::stof(argv[pos+2]) * bitCount);
        int r2Start = (int)(std::stof(argv[pos+3]) * bitCount);
        int r2End = (int)(std::stof(argv[pos+4]) * bitCount);
        std::pair<int, int> r1(r1Start, r1End);
        std::pair<int, int> r2(r2Start, r2End);
        Audio<T> radd = Audio<T>::rangedAdd(aud1, r1, aud2, r2);
        radd.save(outFilename);
        
        std::cout << "File successfully added over range and saved to " << outFilename << std::endl;
        
        return 0;
    }else if(std::string(argv[pos]) == "-cat"){
        if(argc < (pos+2)+1) return 1;
        
        Audio<T> aud1(argv[pos+1], sampleRate, bitCount, noChannels);
        Audio<T> aud2(argv[pos+2], sampleRate, bitCount, noChannels);
        Audio<T> cat = aud1 | aud2;
        cat.save(outFilename);
        
        std::cout << "Files successfully concatenated and saved to " << outFilename << std::endl;
        
        return 0;
    }else if(std::string(argv[pos]) == "-v"){
        if(argc < (pos+3)+1) return 1;
        
        Audio<T> aud(argv[pos+3], sampleRate, bitCount, noChannels);
        std::pair<float, float> fact(std::stof(argv[pos+1]), std::stof(argv[pos+2]));
        Audio<T> factored = aud * fact;
        factored.save(outFilename);
        
        std::cout << "File successfully volume factored and saved to " << outFilename << std::endl;
        
        return 0;
    }else if(std::string(argv[pos]) == "-rev"){
        if(argc < (pos+1)+1) return 1;
        
        Audio<T> aud(argv[pos+1], sampleRate, bitCount, noChannels);
        Audio<T> rev = aud.reverse();
        rev.save(outFilename);
        
        std::cout << "File successfully reversed and stored to " << outFilename << std::endl;
        
        return 0;
    }else if(std::string(argv[pos]) == "-rms"){ 
        if(argc < (pos+1)+1) return 1;
        
        Audio<T> aud(argv[pos+1], sampleRate, bitCount, noChannels);
        std::pair<float, float> rms = aud.computeRMS();
        if(noChannels == 1){
            std::cout << "RMS: " << rms.first << std::endl;
        }else{
            std::cout << "Left channel RMS: " << rms.first << std::endl;
            std::cout << "Right channel RMS: " << rms.second << std::endl;
        }
        
        return 0;
    }else if(std::string(argv[pos]) == "-norm"){
        if(argc < (pos+3)+1) return 1;
        
        Audio<T> aud(argv[pos+3], sampleRate, bitCount, noChannels);
        std::pair<float, float> reqRMS(std::stof(argv[pos+1]), std::stof(argv[pos+2]));
        Audio<T> norm = aud.normalized(reqRMS);
        norm.save(outFilename);
        
        std::cout << "File successfully normalized and stored to " << outFilename << std::endl;
        
        return 0;
    }else{
        return 1;
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
            std::cout << "Invalid usage." << std::endl;
            printUsage();
            return 1;
        }
    }
    
    return 0;
}
