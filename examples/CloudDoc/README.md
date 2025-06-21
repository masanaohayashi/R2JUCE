# Google Drive OAuth認証設定ガイド

このガイドでは、CloudDocアプリケーションでGoogle Drive認証を使用するために必要なGoogle Cloud Console設定手順を説明します。

## 📋 目次

- [事前準備](#事前準備)
- [Google Cloud Consoleでの設定](#google-cloud-consoleでの設定)
- [OAuth同意画面の設定](#oauth同意画面の設定)
- [OAuth Client IDの作成](#oauth-client-idの作成)
- [認証情報の取得](#認証情報の取得)
- [アプリケーションへの設定](#アプリケーションへの設定)
- [テストモードと本番環境](#テストモードと本番環境)
- [トラブルシューティング](#トラブルシューティング)

## 🚀 事前準備

### 必要なもの
- Googleアカウント
- ブラウザ（Chrome、Firefox、Safari等）
- インターネット接続

### 注意事項
- Google Drive APIは**無料**で使用できます（月間1億回のAPIコールまで）
- 設定には約10-15分かかります

## 🔧 Google Cloud Consoleでの設定

### 1. Google Cloud Consoleにアクセス

1. [Google Cloud Console](https://console.cloud.google.com/) にアクセス
2. Googleアカウントでサインイン

### 2. プロジェクトの作成

1. 画面上部の「**プロジェクトの選択**」をクリック
2. 「**新しいプロジェクト**」をクリック
3. プロジェクト名を入力（例：`CloudDoc-App`）
4. 「**作成**」をクリック
5. 作成されたプロジェクトが選択されていることを確認

### 3. Google Drive APIの有効化

1. 左サイドバーの「**APIs & Services**」→「**Library**」をクリック
2. 検索ボックスに「**Google Drive API**」と入力
3. 「**Google Drive API**」をクリック
4. 「**有効にする**」ボタンをクリック

## 🔐 OAuth同意画面の設定

### 1. OAuth同意画面の設定

1. 左サイドバーの「**APIs & Services**」→「**OAuth consent screen**」をクリック
2. 「**External**」を選択（個人用アカウントの場合）
3. 「**作成**」をクリック

### 2. アプリ情報の入力

**必須項目：**
- **App name**: `CloudDoc`（または任意のアプリ名）
- **User support email**: あなたのGmailアドレス
- **Developer contact information**: あなたのGmailアドレス

**オプション項目：**
- **App logo**: アプリのロゴ（120x120px、推奨）
- **App domain**: 空欄でOK
- **App privacy policy link**: 空欄でOK
- **App terms of service link**: 空欄でOK

### 3. スコープの設定

1. 「**保存して次へ**」をクリック
2. 「**Scopes**」ページでは何も設定せず「**保存して次へ**」をクリック

### 4. テストユーザーの追加

1. 「**Test users**」ページで「**+ ADD USERS**」をクリック
2. **あなたのGmailアドレス**を入力
3. 「**保存**」をクリック
4. 「**保存して次へ**」をクリック

### 5. 概要の確認

1. 設定内容を確認
2. 「**ダッシュボードに戻る**」をクリック

## 🔑 OAuth Client IDの作成

### 1. 認証情報の作成

1. 左サイドバーの「**APIs & Services**」→「**Credentials**」をクリック
2. 上部の「**+ 認証情報を作成**」→「**OAuth 2.0 クライアント ID**」をクリック

### 2. アプリケーションタイプの選択

1. 「**Application type**」で「**Desktop application**」を選択
2. 「**Name**」に `CloudDoc Desktop Client`（または任意の名前）を入力
3. 「**作成**」をクリック

## 📝 認証情報の取得

### Client IDとSecretの確認

作成完了後、以下の情報が表示されます：

```
Client ID: 123456789012-xxxxxxxxxxxxxxxxxxxxxxxx.apps.googleusercontent.com
Client Secret: GOCSPX-xxxxxxxxxxxxxxxxxxxxxx
```

**重要：** これらの情報は**安全に保管**してください。

### JSONファイルのダウンロード（オプション）

1. 「**JSON をダウンロード**」をクリック
2. ファイルを安全な場所に保存

## 🔧 アプリケーションへの設定

### MainComponent.hでの設定

`MainComponent.h`ファイルの以下の部分を編集：

```cpp
// Google OAuth認証情報（取得した値に置き換え）
const juce::String googleClientId = "あなたのClient ID";
const juce::String googleClientSecret = "あなたのClient Secret";
```

### 設定例

```cpp
const juce::String googleClientId = "123456789012-abcdefghijklmnopqrstuvwxyz123456.apps.googleusercontent.com";
const juce::String googleClientSecret = "GOCSPX-AbCdEfGhIjKlMnOpQrStUvWxYz";
```

## 🔄 テストモードと本番環境

### テストモード（開発用）

**設定状態：**
- Publishing status: **Testing**
- Test users: 開発者とテストユーザーのみ

**特徴：**
- 警告なしでアプリを使用可能
- テストユーザーのみアクセス可能
- 設定変更が即座に反映

**推奨用途：**
- 開発とテスト
- 小規模な内部使用
- プロトタイプ

### 本番環境（公開用）

**設定手順：**
1. OAuth consent screen → **PUBLISH APP**をクリック
2. App Verificationプロセスの開始（必要に応じて）

**注意事項：**
- `https://www.googleapis.com/auth/drive.file`スコープは通常、App Verification不要
- 全フォルダアクセス（`drive`スコープ）は審査が必要

### モード切り替え

**テスト → 本番：**
1. OAuth consent screen
2. 「**PUBLISH APP**」をクリック
3. 確認ダイアログで「**CONFIRM**」

**本番 → テスト：**
1. OAuth consent screen  
2. 「**BACK TO TESTING**」をクリック

## 🛠️ トラブルシューティング

### よくある問題と解決方法

#### 1. "このアプリは Google で確認されていません"
**原因：** アプリがテストモードでテストユーザーに追加されていない  
**解決方法：** OAuth consent screen → Test users に自分を追加

#### 2. "無効なクライアント"エラー  
**原因：** Client IDが間違っている  
**解決方法：** MainComponent.hのClient IDを確認

#### 3. "認証に失敗しました"
**原因：** Client Secretが間違っている  
**解決方法：** MainComponent.hのClient Secretを確認

#### 4. "スコープが無効です"エラー
**原因：** 使用しているスコープがサポートされていない  
**解決方法：** `drive.file`スコープを使用（現在の実装で対応済み）

### デバッグ方法

1. **コンソール出力を確認**
   ```
   DBG("Device code response - Success: true/false");
   DBG("Response content: ...");
   ```

2. **Google Cloud Consoleで使用状況を確認**
   - APIs & Services → Dashboard
   - 使用統計とエラーログを確認

3. **認証情報の再作成**
   - 問題が解決しない場合は新しいOAuth Client IDを作成

## 📊 使用量とクォータ

### 無料枠
- **1日**: 1,000,000 リクエスト
- **月間**: 100,000,000 リクエスト

### 一般的な使用量
- ファイル1個の保存/読み込み: 2-3 リクエスト
- 個人使用: 月間数千リクエスト程度

### クォータ監視
1. Google Cloud Console → APIs & Services → Dashboard
2. Google Drive APIの使用量を確認

## 🔒 セキュリティのベストプラクティス

### 認証情報の管理
- Client SecretをGitにコミットしない
- 本番環境では環境変数を使用を検討
- 定期的にClient Secretをローテーション

### アクセス権限
- 最小権限の原則（`drive.file`スコープのみ）
- 不要になったOAuth Clientは削除

## 📞 サポート

### Googleサポート
- [Google Cloud Support](https://cloud.google.com/support)
- [OAuth 2.0 ドキュメント](https://developers.google.com/identity/protocols/oauth2)

### このプロジェクトのサポート
- GitHubのIssuesページで質問・バグ報告
- READMEの更新提案はPull Requestで

---

## ✅ チェックリスト

設定完了前に以下を確認してください：

- [ ] Google Cloud Consoleでプロジェクトを作成
- [ ] Google Drive APIを有効化
- [ ] OAuth同意画面を設定
- [ ] テストユーザーに自分を追加
- [ ] OAuth Client IDを作成（Desktop application）
- [ ] Client IDとSecretを取得
- [ ] MainComponent.hに認証情報を設定
- [ ] アプリケーションをビルド・テスト

すべて完了したら、CloudDocアプリでGoogle Drive認証が使用できます！