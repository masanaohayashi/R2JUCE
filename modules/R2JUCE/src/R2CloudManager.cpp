#include "R2CloudManager.h"
#include "R2LocalStorageProvider.h"
#include "R2GoogleDriveProvider.h"
#include "R2OneDriveProvider.h"
#include "R2CloudAuthComponent.h"

namespace r2juce {

R2CloudManager::R2CloudManager()
{
    initializeProviders();
}

R2CloudManager::~R2CloudManager()
{
    DBG("R2CloudManager destructor called");
    
    // Cancel authentication and clear callbacks first
    cancelAuthentication();
    
    // Clear callbacks explicitly
    onAuthStatusChanged = nullptr;
    onServiceChanged = nullptr;
    
    juce::Thread::sleep(10);
    
    // Reset providers (this will trigger their destructors)
    localProvider.reset();
    googleDriveProvider.reset();
    oneDriveProvider.reset();
}

void R2CloudManager::initializeProviders()
{
    localProvider = createProvider(ServiceType::Local);
    googleDriveProvider = createProvider(ServiceType::GoogleDrive);
    oneDriveProvider = createProvider(ServiceType::OneDrive);
    
    auto documentsDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
    auto cloudDocDir = documentsDir.getChildFile("CloudDoc");
    setLocalStorageDirectory(cloudDocDir);
}

std::unique_ptr<R2CloudStorageProvider> R2CloudManager::createProvider(ServiceType type)
{
    switch (type)
    {
        case ServiceType::GoogleDrive:
            return std::make_unique<R2GoogleDriveProvider>();
        case ServiceType::OneDrive:
            return std::make_unique<R2OneDriveProvider>();
        case ServiceType::Local:
        default:
            return std::make_unique<R2LocalStorageProvider>();
    }
}

void R2CloudManager::setGoogleCredentials(const juce::String& clientId, const juce::String& clientSecret)
{
    googleClientId = clientId;
    googleClientSecret = clientSecret;
    
    if (auto* googleProv = dynamic_cast<R2GoogleDriveProvider*>(googleDriveProvider.get()))
    {
        googleProv->setClientCredentials(clientId, clientSecret);
    }
}

void R2CloudManager::setOneDriveCredentials(const juce::String& clientId, const juce::String& clientSecret)
{
    oneDriveClientId = clientId;
    oneDriveClientSecret = clientSecret;
    
    if (auto* oneDriveProv = dynamic_cast<R2OneDriveProvider*>(oneDriveProvider.get()))
    {
        oneDriveProv->setClientCredentials(clientId, clientSecret);
    }
}

void R2CloudManager::setLocalStorageDirectory(const juce::File& directory)
{
    localStorageDir = directory;
    
    if (auto* localProv = dynamic_cast<R2LocalStorageProvider*>(localProvider.get()))
    {
        localProv->setRootDirectory(directory);
    }
}

void R2CloudManager::selectService(ServiceType serviceType)
{
    if (currentServiceType == serviceType)
        return;
    
    hideAuthenticationUI();
    
    currentServiceType = serviceType;
    updateAuthStatus();
    
    if (onServiceChanged)
        onServiceChanged(serviceType);
}

R2CloudManager::AuthStatus R2CloudManager::getAuthStatus() const
{
    return currentAuthStatus;
}

R2CloudStorageProvider* R2CloudManager::getCurrentProvider()
{
    switch (currentServiceType)
    {
        case ServiceType::GoogleDrive:
            return googleDriveProvider.get();
        case ServiceType::OneDrive:
            return oneDriveProvider.get();
        case ServiceType::Local:
        default:
            return localProvider.get();
    }
}

const R2CloudStorageProvider* R2CloudManager::getCurrentProvider() const
{
    switch (currentServiceType)
    {
        case ServiceType::GoogleDrive:
            return googleDriveProvider.get();
        case ServiceType::OneDrive:
            return oneDriveProvider.get();
        case ServiceType::Local:
        default:
            return localProvider.get();
    }
}

void R2CloudManager::updateAuthStatus()
{
    auto provider = getCurrentProvider();
    if (!provider)
    {
        setAuthStatus(AuthStatus::Error);
        return;
    }
    
    AuthStatus newStatus;
    switch (provider->getAuthStatus())
    {
        case R2CloudStorageProvider::Status::Authenticated:
            newStatus = AuthStatus::Authenticated;
            break;
        case R2CloudStorageProvider::Status::Authenticating:
            newStatus = AuthStatus::Authenticating;
            break;
        case R2CloudStorageProvider::Status::Error:
            newStatus = AuthStatus::Error;
            break;
        case R2CloudStorageProvider::Status::NotAuthenticated:
        default:
            newStatus = AuthStatus::NotAuthenticated;
            break;
    }
    
    setAuthStatus(newStatus);
}

void R2CloudManager::setAuthStatus(AuthStatus newStatus)
{
    if (currentAuthStatus != newStatus)
    {
        currentAuthStatus = newStatus;
        if (onAuthStatusChanged)
            onAuthStatusChanged(newStatus);
    }
}

bool R2CloudManager::needsAuthentication() const
{
    auto provider = getCurrentProvider();
    if (!provider)
        return false;
    
    return provider->getAuthStatus() != R2CloudStorageProvider::Status::Authenticated;
}

void R2CloudManager::showAuthenticationUI(juce::Component* parent)
{
    DBG("R2CloudManager::showAuthenticationUI() called");

    auto provider = getCurrentProvider();
    if (provider == nullptr)
    {
        DBG("No provider selected");
        return;
    }

    // �����̔F��UI������ꍇ�͐�ɃL�����Z��
    if (authComponent != nullptr)
    {
        cancelAuthentication();
    }

    // ���[�J���X�g���[�W�̏ꍇ�͔F�ؕs�v
    if (currentServiceType == ServiceType::Local)
    {
        setAuthStatus(AuthStatus::Authenticated);
        return;
    }

    try
    {
        // Google Drive �� OneDrive �̏ꍇ�́ADevice Flow�F�؂ƒʏ��OAuth2��g�ݍ��킹��
        if (currentServiceType == ServiceType::GoogleDrive || currentServiceType == ServiceType::OneDrive)
        {
            // �܂��v���o�C�_�[�ɔF�؂����݂�����i�����g�[�N���⃊�t���b�V���g�[�N���̃`�F�b�N�j
            currentAuthStatus = AuthStatus::Authenticating;

            if (onAuthStatusChanged)
            {
                onAuthStatusChanged(currentAuthStatus);
            }

            // �R�[���o�b�N�����[�J���ϐ��ɃR�s�[
            auto statusCallback = onAuthStatusChanged;

            provider->authenticate([this, parent, statusCallback](bool success, juce::String errorMessage)
                {
                    if (success)
                    {
                        // �����g�[�N���ŔF�ؐ���
                        juce::MessageManager::callAsync([this, statusCallback]()
                            {
                                this->currentAuthStatus = AuthStatus::Authenticated;
                                if (statusCallback)
                                {
                                    statusCallback(this->currentAuthStatus);
                                }
                            });
                    }
                    else
                    {
                        // �F�؂��K�v�ȏꍇ - Device Flow�F��UI��\��
                        if (errorMessage.contains("device flow") || errorMessage.contains("authentication required"))
                        {
                            juce::MessageManager::callAsync([this, parent]()
                                {
                                    this->startDeviceFlowAuthentication(parent);
                                });
                        }
                        else
                        {
                            // ���̑��̃G���[
                            juce::MessageManager::callAsync([this, statusCallback, errorMessage]()
                                {
                                    this->currentAuthStatus = AuthStatus::Error;
                                    if (statusCallback)
                                    {
                                        statusCallback(this->currentAuthStatus);
                                    }
                                });
                        }
                    }
                });
        }
        else
        {
            // ���̑��̃v���o�C�_�[�̏ꍇ�͒��ڔF��
            currentAuthStatus = AuthStatus::Authenticating;

            if (onAuthStatusChanged)
            {
                onAuthStatusChanged(currentAuthStatus);
            }

            // �R�[���o�b�N�����[�J���ϐ��ɃR�s�[
            auto statusCallback = onAuthStatusChanged;

            provider->authenticate([this, statusCallback](bool success, juce::String errorMessage)
                {
                    juce::MessageManager::callAsync([this, statusCallback, success, errorMessage]()
                        {
                            if (success)
                            {
                                this->currentAuthStatus = AuthStatus::Authenticated;
                            }
                            else
                            {
                                this->currentAuthStatus = AuthStatus::Error;
                            }

                            if (statusCallback)
                            {
                                statusCallback(this->currentAuthStatus);
                            }
                        });
                });
        }
    }
    catch (const std::exception& e)
    {
        DBG("Exception in showAuthenticationUI: " + juce::String(e.what()));
        currentAuthStatus = AuthStatus::Error;
        if (onAuthStatusChanged)
        {
            onAuthStatusChanged(currentAuthStatus);
        }
    }
    catch (...)
    {
        DBG("Unknown exception in showAuthenticationUI");
        currentAuthStatus = AuthStatus::Error;
        if (onAuthStatusChanged)
        {
            onAuthStatusChanged(currentAuthStatus);
        }
    }
}

void R2CloudManager::startDeviceFlowAuthentication(juce::Component* parent)
{
    DBG("Starting Device Flow authentication");

    // Device Flow�F��UI���쐬
    authComponent = std::make_unique<R2CloudAuthComponent>();
    parentForAuth = parent;

    // �F�؃T�[�r�X�^�C�v��ݒ�
    if (currentServiceType == ServiceType::GoogleDrive)
    {
        authComponent->setServiceType(R2CloudAuthComponent::ServiceType::GoogleDrive);
        authComponent->setGoogleCredentials(googleClientId, googleClientSecret);
    }
    else if (currentServiceType == ServiceType::OneDrive)
    {
        authComponent->setServiceType(R2CloudAuthComponent::ServiceType::OneDrive);
        authComponent->setOneDriveCredentials(oneDriveClientId, oneDriveClientSecret);
    }

    // �R�[���o�b�N�ݒ�
    authComponent->onAuthenticationComplete = [this](bool success, juce::String errorMessage,
        juce::String accessToken, juce::String refreshToken)
        {
            handleAuthenticationComplete(success, errorMessage, accessToken, refreshToken);
        };

    authComponent->onAuthenticationCancelled = [this]()
        {
            hideAuthenticationUI();
            setAuthStatus(AuthStatus::NotAuthenticated);
        };

    // �e�R���|�[�l���g�ɒǉ�
    if (parent != nullptr)
    {
        parent->addAndMakeVisible(*authComponent);
        authComponent->setBounds(parent->getLocalBounds());
    }

    // �F�؂��J�n
    authComponent->startAuthentication();
}

void R2CloudManager::hideAuthenticationUI()
{
    if (authComponent)
    {
        authComponent->stopAuthentication();
        
        if (parentForAuth)
        {
            parentForAuth->removeChildComponent(authComponent.get());
        }
        
        authComponent.reset();
        parentForAuth = nullptr;
    }
}

void R2CloudManager::handleAuthenticationComplete(bool success, const juce::String& errorMessage, const juce::String& accessToken, const juce::String& refreshToken)
{
    hideAuthenticationUI();
    
    if (success)
    {
        if (currentServiceType == ServiceType::GoogleDrive)
        {
            if (auto* googleProv = dynamic_cast<R2GoogleDriveProvider*>(googleDriveProvider.get()))
            {
                googleProv->setTokens(accessToken, refreshToken);
            }
        }
        else if (currentServiceType == ServiceType::OneDrive)
        {
            if (auto* oneDriveProv = dynamic_cast<R2OneDriveProvider*>(oneDriveProvider.get()))
            {
                oneDriveProv->setTokens(accessToken, refreshToken);
            }
        }
        setAuthStatus(AuthStatus::Authenticated);
    }
    else
    {
        setAuthStatus(AuthStatus::Error);
    }
    
    updateAuthStatus();
}

void R2CloudManager::signOut()
{
    auto provider = getCurrentProvider();
    if (provider)
    {
        provider->signOut();
        updateAuthStatus();
    }
    
    hideAuthenticationUI();
}

void R2CloudManager::cancelAuthentication()
{
    DBG("R2CloudManager::cancelAuthentication() called");
    
    // First, prevent any new callbacks from being triggered
    auto tempAuthCallback = onAuthStatusChanged;
    auto tempServiceCallback = onServiceChanged;
    
    // Clear callbacks immediately to prevent crashes
    onAuthStatusChanged = nullptr;
    onServiceChanged = nullptr;
    
    // Hide authentication UI
    hideAuthenticationUI();
    
    // Cancel authentication in providers
    auto provider = getCurrentProvider();
    if (provider != nullptr)
    {
        if (auto* googleProvider = dynamic_cast<R2GoogleDriveProvider*>(provider))
        {
            googleProvider->cancelAuthentication();
        }
        else if (auto* oneDriveProvider = dynamic_cast<R2OneDriveProvider*>(provider))
        {
            oneDriveProvider->cancelAuthentication();
        }
    }
    
    // Set status without triggering callbacks
    currentAuthStatus = AuthStatus::NotAuthenticated;
    
    // Give time for any pending async operations to complete
    juce::Thread::sleep(50);
    
    DBG("R2CloudManager::cancelAuthentication() completed");
}

void R2CloudManager::saveFile(const juce::String& filename, const juce::String& content, FileOperationCallback callback)
{
    auto provider = getCurrentProvider();
    if (!provider)
    {
        if (callback)
            callback(false, "No storage provider available");
        return;
    }
    
    if (needsAuthentication())
    {
        if (callback)
            callback(false, "Authentication required");
        return;
    }
    
    juce::MemoryBlock data(content.toUTF8(), content.getNumBytesAsUTF8());
    
    provider->uploadFile(filename, data, "root", [callback](bool success, juce::String errorMessage)
                         {
        if (callback)
            callback(success, errorMessage);
    });
}

void R2CloudManager::saveFileWithPath(const juce::String& folderPath, const juce::String& filename,
                                      const juce::String& content, FileOperationCallback callback)
{
    auto provider = getCurrentProvider();
    if (!provider)
    {
        if (callback) callback(false, "No storage provider available");
        return;
    }
    
    if (needsAuthentication())
    {
        if (callback) callback(false, "Authentication required");
        return;
    }
    
    juce::MemoryBlock data(content.toUTF8(), content.getNumBytesAsUTF8());
    
    juce::String fullPath = folderPath + "/" + filename;
    provider->uploadFile(fullPath, data, "path", [callback](bool success, juce::String errorMessage)
                         {
        if (callback) callback(success, errorMessage);
    });
}

void R2CloudManager::loadFile(const juce::String& filename, FileContentCallback callback)
{
    auto provider = getCurrentProvider();
    if (!provider)
    {
        if (callback)
            callback(false, "", "No storage provider available");
        return;
    }
    
    if (needsAuthentication())
    {
        if (callback)
            callback(false, "", "Authentication required");
        return;
    }
    
    if (auto* localProv = dynamic_cast<R2LocalStorageProvider*>(provider))
    {
        auto file = localStorageDir.getChildFile(filename);
        if (file.exists())
        {
            auto content = file.loadFileAsString();
            if (callback)
                callback(true, content, "");
        }
        else
        {
            if (callback)
                callback(false, "", "File not found: " + filename);
        }
        return;
    }
    
    if (filename.contains("/"))
    {
        loadFileWithPath(filename, callback);
    }
    else
    {
        provider->listFiles("root", [this, filename, callback, provider](bool success, juce::Array<R2CloudStorageProvider::FileInfo> files, juce::String errorMessage)
                            {
            if (!success)
            {
                if (callback)
                    callback(false, "", "Failed to list files: " + errorMessage);
                return;
            }
            
            juce::String fileId;
            for (const auto& file : files)
            {
                if (file.name == filename && !file.isFolder)
                {
                    fileId = file.id;
                    break;
                }
            }
            
            if (fileId.isEmpty())
            {
                if (callback)
                    callback(false, "", "File not found: " + filename);
                return;
            }
            
            provider->downloadFile(fileId, [callback, filename](bool downloadSuccess, juce::MemoryBlock data, juce::String downloadError)
                                   {
                if (downloadSuccess)
                {
                    auto content = juce::String::createStringFromData(data.getData(), static_cast<int>(data.getSize()));
                    if (callback)
                        callback(true, content, "");
                }
                else
                {
                    if (callback)
                        callback(false, "", "Failed to download file: " + downloadError);
                }
            });
        });
    }
}

void R2CloudManager::loadFileWithPath(const juce::String& filePath, FileContentCallback callback)
{
    auto provider = getCurrentProvider();
    if (!provider) return;
    
    if (auto* oneDriveProv = dynamic_cast<R2OneDriveProvider*>(provider))
    {
        oneDriveProv->downloadFileWithPath(filePath, [callback](bool success, juce::MemoryBlock data, juce::String error)
        {
            if (success)
            {
                auto content = juce::String::createStringFromData(data.getData(), static_cast<int>(data.getSize()));
                if (callback) callback(true, content, "");
            }
            else
            {
                if (callback) callback(false, "", error);
            }
        });
    }
    else if (auto* googleProv = dynamic_cast<R2GoogleDriveProvider*>(provider))
    {
        googleProv->downloadFileWithPath(filePath, [callback](bool success, juce::MemoryBlock data, juce::String error)
        {
            if (success)
            {
                auto content = juce::String::createStringFromData(data.getData(), static_cast<int>(data.getSize()));
                if (callback) callback(true, content, "");
            }
            else
            {
                if (callback) callback(false, "", error);
            }
        });
    }
}

} // namespace r2juce
