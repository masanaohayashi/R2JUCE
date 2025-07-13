/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "CloudDocAudioProcessor.h"
#include "CloudDocAudioProcessorEditor.h"
#include "Credentials.h"

//==============================================================================
CloudDocAudioProcessor::CloudDocAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(),
                                    true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(),
                                     true)
#endif
      )
#endif
{
    cloudManager = std::make_unique<r2juce::R2CloudManager>();
    cloudManager->setGoogleCredentials(GOOGLE_CLIENT_ID, GOOGLE_CLIENT_SECRET);
    cloudManager->setOneDriveCredentials(ONEDRIVE_CLIENT_ID,
                                         ONEDRIVE_CLIENT_SECRET);
#if JUCE_MAC || JUCE_IOS
    cloudManager->setIcloudContainerId(ICLOUD_CONTAINER_ID);
#endif
#if JUCE_MAC
    cloudManager->setAppGroupId(APP_GROUP_ID);
#endif
    // IMPORTANT: Initialize all providers after setting credentials.
    cloudManager->initializeProviders();

    // Initialize and load settings from the appropriate location.
#if JUCE_MAC
    auto settingsDir =
        juce::File::getContainerForSecurityApplicationGroupIdentifier(
            APP_GROUP_ID);
    // If the container is not valid, fall back to the standard location.
    if (!settingsDir.isDirectory()) {
        settingsDir =
            juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile(JucePlugin_Name);
    }
#else
    auto settingsDir =
        juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile(JucePlugin_Name);
#endif

    if (!settingsDir.exists()) settingsDir.createDirectory();
    
    jassert(settingsDir.isDirectory());

    juce::File settingsFileLocation = settingsDir.getChildFile("settings.xml");
    settingsFile.reset(new juce::PropertiesFile(
        settingsFileLocation, juce::PropertiesFile::Options{}));

    loadSettings();
}

CloudDocAudioProcessor::~CloudDocAudioProcessor() {
    // Save the current file content before exiting.
    if (cloudManager != nullptr && !currentPath.isEmpty() &&
        !currentFilename.isEmpty()) {
        auto currentServiceType = cloudManager->getCurrentState().selectedService;

        juce::String fullPath;
        if (currentPath.isNotEmpty()) {
            if (currentPath.endsWith("/") || currentPath.endsWith("\\"))
                fullPath = currentPath + currentFilename;
            else
                fullPath = currentPath + "/" + currentFilename;
        } else {
            fullPath = currentFilename;
        }

        juce::MemoryBlock data(fileContent.toRawUTF8(),
                               fileContent.getNumBytesAsUTF8());

        // For true web services, we must wait for the async operation to complete
        // to avoid leaks and ensure data is saved. This will block the UI.
        if (currentServiceType == r2juce::R2CloudManager::ServiceType::GoogleDrive ||
            currentServiceType == r2juce::R2CloudManager::ServiceType::OneDrive) {
            juce::WaitableEvent saveFinishedEvent;

            cloudManager->saveFile(fullPath, data,
                                   [&](bool, const juce::String&) {
                                       saveFinishedEvent.signal();
                                   });

            // Wait for up to 10 seconds for the save to complete.
            if (!saveFinishedEvent.wait(10000)) {
                DBG("*** WARNING: Timed out waiting for cloud save on exit.");
            }
        }
        else
        {
            // For local storage and iCloud, the operation is fast enough to be treated as synchronous.
            cloudManager->saveFile(fullPath, data, nullptr);
        }
    }

    saveSettings();
}

r2juce::R2CloudManager& CloudDocAudioProcessor::getCloudManager() {
    return *cloudManager;
}

void CloudDocAudioProcessor::loadSettings() {
    currentPath = settingsFile->getValue(lastFilePathKey, "STUDIO-R/CloudDoc");
    currentFilename = settingsFile->getValue(lastFileNameKey, "data.txt");
    currentServiceId = settingsFile->getIntValue(lastSelectedServiceKey, 1);
}

void CloudDocAudioProcessor::saveSettings() {
    if (settingsFile != nullptr) {
        settingsFile->setValue(lastFilePathKey, currentPath);
        settingsFile->setValue(lastFileNameKey, currentFilename);
        settingsFile->setValue(lastSelectedServiceKey, currentServiceId);
        settingsFile->saveIfNeeded();
    }
}

const juce::String& CloudDocAudioProcessor::getInitialPath() const {
    return currentPath;
}

const juce::String& CloudDocAudioProcessor::getInitialFilename() const {
    return currentFilename;
}

int CloudDocAudioProcessor::getInitialServiceId() const {
    return currentServiceId;
}

const juce::String& CloudDocAudioProcessor::getInitialFileContent() const {
    return fileContent;
}

void CloudDocAudioProcessor::setCurrentPath(const juce::String& newPath) {
    currentPath = newPath;
}

void CloudDocAudioProcessor::setCurrentFilename(
    const juce::String& newFilename) {
    currentFilename = newFilename;
}

void CloudDocAudioProcessor::setCurrentServiceId(int newServiceId) {
    currentServiceId = newServiceId;
}

void CloudDocAudioProcessor::setCurrentFileContent(const juce::String& content) {
    fileContent = content;
}

//==============================================================================
const juce::String CloudDocAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool CloudDocAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool CloudDocAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool CloudDocAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double CloudDocAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int CloudDocAudioProcessor::getNumPrograms() {
    return 1;  // NB: some hosts don't cope very well if you tell them there are 0
               // programs,
               // so this should be at least 1, even if you're not really
               // implementing programs.
}

int CloudDocAudioProcessor::getCurrentProgram() { return 0; }

void CloudDocAudioProcessor::setCurrentProgram(int index) {
    juce::ignoreUnused(index);
}

const juce::String CloudDocAudioProcessor::getProgramName(int index) {
    juce::ignoreUnused(index);
    return {};
}

void CloudDocAudioProcessor::changeProgramName(int index,
                                               const juce::String& newName) {
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void CloudDocAudioProcessor::prepareToPlay(double sampleRate,
                                           int samplesPerBlock) {
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void CloudDocAudioProcessor::releaseResources() {
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CloudDocAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const {
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

// This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void CloudDocAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer& midiMessages) {
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel) {
        auto* channelData = buffer.getWritePointer(channel);
        juce::ignoreUnused(channelData);
        // ..do something to the data...
    }
}

//==============================================================================
bool CloudDocAudioProcessor::hasEditor() const {
    return true;  // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CloudDocAudioProcessor::createEditor() {
    return new CloudDocAudioProcessorEditor(*this);
}

//==============================================================================
void CloudDocAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused(destData);
}

void CloudDocAudioProcessor::setStateInformation(const void* data,
                                                 int sizeInBytes) {
    // You should use this method to restore your parameters from this memory
    // block, whose contents will have been created by the getStateInformation()
    // call.
    juce::ignoreUnused(data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new CloudDocAudioProcessor();
}

