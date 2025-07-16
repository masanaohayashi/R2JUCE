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
    auto settingsDir = juce::File::getContainerForSecurityApplicationGroupIdentifier(APP_GROUP_ID);
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
    applyCacheSettings();
}

CloudDocAudioProcessor::~CloudDocAudioProcessor() {
    // Save the current file content before exiting.
    if (cloudManager != nullptr && !currentPath.isEmpty() &&
        !currentFilename.isEmpty()) {
        auto currentProvider = cloudManager->getProvider();
        if (!currentProvider) return;

        auto currentServiceType = currentProvider->getServiceType();

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

        // For true web services without caching, we must wait for the async operation to complete
        // to avoid leaks and ensure data is saved. This will block the UI.
        if (currentProvider->isCachingEnabled() == false && (currentServiceType == r2juce::R2CloudManager::ServiceType::GoogleDrive ||
            currentServiceType == r2juce::R2CloudManager::ServiceType::OneDrive)) {
            
            if (!cloudManager->saveFileSync(fullPath, data))
            {
                DBG("*** WARNING: Failed to save file synchronously on exit.");
            }
        }
        else
        {
            // For local storage, iCloud and cached services, the operation is fast enough.
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
    useLocalCache = settingsFile->getBoolValue(useLocalCacheKey, true);
}

void CloudDocAudioProcessor::saveSettings() {
    if (settingsFile != nullptr) {
        settingsFile->setValue(lastFilePathKey, currentPath);
        settingsFile->setValue(lastFileNameKey, currentFilename);
        settingsFile->setValue(lastSelectedServiceKey, currentServiceId);
        settingsFile->setValue(useLocalCacheKey, useLocalCache);
        settingsFile->saveIfNeeded();
    }
}

void CloudDocAudioProcessor::applyCacheSettings()
{
    DBG("CloudDocAudioProcessor::applyCacheSettings called. useLocalCache is: " + juce::String(useLocalCache ? "true" : "false"));
    
    juce::File cacheDir;
#if (JUCE_MAC || JUCE_IOS)
    if (juce::String(APP_GROUP_ID).isNotEmpty())
    {
        cacheDir = juce::File::getContainerForSecurityApplicationGroupIdentifier(APP_GROUP_ID);
        if (cacheDir.isDirectory())
        {
             DBG("Using App Group container for cache: " + cacheDir.getFullPathName());
        }
        else
        {
            DBG("App Group container not available, falling back to user app data.");
            cacheDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile(JucePlugin_Name);
        }
    }
    else
    {
         cacheDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile(JucePlugin_Name);
    }
#else
    cacheDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile(JucePlugin_Name);
#endif

    cacheDir = cacheDir.getChildFile("Cache");
    if (!cacheDir.exists())
        cacheDir.createDirectory();
    
    DBG("Final cache directory: " + cacheDir.getFullPathName());
        
    cloudManager->setCachingForService(r2juce::R2CloudManager::ServiceType::GoogleDrive, useLocalCache, cacheDir);
    cloudManager->setCachingForService(r2juce::R2CloudManager::ServiceType::OneDrive, useLocalCache, cacheDir);
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

bool CloudDocAudioProcessor::getInitialUseLocalCache() const
{
    return useLocalCache;
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

void CloudDocAudioProcessor::setUseLocalCache(bool shouldUseCache)
{
    DBG("CloudDocAudioProcessor::setUseLocalCache called with: " + juce::String(shouldUseCache ? "true" : "false"));
    if (useLocalCache != shouldUseCache)
    {
        useLocalCache = shouldUseCache;
        applyCacheSettings();
    }
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
    return 1;
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
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void CloudDocAudioProcessor::releaseResources() {
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CloudDocAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const {
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

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

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    for (int channel = 0; channel < totalNumInputChannels; ++channel) {
        auto* channelData = buffer.getWritePointer(channel);
        juce::ignoreUnused(channelData);
    }
}

//==============================================================================
bool CloudDocAudioProcessor::hasEditor() const {
    return true;
}

juce::AudioProcessorEditor* CloudDocAudioProcessor::createEditor() {
    return new CloudDocAudioProcessorEditor(*this);
}

//==============================================================================
void CloudDocAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    juce::ignoreUnused(destData);
}

void CloudDocAudioProcessor::setStateInformation(const void* data,
                                                 int sizeInBytes) {
    juce::ignoreUnused(data, sizeInBytes);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new CloudDocAudioProcessor();
}

