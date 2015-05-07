#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <cstdint>
#include <vector>

#include "audio.h"

using namespace ptlmuh006;

TEST_CASE("Test Mono Operator Overloads", ""){
    
    int sampleRate = 44100,
        bitCount = 16,
        channels = 1;
        
    std::vector<int16_t> samplesA = {-1, 1, 2, -2, 3, -3, 10, -10};
    Audio<int16_t> a(sampleRate, bitCount, channels, samplesA);
    
    SECTION("Test concatenate operator a|b"){
        //GIVEN: some audio objects a and b
        std::vector<int16_t> samplesB = {-1, 1, 2, -2, 3, -3, 10, -10};
        Audio<int16_t> b(sampleRate, bitCount, channels, samplesB);
        
        //WHEN: we concatenate them using the concatenation operator
        Audio<int16_t> concatenated = a | b;
        
        //THEN: the result should be the two files back to back
        std::vector<int16_t> expected = {-1, 1, 2, -2, 3, -3, 10, -10, -1, 1, 2, -2, 3, -3, 10, -10};
        REQUIRE( concatenated.getData().size() == expected.size() );
        REQUIRE( concatenated.getData() == expected );
    }
    
    SECTION("Test volume factor operator a*f"){
        //GIVEN: some audio object a        
        //WHEN: we apply the volume factor operator with 0.5
        Audio<int16_t> factored = a * std::pair<float, float>(0.5f, 0.5f);
        
        //THEN: the result should have all samples halved and clamped
        std::vector<int16_t> expected = {0, 0, 1, -1, 1, -1, 5, -5};
        REQUIRE( factored.getData().size() == expected.size() );
        REQUIRE( factored.getData() == expected );
    }
    //32767
    SECTION("Test addition operator a+b"){
        //GIVEN: some audio objects a and b
        std::vector<int16_t> samplesB = {-1, 1, 2, -2, 3, -3, std::numeric_limits<int16_t>::max(), std::numeric_limits<int16_t>::min()};
        Audio<int16_t> b(sampleRate, bitCount, channels, samplesB);
        
        //WHEN: we add a and b
        Audio<int16_t> sum = a + b;
        
        //THEN: the result should have all samples added together and clamped
        std::vector<int16_t> expected = {-2, 2, 4, -4, 6, -6, std::numeric_limits<int16_t>::max(), std::numeric_limits<int16_t>::min()};
        //int16_t yo = 32767 + 5;
        //REQUIRE( yo == 0 );
        REQUIRE( sum.getData().size() == expected.size() );
        REQUIRE( sum.getData() == expected );
    }

    SECTION("Test cut operator a^f"){
        //GIVEN: some audio object a
        //WHEN: we apply the cut operator with (2, 5)
        Audio<int16_t> cut = a ^ std::pair<int, int>(2, 5);

        //THEN: the result should be a copy of a without the samples in the specified range (inclusive)
        std::vector<int16_t> expected = {-1, -3, 10, -10};
        REQUIRE( cut.getData().size() == expected.size() );
        REQUIRE( cut.getData() == expected );
    }
}
}

TEST_CASE("Test Mono Move/Copy Semantics", ""){
    
    int sampleRate = 44100,
        bitCount = 16,
        channels = 1;
    std::vector<int16_t> samples = {-1, 1, 2, -2, 3, -3, 10, -10};
    
    Audio<int16_t> aud(sampleRate, bitCount, channels, samples);
    
    SECTION("Test move constructor"){
        //GIVEN: A constructed object (aud)
        //WHEN: We create a new object using the move constructor
        Audio<int16_t> newAud = std::move(aud);
        
        //THEN: aud should be empty and newAud should be constructed
        REQUIRE( aud.getData().size() == 0 );
        REQUIRE( newAud.getData() == samples );
    }
    
    SECTION("Test copy constructor"){
        //GIVEN: A constructed object (aud)
        //WHEN: We create a new object using the copy constructor
        Audio<int16_t> newAud = aud;
        
        //THEN: aud should be unchanged and newAud should be constructed
        REQUIRE( aud.getData() == samples );
        REQUIRE( newAud.getData() == samples );
    }
    
    SECTION("Test copy assignment"){
        //GIVEN: A constructed object (aud)
        //WHEN: We copy it into another object
        Audio<int16_t> newAud;
        newAud = aud;
        
        //THEN: aud should be unchanged and newAud should have a copy of aud
        REQUIRE( aud.getData() == samples );
        REQUIRE( newAud.getData() == samples );
    }
    
    SECTION("Test move assignment"){
        //GIVEN: A constructed object (aud)
        //WHEN: We move it into another object
        Audio<int16_t> newAud;
        newAud = std::move(aud);
        
        //THEN: aud should be empty and newAud should have a copy of aud
        REQUIRE( aud.getData().size() == 0 );
        REQUIRE( newAud.getData() == samples );
    }

}
