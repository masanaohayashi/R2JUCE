/*
  ==============================================================================
  
  Google OAuth Credentials Configuration Template
  
  SETUP INSTRUCTIONS:
  1. Copy this file to "Credentials.h" (remove "_template")
  2. Replace the placeholder values with your actual Google OAuth credentials
  3. Follow the setup guide in README.md to get your credentials
  
  ==============================================================================
*/

#pragma once

// Google OAuth 2.0 Credentials
// Replace these with your actual credentials from Google Cloud Console
#define GOOGLE_CLIENT_ID "YOUR_ACTUAL_CLIENT_ID.apps.googleusercontent.com"
#define GOOGLE_CLIENT_SECRET "GOCSPX-YOUR_ACTUAL_CLIENT_SECRET"

/*
How to get these credentials:

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

Example format:
#define GOOGLE_CLIENT_ID "123456789012-abcdefghijklmnopqrstuvwxyz123456.apps.googleusercontent.com"
#define GOOGLE_CLIENT_SECRET "GOCSPX-AbCdEfGhIjKlMnOpQrStUvWxYz123"

For detailed setup instructions, see README.md in the project root.
*/
