##Assignment 5 - _Simple Audio Manipulation Program \(samp\)_

**Author** : Muhummad Yunus Patel  
**Student#** : PTLMUH006  
**Date**  : 14-May-2015  

###Usage:
./samp -r sampleRateInHz -b bitcount -c noChannels
	 \[-o outfilename\] \[<ops>\]
	 sndfile1 \[sndfile2\]

**Options:**

* -add: add sndFile1 and sndFile2
* -cut r1 r2: remove samples over range [r1,r2] (inclusive)
* -radd r1 r2 s1 s2 : add sndFile1 and sndFile2 over sub-ranges(seconds)
* -cat: concatenate soundFile1 and soundFile2
* -v r1 r2: volume factor for left/right audio
* -rev: reverse sound file
* -rms: Prints out the RMS of the soundfile
* -norm r1 r2: normalize file for left/right audio

**Compiling:**

* To compile the executable, simply type _make_.
* To run, see usage notes above.
* To clean up, type _make clean_.

To compile and run the unit tests, type _make test_.

**Example runs:**  
_Note_:	before running any of these, please compile, and ensure that the
 referenced files are present in the working directory.

* ./samp -r 44100 -b 16 -c 2 -add frogs18sec_44100_signed_16bit_stereo.raw beez18sec_44100_signed_16bit_stereo.raw 
* ./samp -r 44100 -b 16 -c 2 -cut 44100 220500 frogs18sec_44100_signed_16bit_stereo.raw
* ./samp -r 44100 -b 16 -c 2 -radd 1 4 2 5 frogs18sec_44100_signed_16bit_stereo.raw beez18sec_44100_signed_16bit_stereo.raw 
* ./samp -r 44100 -b 16 -c 2 -cat frogs18sec_44100_signed_16bit_stereo.raw beez18sec_44100_signed_16bit_stereo.raw 
* ./samp -r 44100 -b 16 -c 2 -v 0.5 0.5 beez18sec_44100_signed_16bit_stereo.raw
* ./samp -r 44100 -b 16 -c 2 -rev beez18sec_44100_signed_16bit_stereo.raw
* ./samp -r 44100 -b 16 -c 2 -rms beez18sec_44100_signed_16bit_stereo.raw
* ./samp -r 44100 -b 16 -c 2 -norm 500 500 beez18sec_44100_signed_16bit_stereo.raw

Should you require any further clarification regarding this submission, please
feel free to contact me through muhummad.patel@gmail.com. Thank you. :)

###Description:
A description of what each file does and how it fits into the program follows:  

1. **audio.h:**  
    This header file defines the Audio class used to read in, store, manipulate,
    and write .raw audio files. The Audio class is templated to allow for audio
    files that use samples of different sizes \(in pearticular 8bit and 16bit 
    samples\). The template was partially specialised for the case where we pass
    in a std::pair<T, T> as the type parameter. The relevant functions were then
    adapted to work for stereo audio clips.

2. **main.cpp:**  
    This is the driver/main file. It contains the main method that is run when 
    the program is executed. It handles all argument parsing and carries out the
    option requested by the options passed in. If incorrect arguments/format are
    used, it will print out the expected arguments and usage.

3. **test.cpp:**  
    This file contains the implementation of the unit-tests that were requested
    as part of this assignment specification. Each test case, tests a specific
    use-case and the sections test each step of that use case. GIVEN, WHEN, THEN
    semantics have been used for each of the unit tests for readability. Both
    stereo and mono instances of the Audio class
