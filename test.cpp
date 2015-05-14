/*
 * All unit tests for the audio class is implemented here. Move/copy semantics
 * have been tested for both the mono and stereo specialisations of the audio 
 * class. All operator overloads and transformations have also been tested.
 * I have used an "if", "then", "when" pattern to set out the unit tests. This 
 * should make them easier to read and understand.
 *
 * Muhummad Patel PTLMUH006
 * 14-May-2015
 */

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

TEST_CASE("Test Mono Audio Transformations", ""){

    int sampleRate = 44100,
        bitCount = 16,
        channels = 1;

    std::vector<int16_t> samplesA = {-1, 1, 2, -2, 3, -3, 10, -10};
    Audio<int16_t> a(sampleRate, bitCount, channels, samplesA);

    SECTION("Test reverse transformation"){
        //GIVEN: some audio file a
        //WHEN: we apply the reverse transformation to a
        Audio<int16_t> rev = a.reverse();

        //THEN: we should get a new audio object with all the samples reversed
        std::vector<int16_t> expected = {-10, 10, -3, 3, -2, 2, 1, -1};
        REQUIRE( rev.getData().size() == expected.size() );
        REQUIRE( rev.getData() == expected );
    }

    SECTION("Test ranged add transformation"){
        //GIVEN: some audio files a and b
        std::vector<int16_t> samplesB = {-1, 1, std::numeric_limits<int16_t>::max(), 2, std::numeric_limits<int16_t>::min(), -3, 10, -10};
        Audio<int16_t> b(sampleRate, bitCount, channels, samplesB);

        //WHEN: we apply the ranged add transformation to a and b
        Audio<int16_t> radd = Audio<int16_t>::rangedAdd(a, std::pair<int, int>(1, 4), b, std::pair<int, int>(2, 5));

        //THEN: we should get a new audio object with all the samples in the given ranges added
        std::vector<int16_t> expected = {0, std::numeric_limits<int16_t>::max(), 4, std::numeric_limits<int16_t>::min()};
        REQUIRE( radd.getData().size() == expected.size() );
        REQUIRE( radd.getData() == expected );
        REQUIRE( a.getData().size() == samplesA.size() );
        REQUIRE( b.getData().size() == samplesB.size() );
    }

    SECTION("Test compute rms transformation"){
        //GIVEN: some audio object a
        //WHEN: we compute the rms for that audio object
        std::pair<float, float> rms = a.computeRMS();

        //THEN: we should get the correct RMS value and the object should not have changed
        std::pair<float, float> expected(5.338539126, 0.0f);
        REQUIRE( rms.first == Approx(expected.first) );
        REQUIRE( rms.second == Approx(expected.second) );
        REQUIRE( a.getData() == samplesA);
    }

    SECTION("Test sound normalization transformation"){
        //GIVEN: some audio object a
        //WHEN: we normalize to a specific rms value
        std::vector<int16_t> samplesB = {1, 1, 1, 2, 1, 1, 1, 1, 1, 1};
        Audio<int16_t> b(sampleRate, bitCount, channels, samplesB);
        Audio<int16_t> norm = b.normalized(std::pair<float, float>(20.0f, 20.0f));

        //THEN: we should get a new audio object with the channel normalized to the specified level
        REQUIRE( norm.computeRMS().first == Approx(20).epsilon(1.0));
        REQUIRE( a.getData() == samplesA);
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

TEST_CASE("Test Stereo Move/Copy Semantics", ""){
    int sampleRate = 44100,
        bitCount = 16,
        channels = 2;
    std::vector<std::pair<int16_t, int16_t>> samples = {
        std::pair<int16_t, int16_t>(1, -1),
        std::pair<int16_t, int16_t>(2, -2),
        std::pair<int16_t, int16_t>(3, -3),
        std::pair<int16_t, int16_t>(4, -4),
        std::pair<int16_t, int16_t>(5, -5)
    };
    
    Audio<std::pair<int16_t, int16_t>> aud(sampleRate, bitCount, channels, samples);
    
    SECTION("Test move constructor"){
        //GIVEN: A constructed object (aud)
        //WHEN: We create a new object using the move constructor
        Audio<std::pair<int16_t, int16_t>> newAud = std::move(aud);
        
        //THEN: aud should be empty and newAud should be constructed
        REQUIRE( aud.getData().size() == 0 );
        REQUIRE( newAud.getData() == samples );
    }
    
    SECTION("Test copy constructor"){
        //GIVEN: A constructed object (aud)
        //WHEN: We create a new object using the copy constructor
        Audio<std::pair<int16_t, int16_t>> newAud = aud;
        
        //THEN: aud should be unchanged and newAud should be constructed
        REQUIRE( aud.getData() == samples );
        REQUIRE( newAud.getData() == samples );
    }
    
    SECTION("Test copy assignment"){
        //GIVEN: A constructed object (aud)
        //WHEN: We copy it into another object
        Audio<std::pair<int16_t, int16_t>> newAud;
        newAud = aud;
        
        //THEN: aud should be unchanged and newAud should have a copy of aud
        REQUIRE( aud.getData() == samples );
        REQUIRE( newAud.getData() == samples );
    }
    
    SECTION("Test move assignment"){
        //GIVEN: A constructed object (aud)
        //WHEN: We move it into another object
        Audio<std::pair<int16_t, int16_t>> newAud;
        newAud = std::move(aud);
        
        //THEN: aud should be empty and newAud should have a copy of aud
        REQUIRE( aud.getData().size() == 0 );
        REQUIRE( newAud.getData() == samples );
    }
}

TEST_CASE("Test Stereo Operator Overloads", ""){
    
    int sampleRate = 44100,
        bitCount = 16,
        channels = 2;
    std::vector<std::pair<int16_t, int16_t>> samplesA = {
        std::pair<int16_t, int16_t>(1, -1),
        std::pair<int16_t, int16_t>(2, -2),
        std::pair<int16_t, int16_t>(3, -3),
        std::pair<int16_t, int16_t>(4, -4),
        std::pair<int16_t, int16_t>(5, -5)
    };
    
    Audio<std::pair<int16_t, int16_t>> a(sampleRate, bitCount, channels, samplesA);
    
    SECTION("Test concatenate operator a|b"){
        //GIVEN: some audio objects a and b
        std::vector<std::pair<int16_t, int16_t>> samplesB = {
            std::pair<int16_t, int16_t>(6, -1),
            std::pair<int16_t, int16_t>(7, -2),
            std::pair<int16_t, int16_t>(8, -3),
            std::pair<int16_t, int16_t>(9, -4),
            std::pair<int16_t, int16_t>(10, -5)
        };
        Audio<std::pair<int16_t, int16_t>> b(sampleRate, bitCount, channels, samplesB);
        
        //WHEN: we concatenate them using the concatenation operator
        Audio<std::pair<int16_t, int16_t>> concatenated = a | b;
        
        //THEN: the result should be the two files back to back
        std::vector<std::pair<int16_t, int16_t>> expected = {
            std::pair<int16_t, int16_t>(1, -1),
            std::pair<int16_t, int16_t>(2, -2),
            std::pair<int16_t, int16_t>(3, -3),
            std::pair<int16_t, int16_t>(4, -4),
            std::pair<int16_t, int16_t>(5, -5),
            std::pair<int16_t, int16_t>(6, -1),
            std::pair<int16_t, int16_t>(7, -2),
            std::pair<int16_t, int16_t>(8, -3),
            std::pair<int16_t, int16_t>(9, -4),
            std::pair<int16_t, int16_t>(10, -5)
        };
        REQUIRE( concatenated.getData().size() == expected.size() );
        REQUIRE( concatenated.getData() == expected );
    }
    
    SECTION("Test volume factor operator a*f"){
        //GIVEN: some audio object a        
        //WHEN: we apply the volume factor operator with 0.5 and 2.0
        Audio<std::pair<int16_t, int16_t>> factored = a * std::pair<float, float>(0.5f, 2.0f);
        
        //THEN: the result should have all samples halved and clamped
        std::vector<std::pair<int16_t, int16_t>> expected = {
            std::pair<int16_t, int16_t>(0, -2),
            std::pair<int16_t, int16_t>(1, -4),
            std::pair<int16_t, int16_t>(1, -6),
            std::pair<int16_t, int16_t>(2, -8),
            std::pair<int16_t, int16_t>(2, -10)
        };
        REQUIRE( factored.getData().size() == expected.size() );
        REQUIRE( factored.getData() == expected );
    }
    
    SECTION("Test addition operator a+b"){
        //GIVEN: some audio objects a and b
        std::vector<std::pair<int16_t, int16_t>> samplesB = {
            std::pair<int16_t, int16_t>(6, -1),
            std::pair<int16_t, int16_t>(7, -2),
            std::pair<int16_t, int16_t>(8, -3),
            std::pair<int16_t, int16_t>(9, -4),
            std::pair<int16_t, int16_t>(std::numeric_limits<int16_t>::max(), std::numeric_limits<int16_t>::min())
        };
        Audio<std::pair<int16_t, int16_t>> b(sampleRate, bitCount, channels, samplesB);
        
        //WHEN: we add a and b
        Audio<std::pair<int16_t, int16_t>> sum = a + b;
        
        //THEN: the result should have all samples added together and clamped
        std::vector<std::pair<int16_t, int16_t>> expected = {
            std::pair<int16_t, int16_t>(7, -2),
            std::pair<int16_t, int16_t>(9, -4),
            std::pair<int16_t, int16_t>(11, -6),
            std::pair<int16_t, int16_t>(13, -8),
            std::pair<int16_t, int16_t>(std::numeric_limits<int16_t>::max(), std::numeric_limits<int16_t>::min())
        };
        
        REQUIRE( sum.getData().size() == expected.size() );
        REQUIRE( sum.getData() == expected );
    }
    
    SECTION("Test cut operator a^f"){
        //GIVEN: some audio object a
        //WHEN: we apply the cut operator with (2, 4)
        Audio<std::pair<int16_t, int16_t>> cut = a ^ std::pair<int, int>(2, 4);

        //THEN: the result should be a copy of a without the samples in the specified range (inclusive)
        std::vector<std::pair<int16_t, int16_t>> expected = {
            std::pair<int16_t, int16_t>(1, -1),
            std::pair<int16_t, int16_t>(5, -5)
        };
        
        REQUIRE( cut.getData().size() == expected.size() );
        REQUIRE( cut.getData() == expected );
    }
}

TEST_CASE("Test Stereo Audio Transformations", ""){

    int sampleRate = 44100,
        bitCount = 16,
        channels = 2;
        
    std::vector<std::pair<int16_t, int16_t>> samplesA = {
        std::pair<int16_t, int16_t>(1, -1),
        std::pair<int16_t, int16_t>(2, -2),
        std::pair<int16_t, int16_t>(3, -3),
        std::pair<int16_t, int16_t>(4, -4),
        std::pair<int16_t, int16_t>(5, -5)
    };
    
    Audio<std::pair<int16_t, int16_t>> a(sampleRate, bitCount, channels, samplesA);
    
    SECTION("Test reverse transformation"){
        //GIVEN: some audio file a
        //WHEN: we apply the reverse transformation to a
        Audio<std::pair<int16_t, int16_t>> rev = a.reverse();

        //THEN: we should get a new audio object with all the samples reversed
        std::vector<std::pair<int16_t, int16_t>> expected = {
            std::pair<int16_t, int16_t>(5, -5),
            std::pair<int16_t, int16_t>(4, -4),
            std::pair<int16_t, int16_t>(3, -3),
            std::pair<int16_t, int16_t>(2, -2),
            std::pair<int16_t, int16_t>(1, -1)
        };
    
        REQUIRE( rev.getData().size() == expected.size() );
        REQUIRE( rev.getData() == expected );
    }
    
    SECTION("Test ranged add transformation"){
        //GIVEN: some audio files a and b
        std::vector<std::pair<int16_t, int16_t>> samplesB = {
            std::pair<int16_t, int16_t>(6, -1),
            std::pair<int16_t, int16_t>(7, -2),
            std::pair<int16_t, int16_t>(std::numeric_limits<int16_t>::max(), std::numeric_limits<int16_t>::min()),
            std::pair<int16_t, int16_t>(8, -3),
            std::pair<int16_t, int16_t>(9, -4)
        };
        Audio<std::pair<int16_t, int16_t>> b(sampleRate, bitCount, channels, samplesB);

        //WHEN: we apply the ranged add transformation to a and b
        Audio<std::pair<int16_t, int16_t>> radd = Audio<std::pair<int16_t, int16_t>>::rangedAdd(a, std::pair<int, int>(1, 4), b, std::pair<int, int>(2, 5));

        //THEN: we should get a new audio object with all the samples in the given ranges added
        std::vector<std::pair<int16_t, int16_t>> expected = {
            std::pair<int16_t, int16_t>(8, -3),
            std::pair<int16_t, int16_t>(std::numeric_limits<int16_t>::max(), std::numeric_limits<int16_t>::min()),
            std::pair<int16_t, int16_t>(11, -6),
            std::pair<int16_t, int16_t>(13, -8)
        };
        
        REQUIRE( radd.getData().size() == expected.size() );
        REQUIRE( a.getData().size() == samplesA.size() );
        REQUIRE( b.getData().size() == samplesB.size() );
    }
    
    SECTION("Test compute rms transformation"){
        //GIVEN: some audio object a
        //WHEN: we compute the rms for that audio object
        std::pair<float, float> rms = a.computeRMS();

        //THEN: we should get the correct RMS value and the object should not have changed
        std::pair<float, float> expected(3.3166247904, 3.3166247904);
        REQUIRE( rms.first == Approx(expected.first) );
        REQUIRE( rms.second == Approx(expected.second) );
        REQUIRE( a.getData() == samplesA);
    }
    
    SECTION("Test sound normalization transformation"){
        //GIVEN: some audio object b
        std::vector<std::pair<int16_t, int16_t>> samplesB = {
            std::pair<int16_t, int16_t>(1, 2),
            std::pair<int16_t, int16_t>(1, 2),
            std::pair<int16_t, int16_t>(1, 2),
            std::pair<int16_t, int16_t>(1, 2),
            std::pair<int16_t, int16_t>(1, 2)
        };
        Audio<std::pair<int16_t, int16_t>> b(sampleRate, bitCount, channels, samplesB);
        
        //WHEN: we normalize to a specific rms value
        Audio<std::pair<int16_t, int16_t>> norm = b.normalized(std::pair<float, float>(20.0f, 40.0f));

        //THEN: we should get a new audio object with the channels normalized to the specified level
        REQUIRE( norm.computeRMS().first == Approx(20).epsilon(1.0) );
        REQUIRE( norm.computeRMS().second == Approx(40).epsilon(1.0) );
        REQUIRE( a.getData() == samplesA);
    }
}



























