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
