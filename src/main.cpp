#include <Arduino.h>
#include <TFT_eSPI.h>
#include <EasyButton.h>
#include <LittleFS.h>
#include <xfont.h>

XFont *_xFont;
#define BUTTON_PIN 0
#define NOVELS_PATH "/novels/story.txt"

File storyFile;
int currentLine = 0;
TFT_eSPI tft = TFT_eSPI();
EasyButton flashButton(BUTTON_PIN);

bool openStoryFile()
{
    if (!storyFile)
    {
        storyFile = LittleFS.open(NOVELS_PATH, "r");
        if (!storyFile)
        {
            Serial.println("Failed to open file for reading");
            return false;
        }
    }
    return true;
}

void displayText(const String &text)
{
    pinMode(BUTTON_PIN, OUTPUT); // 设置GPIO模式为输出
    _xFont->CleanSout();
    tft.fillScreen(TFT_BLACK);
    _xFont->DrawChinese(0, 0, text, TFT_WHITE);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(0, tft.height() - 10);
    tft.printf("ln:%d", currentLine + 1);
    pinMode(BUTTON_PIN, INPUT); // 恢复GPIO模式为输入
}

String getNextLine()
{
    if (!openStoryFile())
        return "";
    String line = storyFile.readStringUntil('\n');
    if (line.length() == 0)
    {
        storyFile.seek(0, SeekSet);
        line = storyFile.readStringUntil('\n');
        currentLine = 0;
    }
    currentLine++;
    return line;
}

class IView
{
public:
    void setTFT(TFT_eSPI *tft)
    {
        _tft = tft;
    }
    void setXFont(XFont *xFont)
    {
        _xFont = xFont;
    }
    void setFlashButton(EasyButton *flashButton)
    {
        _flashButton = flashButton;
        _flashButton->begin();
        _flashButton->onPressed([this]()
                                { onPressed(); });
        _flashButton->onSequence(3, 500, [this]()
                                 { onTriplePressed(); });
    }
    void reInitXFont(const char *fontPath)
    {
        _xFont->reInitZhiku(fontPath);
    }
    virtual void onPressed() = 0;
    virtual void onTriplePressed() = 0;
    virtual void onHold() = 0;
    virtual void render() = 0;

protected:
    TFT_eSPI *_tft;
    XFont *_xFont;
    EasyButton *_flashButton;
};

class GotoView : public IView
{
public:
    void render() override
    {
        displayText(getNextLine());
    }
    void onPressed() override
    {
        displayText(getNextLine());
    }
    void onTriplePressed() override
    {
        if (!openStoryFile())
            return;

        for (int i = 0; i < 40; i++)
        {
            String line = storyFile.readStringUntil('\n');
            if (line.length() == 0)
            {
                storyFile.seek(0, SeekSet);
                currentLine = 0;
                break;
            }
            currentLine++;
        }

        String line = storyFile.readStringUntil('\n');
        displayText(line);
    }
    void onHold() override{}
};

class NovelView : public IView
{
public:
    void render() override
    {
        displayText(getNextLine());
    }
    void onPressed() override
    {
        displayText(getNextLine());
    }
    void onTriplePressed() override
    {
        if (!openStoryFile())
            return;

        for (int i = 0; i < 40; i++)
        {
            String line = storyFile.readStringUntil('\n');
            if (line.length() == 0)
            {
                storyFile.seek(0, SeekSet);
                currentLine = 0;
                break;
            }
            currentLine++;
        }

        String line = storyFile.readStringUntil('\n');
        displayText(line);
    }
    void onHold() override{
            if (!openStoryFile())
        return;

    if (currentLine > 1)
    {
        storyFile.seek(0, SeekSet);
        for (int i = 0; i < currentLine - 1; i++)
        {
            storyFile.readStringUntil('\n');
        }
        currentLine--;
    }
    else
    {
        storyFile.seek(0, SeekSet);
        currentLine = 0;
    }

    String line = storyFile.readStringUntil('\n');
    displayText(line);
    }
};

IView *currentView;

class ClockView : public IView
{
public:
    void render() override
    {
        _tft->fillScreen(TFT_BLACK);
        _tft->setTextColor(TFT_WHITE, TFT_BLACK);
        _tft->setCursor(0, 0);
        _tft->setTextSize(2);
        _tft->setRotation(1);
        _tft->printf("11:45");
    }
    void onPressed() override
    {
    }
    void onTriplePressed() override
    {
        currentView = new NovelView();
        currentView->setTFT(&tft);
        currentView->setXFont(_xFont);
        currentView->setFlashButton(&flashButton);
        currentView->render();
    }
    void onHold() override{}
};

void initDisplay()
{
    tft.init();

    currentView->setTFT(&tft);
    currentView->setXFont(_xFont);
    currentView->setFlashButton(&flashButton);
    currentView->render();
}

void onPressed()
{
    displayText(getNextLine());
}

void onPressedFor()
{
    if (!openStoryFile())
        return;

    if (currentLine > 1)
    {
        storyFile.seek(0, SeekSet);
        for (int i = 0; i < currentLine - 1; i++)
        {
            storyFile.readStringUntil('\n');
        }
        currentLine--;
    }
    else
    {
        storyFile.seek(0, SeekSet);
        currentLine = 0;
    }

    String line = storyFile.readStringUntil('\n');
    displayText(line);
}

void onDoublePressed()
{
    if (!openStoryFile())
        return;

    for (int i = 0; i < 40; i++)
    {
        String line = storyFile.readStringUntil('\n');
        if (line.length() == 0)
        {
            storyFile.seek(0, SeekSet);
            currentLine = 0;
            break;
        }
        currentLine++;
    }

    String line = storyFile.readStringUntil('\n');
    displayText(line);
}

void onLongPressedForTwoSeconds()
{
    if (!openStoryFile())
        return;

    int targetLine = currentLine > 50 ? currentLine - 50 : 0;
    storyFile.seek(0, SeekSet);

    for (int i = 0; i < targetLine; i++)
    {
        storyFile.readStringUntil('\n');
    }
    currentLine = targetLine;

    String line = storyFile.readStringUntil('\n');
    displayText(line);
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Init.");

    if (!LittleFS.begin())
    {
        Serial.println("LittleFS mount failed");
        return;
    }

    _xFont = new XFont(true);
    initDisplay();

    flashButton.begin();
    flashButton.onPressed(onPressed);
    flashButton.onPressedFor(500, onPressedFor);
    flashButton.onPressedFor(1000, onLongPressedForTwoSeconds);
    flashButton.onSequence(3, 500, onDoublePressed);

    pinMode(BUTTON_PIN, INPUT);

    _xFont->reInitZhiku("/x_simsun.ttc_12_b64.font");
    _xFont->DrawChinese(0, 0, "启动成功, 按下flash键可以激活系统。另外的, 白屏请重启...", TFT_WHITE);
}

void loop()
{
    flashButton.read();
}