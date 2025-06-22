#pragma once

#include "R2CloudStorageProvider.h"

namespace r2juce {
class R2OneDriveProvider : public R2CloudStorageProvider
{
public:
    R2OneDriveProvider();
    ~R2OneDriveProvider() override = default;
    
    // R2CloudStorageProvider interface
    void authenticate(AuthCallback callback) override;
    void signOut() override;
    Status getAuthStatus() const override;
    juce::String getDisplayName() const override;
    
    void listFiles(const juce::String& folderId, FileListCallback callback) override;
    void uploadFile(const juce::String& fileName, const juce::MemoryBlock& data,
                    const juce::String& folderId, FileOperationCallback callback) override;
    void downloadFile(const juce::String& fileId, DownloadCallback callback) override;
    void deleteFile(const juce::String& fileId, FileOperationCallback callback) override;
    void createFolder(const juce::String& folderName, const juce::String& parentId,
                      FileOperationCallback callback) override;
    
    // OneDrive specific
    void setClientCredentials(const juce::String& clientId, const juce::String& clientSecret);
    void exchangeAuthCodeForTokens(const juce::String& authCode, std::function<void(bool, juce::String)> callback);
    void setTokens(const juce::String& accessToken, const juce::String& refreshToken);
    void downloadFileWithPath(const juce::String& filePath, DownloadCallback callback);

private:
    void startNewAuthFlow(AuthCallback callback);
    void parseTokenResponse(const juce::String& response, std::function<void(bool, juce::String)> callback);

    class OAuth2Handler : public juce::Component
    {
    public:
        OAuth2Handler(R2OneDriveProvider& parent);
        ~OAuth2Handler() override = default;
        
        void startAuthentication(const juce::String& clientId, R2CloudStorageProvider::AuthCallback callback);
        void resized() override;

    private:
        juce::String generateStateParameter();
        void checkAuthCallback(const juce::String& url);
        void handleAuthSuccess(const juce::String& authCode);
        void handleAuthError(const juce::String& error);
        juce::String extractURLParameter(const juce::String& url, const juce::String& paramName);
        juce::String extractAuthCode(const juce::String& url);
        juce::String extractState(const juce::String& url);
        juce::String extractError(const juce::String& url);
        void showSuccessPage();
        void showErrorPage(const juce::String& error);

        R2OneDriveProvider& provider;
        std::unique_ptr<juce::WebBrowserComponent> webBrowser;
        R2CloudStorageProvider::AuthCallback authCallback;
        juce::String redirectUri;
        juce::String currentClientId;
        juce::String stateParameter;
   };

    std::unique_ptr<OAuth2Handler> oauth2Handler;
    
    juce::String clientId;
    juce::String clientSecret;
    juce::String accessToken;
    juce::String refreshToken;
    juce::Time tokenExpiry;
    
    bool isTokenValid() const;
    void refreshAccessToken(std::function<void(bool)> callback);
    void makeAPIRequest(const juce::String& endpoint, const juce::String& method,
                        const juce::StringPairArray& headers, const juce::String& postData,
                        std::function<void(bool, juce::String)> callback);
    
    void saveTokens();
    bool loadTokens();
    juce::File getTokenFile() const;
};
}   //  namespace r2juce
