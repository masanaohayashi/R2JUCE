/*
  ==============================================================================
  
  Google OAuth & Microsoft OneDrive Credentials Configuration Template
  
  SETUP INSTRUCTIONS:
  1. Copy this file to "Credentials.h" (remove "_template")
  2. Replace the placeholder values with your actual OAuth credentials
  3. Follow the setup guide in README.md to get your credentials
  
  ==============================================================================
*/

#pragma once

// Google OAuth 2.0 Credentials
// Replace these with your actual credentials from Google Cloud Console
#define GOOGLE_CLIENT_ID "YOUR_GOOGLE_CLIENT_ID.apps.googleusercontent.com"
#define GOOGLE_CLIENT_SECRET "GOCSPX-YOUR_GOOGLE_CLIENT_SECRET"

// Microsoft OneDrive OAuth 2.0 Credentials
// Replace these with your actual credentials from Azure AD
#define ONEDRIVE_CLIENT_ID "YOUR_ONEDRIVE_CLIENT_ID"
#define ONEDRIVE_CLIENT_SECRET "YOUR_ONEDRIVE_CLIENT_SECRET"

// Apple iCloud Drive Container ID
// This is the identifier for your app's iCloud container.
#define ICLOUD_CONTAINER_ID "iCloud.com.yourcompany.yourapp"



/*
How to enable iCloud on macOS and iOS

1) Paste this into Custom PList for macOS and iOS exporter
<plist>
  <dict>
    <key>NSUbiquitousContainers</key>
    <dict>
        <key>iCloud.com.yourcompany.yourapp</key>
        <dict>
            <key>NSUbiquitousContainerIsDocumentScopePublic</key>
            <true/>
            <key>NSUbiquitousContainerSupportedFolderLevels</key>
            <string>Any</string>
            <key>NSUbiquitousContainerName</key>
            <string>$(PRODUCT_NAME)</string>
        </dict>
    </dict>
  </dict>
</plist>


2) Paste this into App Group ID for iOS exporter
group.com.yourcompany.yourapp
 
3) Paste this into Pre-Build Shell Script for macOS exporter

#!/bin/bash

# 対象となるエンタイトルメントファイルのリスト
# これらのファイル名はProjucerが勝手に決めているのでそれを使う
ENTITLEMENTS_FILES=(
    "$PROJECT_DIR/AUv3_AppExtension.entitlements"
    "$PROJECT_DIR/Standalone_Plugin.entitlements"
)

# チェックするキーと追加する値
KEY_TO_CHECK="com.apple.security.application-groups"
APP_GROUP="group.com.yourcompany.yourapp"
iCLOUD_KEY="com.apple.developer.icloud-container-identifiers"
iCLOUD_CONTAINER="iCloud.com.yourcompany.yourapp"
iCLOUD_SERVICES_KEY="com.apple.developer.icloud-services"
iCLOUD_SERVICES_VALUE="CloudDocuments"

# 各エンタイトルメントファイルを処理
for ENTITLEMENTS_FILE in "${ENTITLEMENTS_FILES[@]}"; do
    echo "Processing ${ENTITLEMENTS_FILE}..."

    # ファイルが存在しない場合はエラーを出力してスキップ
    if [ ! -f "$ENTITLEMENTS_FILE" ]; then
        echo "Error: Entitlements file not found: $ENTITLEMENTS_FILE"
        continue
    fi

    # iCloudの設定がない場合、iCloudコンテナ設定を追加
    if ! /usr/libexec/PlistBuddy -c "Print ${iCLOUD_KEY}" "$ENTITLEMENTS_FILE" &>/dev/null; then
        echo "Adding iCloud container ${iCLOUD_CONTAINER} to ${ENTITLEMENTS_FILE}"
        /usr/libexec/PlistBuddy -c "Add ${iCLOUD_KEY} array" "$ENTITLEMENTS_FILE"
        /usr/libexec/PlistBuddy -c "Add ${iCLOUD_KEY}:0 string ${iCLOUD_CONTAINER}" "$ENTITLEMENTS_FILE"
        if [ $? -ne 0 ]; then
            echo "Error: Failed to add iCloud container to $ENTITLEMENTS_FILE"
            continue
        fi
    else
        echo "iCloud container ${iCLOUD_CONTAINER} already exists in ${iCLOUD_KEY} in ${ENTITLEMENTS_FILE}"
    fi

    # iCloudサービスの設定がない場合、CloudDocumentsを追加
    if ! /usr/libexec/PlistBuddy -c "Print ${iCLOUD_SERVICES_KEY}" "$ENTITLEMENTS_FILE" &>/dev/null; then
        echo "Adding iCloud service ${iCLOUD_SERVICES_VALUE} to ${ENTITLEMENTS_FILE}"
        /usr/libexec/PlistBuddy -c "Add ${iCLOUD_SERVICES_KEY} array" "$ENTITLEMENTS_FILE"
        /usr/libexec/PlistBuddy -c "Add ${iCLOUD_SERVICES_KEY}:0 string ${iCLOUD_SERVICES_VALUE}" "$ENTITLEMENTS_FILE"
        if [ $? -ne 0 ]; then
            echo "Error: Failed to add iCloud service to $ENTITLEMENTS_FILE"
            continue
        fi
    else
        echo "iCloud service ${iCLOUD_SERVICES_VALUE} already exists in ${iCLOUD_SERVICES_KEY} in ${ENTITLEMENTS_FILE}"
    fi

    # キーが存在しない場合、新しい配列を追加
    if ! /usr/libexec/PlistBuddy -c "Print ${KEY_TO_CHECK}" "$ENTITLEMENTS_FILE" &>/dev/null; then
        echo "Adding ${KEY_TO_CHECK} as a new array to ${ENTITLEMENTS_FILE}"
        /usr/libexec/PlistBuddy -c "Add ${KEY_TO_CHECK} array" "$ENTITLEMENTS_FILE"
        if [ $? -ne 0 ]; then
            echo "Error: Failed to add ${KEY_TO_CHECK} as an array in $ENTITLEMENTS_FILE"
            continue
        fi
    fi

    # 配列内に値が存在しない場合、値を追加
    if ! /usr/libexec/PlistBuddy -c "Print ${KEY_TO_CHECK}" "$ENTITLEMENTS_FILE" | grep -q "$APP_GROUP"; then
        echo "Adding ${APP_GROUP} to ${KEY_TO_CHECK} in ${ENTITLEMENTS_FILE}"
        /usr/libexec/PlistBuddy -c "Add ${KEY_TO_CHECK}: string $APP_GROUP" "$ENTITLEMENTS_FILE"
        if [ $? -ne 0 ]; then
            echo "Error: Failed to add ${APP_GROUP} to ${KEY_TO_CHECK} in $ENTITLEMENTS_FILE"
            continue
        fi
    else
        echo "${APP_GROUP} already exists in ${KEY_TO_CHECK} in ${ENTITLEMENTS_FILE}"
    fi

    echo "Successfully updated ${ENTITLEMENTS_FILE}!"
done

echo "All entitlements files processed successfully!"
*/


/*
=== GOOGLE DRIVE SETUP ===

How to get Google credentials:
1. Go to Google Cloud Console: https://console.cloud.google.com/
2. Create a new project or select an existing one
3. Enable Google Drive API:
   - APIs & Services > Library > Search "Google Drive API" > Enable
4. Set up OAuth consent screen:
   - APIs & Services > OAuth consent screen
   - Choose "External" user type
   - Fill in required fields (App name, User support email, etc.)
   - Add yourself as a test user
5. Create OAuth 2.0 Client ID:
   - APIs & Services > Credentials
   - Create Credentials > OAuth 2.0 Client ID
   - Application type: Desktop application
   - Name: CloudDoc Desktop Client
6. Copy the generated Client ID and Client Secret

=== ONEDRIVE SETUP ===

How to get OneDrive credentials:
1. Go to Azure Portal: https://portal.azure.com/
2. Navigate to Azure Active Directory > App registrations
3. Click "New registration"
   - Name: CloudDoc Desktop Client
   - Supported account types: Personal Microsoft accounts only
   - Redirect URI: Leave empty (we'll use device flow authentication)
4. After registration, go to the app's Overview page
   - Copy the "Application (client) ID" - this is your ONEDRIVE_CLIENT_ID
5. Configure authentication (重要):
   - Go to "Authentication" in the left menu
   - Click "Add a platform" > "Mobile and desktop applications"
   - Check "https://login.microsoftonline.com/common/oauth2/nativeclient"
   - Under "Advanced settings", set "Allow public client flows" to "Yes"
   - Click "Save"
6. Create a client secret:
   - Go to "Certificates & secrets" in the left menu
   - Click "New client secret"
   - Description: CloudDoc Client Secret
   - Expires: 24 months (recommended)
   - Click "Add"
   - IMPORTANT: Copy the secret VALUE immediately (not the ID) - you won't see it again!
7. Add API permissions:
   - Go to "API permissions" in the left menu
   - Click "Add a permission"
   - Choose "Microsoft Graph"
   - Choose "Delegated permissions"
   - Search and add these permissions:
     * Files.ReadWrite - Access user's files
     * offline_access - Maintain access to data
     * User.Read - Sign in and read user profile (usually added by default)
   - Click "Add permissions"
   - Note: Admin consent is NOT required for personal accounts

=== EXAMPLE FORMAT ===

Google:
#define GOOGLE_CLIENT_ID "123456789012-abcdefghijklmnopqrstuvwxyz123456.apps.googleusercontent.com"
#define GOOGLE_CLIENT_SECRET "GOCSPX-AbCdEfGhIjKlMnOpQrStUvWxYz123"

OneDrive:
#define ONEDRIVE_CLIENT_ID "12345678-1234-1234-1234-123456789012"
#define ONEDRIVE_CLIENT_SECRET "AbC~1234567890AbCdEfGhIjKlMnOpQrSt"

=== SECURITY NOTES ===
- Never commit Credentials.h to version control
- Keep client secrets secure and private
- Do not share credentials with others
- Consider using environment variables in production
- OneDrive secrets expire - note the expiration date

=== TROUBLESHOOTING ===

If authentication fails:
- For OneDrive: Ensure "Allow public client flows" is set to "Yes"
- For Google: Make sure you've added yourself as a test user
- Check that all required permissions are granted
- Verify client ID and secret are copied correctly (no extra spaces)

For detailed setup instructions, see README.md in the project root.
*/
