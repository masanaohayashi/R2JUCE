#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

namespace r2juce {

/**
 * クラウドサービス認証専用のUIコンポーネント
 * 親コンポーネントの上にオーバーレイ表示される
 */
class R2CloudAuthComponent : public juce::Component,
                             public juce::Timer,
                             public juce::Button::Listener
{
public:
    R2CloudAuthComponent();
    ~R2CloudAuthComponent() override;
    
    enum class ServiceType 
    {
        GoogleDrive
        // 今後追加予定: Dropbox, OneDrive, iCloud
    };
    
    // === 設定 ===
    void setGoogleCredentials(const juce::String& clientId, const juce::String& clientSecret);
    void setServiceType(ServiceType type);
    
    // === 認証開始/停止 ===
    void startAuthentication();
    void stopAuthentication();
    
    // === コールバック ===
    std::function<void(bool success, juce::String errorMessage, juce::String accessToken, juce::String refreshToken)> onAuthenticationComplete;
    std::function<void()> onAuthenticationCancelled;
    
    // === Component overrides ===
    void paint(juce::Graphics& g) override;
    void resized() override;
    void buttonClicked(juce::Button* button) override;
    void timerCallback() override;
    
    // === キー入力でESCで閉じる ===
    bool keyPressed(const juce::KeyPress& key) override;

private:
    ServiceType serviceType = ServiceType::GoogleDrive;
    
    // Google OAuth設定
    juce::String googleClientId;
    juce::String googleClientSecret;
    
    // Device Flow状態
    bool deviceFlowActive = false;
    juce::String deviceCode;
    juce::String userCode;
    juce::String verificationUrl;
    int pollCount = 0;
    int interval = 5;
    static constexpr int maxPolls = 120;
    
    // UI要素
    std::unique_ptr<juce::Label> titleLabel;
    std::unique_ptr<juce::Label> instructionLabel;
    std::unique_ptr<juce::Label> codeDisplayLabel;
    std::unique_ptr<juce::Label> urlDisplayLabel;
    std::unique_ptr<juce::TextButton> openUrlButton;
    std::unique_ptr<juce::TextButton> cancelButton;
    std::unique_ptr<juce::TextButton> copyCodeButton;
    std::unique_ptr<juce::TextButton> copyUrlButton;
    
    // プログレス表示
    std::unique_ptr<juce::Label> statusLabel;
    std::unique_ptr<juce::ProgressBar> progressBar;
    double progressValue = 0.0;
    
    // Internal methods
    void setupUI();
    void requestDeviceCode();
    void pollForAccessToken();
    void parseDeviceCodeResponse(const juce::String& response);
    void parseTokenResponse(const juce::String& response);
    void updateUI();
    void updateStatus(const juce::String& status);
    void showError(const juce::String& error);
    void showSuccess();
    void copyToClipboard(const juce::String& text);
    
    // HTTP Request
    void makeHttpRequest(const juce::String& url, const juce::String& postData, 
                        std::function<void(bool, juce::String, int)> callback);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(R2CloudAuthComponent)
};

} // namespace r2juce
