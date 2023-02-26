//
//  main.cpp
//  3dti_example
//
//  Created by Johan Pauwels on 06/02/2023.
//

#include <algorithm>
#include <functional>
#include <iostream>
#include <vector>
#include "BinauralSpatializer/3DTI_BinauralSpatializer.h"
#include "HRTF/HRTFFactory.h"
#include "ILD/ILDCereal.h"
#include "BRIR/BRIRFactory.h"
#include "AudioFile.h"


int main(int argc, const char * argv[]) {
    Binaural::CCore renderer;
    int sampleRate = 44100;
    int bufferSize = 8192;
    renderer.SetAudioState({sampleRate, bufferSize});
    renderer.SetHRTFResamplingStep(15);

    // Listener
    std::shared_ptr<Binaural::CListener> listener = renderer.CreateListener();

    bool specifiedDelays;
    bool hrtfLoaded = HRTF::CreateFromSofa("3dti_AudioToolkit/resources/HRTF/SOFA/3DTI_HRTF_IRC1008_512s_" + std::to_string(sampleRate) + "Hz.sofa" , listener, specifiedDelays);
    if (!hrtfLoaded) {
        std::cout << "Loading HRTF from SOFA file failed" << std::endl;
    }

    bool ildLoaded = ILD::CreateFrom3dti_ILDNearFieldEffectTable("3dti_AudioToolkit/resources/ILD/NearFieldCompensation_ILD_" + std::to_string(sampleRate) + ".3dti-ild", listener);
    if (!ildLoaded) {
        std::cout << "Loading ILD Near Field Effect simulation file failed" << std::endl;
    }

    // Optional: change listener orientation (in case of head movement)
    //    Common::CTransform listenerTransform;
    //    listenerTransform.SetOrientation(Common::CQuaternion(1, 0, 0, 0));
    //    listener->SetListenerTransform(listenerTransform);

    // Environment
    std::shared_ptr<Binaural::CEnvironment> environment = renderer.CreateEnvironment();
    bool brirLoaded = BRIR::CreateFromSofa("3dti_AudioToolkit/resources/BRIR/SOFA/3DTI_BRIR_large_" + std::to_string(sampleRate) + "Hz.sofa", environment);
    if (!brirLoaded) {
        std::cout << "Loading BRIR from SOFA file failed" << std::endl;
    }

    // Sources
    const std::vector<const string> sourcePaths = {
        "3dti_AudioToolkit/resources/AudioSamples/Anechoic Speech " + std::to_string(sampleRate) + ".wav",
    };
    const std::vector<const Common::CVector3> sourcePositions = {
        Common::CVector3(0, 1, 0),
    };
    if (sourcePaths.size() != sourcePositions.size()) {
        throw std::runtime_error("The number of source positions needs to be equal to the number of source paths");
    }
    const size_t numSources = sourcePaths.size();

    std::vector<std::shared_ptr<Binaural::CSingleSourceDSP>> sources(numSources);
    std::vector<AudioFile<float>> sourceFiles(numSources);
    for (size_t i = 0; i < numSources; ++i) {
        // Create source
        sources[i] = renderer.CreateSingleSourceDSP();
        sources[i]->SetSpatializationMode(Binaural::TSpatializationMode::HighQuality);
        // Set static position
        Common::CTransform sourceTransform;
        sourceTransform.SetPosition(sourcePositions[i]);
        sources[i]->SetSourceTransform(sourceTransform);
        // Open corresponding audio file
        sourceFiles[i].load(sourcePaths[i]);
    }
    int numSamples = sourceFiles.front().getNumSamplesPerChannel();
    for (size_t i = 0; i < numSources; ++i) {
        if (sourceFiles[i].getNumSamplesPerChannel() != numSamples) {
            throw std::runtime_error("All source audio files need to have the same duration");
        }
        if (sourceFiles[i].getSampleRate() != sampleRate) {
            throw std::runtime_error("All source audio files need to have a sample rate of " + std::to_string(sampleRate));
        }
        if (sourceFiles[i].getNumChannels() > 1) {
            std::cout << "Only the first channel of multichannel source audio files will be used" << std::endl;
        }
    }

    // Create binaural stereo file to store result in memory
    AudioFile<double> binauralFile;
    binauralFile.setSampleRate(sampleRate);
    binauralFile.setBitDepth(16);
    binauralFile.setAudioBufferSize(2, numSamples); // initialised to all zeros

    // Create buffers to process audio in blocks
    CMonoBuffer<float> inputBuffer(bufferSize);
    CMonoBuffer<float> leftBuffer(bufferSize);
    CMonoBuffer<float> rightBuffer(bufferSize);

    int start = 0;
    // Helper function for adding output buffers to contents of stereo file
    auto addToOutput = [&](const int size) {
        std::transform(leftBuffer.begin(), leftBuffer.begin()+size, &binauralFile.samples[0][start], &binauralFile.samples[0][start], std::plus<float>());
        std::transform(rightBuffer.begin(), rightBuffer.begin()+size, &binauralFile.samples[1][start], &binauralFile.samples[1][start], std::plus<float>());
    };
    // Helper function for blockwise rendering of binaural audio and accumulating into stereo file
    auto processBuffer = [&](const int size) {
        for (size_t i = 0; i < numSources; ++i) {
            std::copy(&sourceFiles[i].samples.front()[start], &sourceFiles[i].samples.front()[start+size], inputBuffer.begin());
            sources[i]->SetBuffer(inputBuffer);
            sources[i]->ProcessAnechoic(leftBuffer, rightBuffer);
            addToOutput(size);
        }
        environment->ProcessVirtualAmbisonicReverb(leftBuffer, rightBuffer);
        addToOutput(size);
    };

    // Process complete buffers
    for (; start < numSamples - bufferSize; start += bufferSize) {
        processBuffer(bufferSize);
    }
    // Process last partial buffer
    int remainingSize = numSamples - start;
    processBuffer(remainingSize);

    // Save resulting wave file to disk
    binauralFile.save("binaural.wav");

    return 0;
}
