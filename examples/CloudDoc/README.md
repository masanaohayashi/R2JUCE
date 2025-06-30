# CloudDoc: R2JUCEクラウド連携サンプルアプリケーション

English follows Japanese

CloudDocは、JUCEフレームワークとカスタムモジュールR2JUCEを使用して、クラウドストレージ（Google Drive、OneDrive）およびローカルストレージとの間でファイルを操作するサンプルアプリケーションです。このアプリケーションは、R2JUCEが提供するクラウド連携機能を実演します。

## 主な機能

* **クラウドサービス選択**: Google Drive、Microsoft OneDrive、およびローカルストレージ間で切り替えてファイル操作を行うことができます。
* **OAuth 2.0デバイス認証フロー**: Google DriveおよびOneDriveへの初回アクセス時には、OAuth 2.0デバイス認証フローを介したユーザー認証が必要です。このプロセスはアプリケーション内で完結し、ユーザーは表示されるURLにアクセスし、コードを入力することで認証を完了できます。この認証方式は、通常、TV用アプリなど、キーボード入力が困難なデバイスで用いられます。そのため、Raspberry Piなどのデバイスで動作するCloudDocアプリケーションを認証する際にも、スマートフォンやMac/PCを使用して認証を行うことが可能です。
* **ファイル操作**: 画面内のテキストエディタに表示されたテキストデータをクラウドまたはローカルに保存したり、読み込んだりすることができます。
* **ドラッグ＆ドロップによるファイルアップロード**: アプリケーションの指定されたエリアにファイルをドラッグ＆ドロップすることで、その内容を読み込み、現在選択されているクラウドサービスにアップロードできます。この機能は主に画像や音声などのファイルに対応しています。フォルダのアップロードはサポートされておらず、ファイルサイズは10MBに制限されています。
* **設定の永続化**: 最後に選択したクラウドサービス、ファイルパス、ファイル名がアプリケーションの終了時に保存され、次回起動時に自動的に復元されます。
* **カスタムUIコンポーネント**: 進捗表示付きアラートや、ソフトウェアキーボード対応（Raspberry Pi向け）のテキストエディタなど、R2JUCEが提供するカスタムUIコンポーネントを使用しています。

## はじめに

このプロジェクトをビルドして実行するには、JUCEフレームワークの基本的な知識と、C++開発環境がセットアップされている必要があります。

### Google Drive / OneDrive 認証情報の取得

Google DriveおよびOneDriveと連携するためには、ご自身のクラウドプラットフォームでアプリケーションを登録し、**クライアントID**と**クライアントシークレット**を取得する必要があります。これらの情報は機密性が高いため、公開リポジトリにコミットしないように注意してください。

#### Google Driveの場合:

1.  [Google Cloud Console](https://console.cloud.google.com/)にアクセスします。
2.  新規プロジェクトを作成するか、既存のプロジェクトを選択します。
3.  「APIとサービス」>「ライブラリ」から「**Google Drive API**」を有効にします。
4.  「APIとサービス」>「認証情報」に移動します。
5.  「認証情報を作成」をクリックし、「OAuth クライアント ID」を選択します。
6.  アプリケーションの種類として「**デスクトップ アプリケーション**」を選択し、クライアントを作成します。
7.  作成されたクライアントの「**クライアント ID**」と「**クライアント シークレット**」をメモします。

#### Microsoft OneDriveの場合:

1.  [Azure Portal](https://portal.azure.com/)にアクセスし、サインインします。
2.  「Azure Active Directory」>「アプリの登録」に移動します。
3.  「新規登録」をクリックし、以下の情報を入力します。
    * **名前**: `CloudDoc Desktop Client` (任意の名前)
    * **サポートされているアカウントの種類**: 「個人用 Microsoft アカウントのみ」を選択します。
    * **リダイレクト URI**: ここは**空欄**のまま進めてください。デバイス認証フローではリダイレクトURIは使用しません。
4.  アプリの登録後、左メニューの「証明書とシークレット」に移動し、「**新しいクライアント シークレット**」を作成します。**表示される値（シークレット）を必ずすぐにメモしてください。この値は一度しか表示されず、後から確認することはできません。メモを忘れた場合は、シークレットを削除して再作成する必要があります。**
5.  左メニューの「API のアクセス許可」に移動し、「アクセス許可の追加」をクリックします。
6.  「Microsoft Graph」を選択し、「委任されたアクセス許可」タブで以下のアクセス許可を追加します。
    * `Files.ReadWrite`
    * `offline_access`
7.  追加後、「管理者の同意を与えます」をクリックします。

### 認証情報の設定

取得したクライアントIDとクライアントシークレットは、`Source/Credentials_template.h` ファイルを `Source/Credentials.h` にリネームし、そのファイルに定義されている以下のマクロを置き換えることで設定します。

**警告**: `Credentials.h` ファイルは機密情報を含むため、バージョン管理システム（Gitなど）にコミットしないでください。

## ビルドと実行

1.  Projucerでプロジェクトファイル（`.jucer`）を開きます。
2.  適切なエクスポートターゲット（Visual Studio, Xcode, Makefileなど）を選択し、IDEプロジェクトを生成します。
3.  生成されたプロジェクトをIDE（Visual Studio, Xcodeなど）で開き、ビルドして実行します。

アプリケーションが起動したら、ドロップダウンメニューからクラウドサービスを選択し、ファイル操作を試すことができます。


## CloudDoc アプリケーションの Google Drive および OneDrive API を使ったファイル読み書きの詳細（ソースコード引用付き）

CloudDoc アプリケーションは、`R2CloudManager` クラスを介して抽象化された `R2CloudStorageProvider` インターフェースを利用し、具体的なクラウドサービスプロバイダ（`R2GoogleDriveProvider`、`R2OneDriveProvider`）がそれぞれの API と連携しています。

### 認証方式 (OAuth 2.0 Device Authorization Flow)

Google Drive と OneDrive の両方で、OAuth 2.0 Device Authorization Flow が使用されています。これは、Web ブラウザを持たないデバイス（例: Raspberry Pi）から認証を行う場合に適したフローです。

1. デバイスコードのリクエスト:
- アプリは、選択されたサービス（Google DriveまたはOneDrive）に応じて、それぞれの認証エンドポイントにデバイスコードをリクエストします。この処理は R2CloudAuthComponent::requestDeviceCode() メソッドで行われます。

- Google Drive: https://oauth2.googleapis.com/device/code

-- client_id と scope (https://www.googleapis.com/auth/drive.file) を POST データとして送信します。


---

# CloudDoc: R2JUCE Cloud Integration Sample Application

English follows Japanese

CloudDoc is a sample application that demonstrates how to interact with cloud storage (Google Drive, OneDrive) and local storage for file operations, built using the JUCE framework and the custom R2JUCE module. This application showcases R2JUCE's cloud integration features.

## Key Features

* **Cloud Service Selection**: Switch between Google Drive, Microsoft OneDrive, and local storage for file operations.
* **OAuth 2.0 Device Authorization Flow**: Initial access to Google Drive and OneDrive requires user authentication via the OAuth 2.0 Device Authorization Flow. This process is handled within the application, allowing users to complete authentication by visiting a displayed URL and entering a code. This authentication method is typically used for devices like smart TVs where direct keyboard input is difficult. Therefore, when authenticating the CloudDoc application running on devices such as Raspberry Pi, it is possible to perform authentication using a smartphone or a Mac/PC.
* **File Operations**: Save and load text data displayed in the on-screen text editor to/from cloud or local storage.
* **Drag & Drop File Upload**: Drag and drop files onto a designated area within the application to read their content and upload them to the currently selected cloud service. This feature primarily supports files such as images and audio. Folder uploads are not supported, and file size is limited to 10MB.
* **Settings Persistence**: The last selected cloud service, file path, and file name are saved upon application exit and automatically restored on the next launch.
* **Custom UI Components**: Utilizes R2JUCE's custom UI components, including an alert with a progress bar and a text editor with on-screen keyboard support (for Raspberry Pi).

## Getting Started

To build and run this project, you need a basic understanding of the JUCE framework and a C++ development environment set up.

### Obtaining Google Drive / OneDrive Credentials

To integrate with Google Drive and OneDrive, you must register your application with their respective cloud platforms and obtain your **Client ID** and **Client Secret**. These are sensitive information and should not be committed to public repositories.

#### For Google Drive:

1.  Go to [Google Cloud Console](https://console.cloud.google.com/).
2.  Create a new project or select an existing one.
3.  Enable the "**Google Drive API**" under "APIs & Services" > "Library".
4.  Navigate to "APIs & Services" > "Credentials".
5.  Click "Create Credentials" and select "OAuth client ID".
6.  Choose "**Desktop application**" as the application type and create the client.
7.  Make a note of the "**Client ID**" and "**Client Secret**" displayed.

#### For Microsoft OneDrive:

1.  Go to [Azure Portal](https://portal.azure.com/) and sign in.
2.  Navigate to "Azure Active Directory" > "App registrations".
3.  Click "New registration" and fill in the following details:
    * **Name**: `CloudDoc Desktop Client` (or any name you prefer)
    * **Supported account types**: Select "Personal Microsoft accounts only".
    * **Redirect URI**: Leave this **blank**. Redirect URIs are not used for device authorization flow.
4.  After registering the app, go to "Certificates & secrets" in the left menu and create a "**New client secret**". **Immediately copy the Value (secret) that is displayed. This value is shown only once and cannot be retrieved later. If you forget to copy it, you will need to delete the secret and create a new one.**
5.  Go to "API permissions" in the left menu and click "Add a permission".
6.  Select "Microsoft Graph" and add the following delegated permissions:
    * `Files.ReadWrite`
    * `offline_access`
7.  After adding, click "Grant admin consent".

### Setting up Credentials

The obtained Client ID and Client Secret should be set by renaming the `Source/Credentials_template.h` file to `Source/Credentials.h` and then replacing the following macros defined in that file:

**WARNING**: `Credentials.h` file contains sensitive information and should **NOT** be committed to version control systems (like Git).

## Build and Run

1.  Open the project file (`.jucer`) with Projucer.
2.  Select the appropriate export target (Visual Studio, Xcode, Makefile, etc.) and generate the IDE project.
3.  Open the generated project in your IDE (Visual Studio, Xcode, etc.), then build and run the application.

Once the application starts, you can select a cloud service from the dropdown menu and try out the file operations.
