#pragma once

#include "R2CloudStorageProvider.h"

namespace r2juce {

/**
 * @class R2LocalStorageProvider
 * @brief Implements the R2CloudStorageProvider interface for local file
 * storage.
 * @details On macOS, this provider uses a shared App Group container to allow
 * data sharing between applications (e.g., a plugin and its standalone
 * version). On other platforms, it uses a standard user application
 * data directory.
 */
class R2LocalStorageProvider : public R2CloudStorageProvider {
 public:
  /**
   * @brief Constructs a local storage provider.
   * @param appGroupId The App Group Identifier to use on macOS for shared
   * storage. If empty, a default path will be used.
   */
  R2LocalStorageProvider(const juce::String& appGroupId);
  ~R2LocalStorageProvider() override = default;

  void authenticate(AuthCallback callback) override;
  void signOut() override;
  Status getAuthStatus() const override;
  juce::String getDisplayName() const override;
  ServiceType getServiceType() const override;

  void uploadFileByPath(const juce::String& filePath,
                        const juce::MemoryBlock& data,
                        FileOperationCallback callback) override;
  void downloadFileByPath(const juce::String& filePath,
                          DownloadCallback callback) override;
  void listFiles(const juce::String& folderId,
                 FileListCallback callback) override;
  void uploadFile(const juce::String& fileName, const juce::MemoryBlock& data,
                  const juce::String& folderId,
                  FileOperationCallback callback) override;
  void downloadFile(const juce::String& fileId,
                    DownloadCallback callback) override;
  void deleteFile(const juce::String& fileId,
                  FileOperationCallback callback) override;
  void createFolder(const juce::String& folderName,
                    const juce::String& parentId,
                    FileOperationCallback callback) override;

 private:
  /**
   * @brief Gets the root directory for local storage.
   * @return A juce::File object pointing to the root directory. This is
   * platform-dependent.
   */
  juce::File getLocalStorageRoot() const;

  juce::String appGroupId;  // Store the App Group ID

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(R2LocalStorageProvider)
};

}  // namespace r2juce

