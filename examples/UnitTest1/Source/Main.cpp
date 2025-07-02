/*
  ==============================================================================

    This file contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <JuceHeader.h>
#include "MainComponent.h"

// JUCEのユニットテスト機能をインクルードします
// #include "R2ComponentUnitTests.cpp" // この行は不要なため削除しました

//==============================================================================
class UnitTest1Application  : public juce::JUCEApplication
{
public:
    //==============================================================================
    UnitTest1Application() {}

    const juce::String getApplicationName() override       { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    //==============================================================================
    void initialise (const juce::String& commandLine) override
    {
        // アプリケーションの初期化コードをここに追加します。

        mainWindow.reset (new MainWindow (getApplicationName()));

        // ここでユニットテストを実行します。
        // アプリケーションの起動時にすべての登録済みユニットテストが実行され、
        // 結果はデバッグコンソールに出力されます。
        DBG("Application started and tests are running.");
        juce::UnitTestRunner runner;
        runner.runAllTests();
    }

    void shutdown() override
    {
        // アプリケーションのシャットダウンコードをここに追加します。

        mainWindow = nullptr; // (ウィンドウを削除します)
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        // アプリケーションが終了を要求されたときに呼び出されます。
        // この要求を無視してアプリケーションを継続して実行することも、
        // quit()を呼び出してアプリケーションを閉じさせることもできます。
        quit();
    }

    void anotherInstanceStarted (const juce::String& commandLine) override
    {
        // このインスタンスが実行中に、別のアプリケーションインスタンスが起動されたときに呼び出されます。
        // commandLineパラメータは、別のインスタンスのコマンドライン引数を示します。
        DBG("Application started and tests are running."); // これを追加
    }

    //==============================================================================
    /*
        このクラスは、MainComponentクラスのインスタンスを含むデスクトップウィンドウを実装します。
    */
    class MainWindow    : public juce::DocumentWindow
    {
    public:
        MainWindow (juce::String name)
            : DocumentWindow (name,
                              juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                          .findColour (juce::ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent(), true);

           #if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
           #else
            setResizable (true, true);
            centreWithSize (getWidth(), getHeight());
           #endif

            setVisible (true);
        }

        void closeButtonPressed() override
        {
            // ユーザーがこのウィンドウを閉じようとしたときに呼び出されます。
            // ここでは、アプリケーションに終了を要求するだけですが、
            // 必要に応じて変更できます。
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

        /* 注意: DocumentWindowメソッドをオーバーライドする場合は注意してください。
           基底クラスはそれらの多くを使用しているため、オーバーライドするとその機能が
           損なわれる可能性があります。すべての作業はコンテンツコンポーネントで行うのが
           最善ですが、どうしてもDocumentWindowメソッドをオーバーライドする必要がある場合は、
           サブクラスもスーパークラスのメソッドを呼び出すようにしてください。
        */

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
// このマクロは、アプリケーションを起動するmain()ルーチンを生成します。
START_JUCE_APPLICATION (UnitTest1Application)

