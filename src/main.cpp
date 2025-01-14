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
int prevLine = 0;
TFT_eSPI tft = TFT_eSPI();
EasyButton flashButton(BUTTON_PIN);

enum ViewType
{
    CLOCK,
    NOVEL,
    GOTO_LINE,
};

ViewType currentView = CLOCK;

void initDisplay()
{
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);
}

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
    Serial.println("currentLine: " + String(currentLine));
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

void renderClock()
{
    pinMode(BUTTON_PIN, OUTPUT);
    Serial.println("renderClock");
    currentView = CLOCK;
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextSize(4);
    tft.print("11:45");
    tft.setTextSize(1);
    pinMode(BUTTON_PIN, INPUT);
}

void renderNovel()
{
    currentView = NOVEL;
    String line = getNextLine();
    displayText(line);
}

int gotoLineViewCurrentSelection = 0;
int gotoLineViewInput = 0;

void renderGotoLine()
{
    pinMode(BUTTON_PIN, OUTPUT);
    Serial.println("renderGotoLine");
    currentView = GOTO_LINE;
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    // header
    tft.setCursor(0, 0);
    tft.setTextSize(1);
    tft.print("goto:");
    tft.setTextSize(2);
    tft.setCursor(0, 20);
    tft.print(gotoLineViewInput);

    // keyboard
    tft.setCursor(0, 40);
    tft.setTextSize(1);
    String keys[14] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "DEL", "AUTO", "BACK", "GO"};
    for (int i = 0; i < 14; i++)
    {
        String key_str = keys[i];
        if (i == gotoLineViewCurrentSelection)
        {
            tft.setTextColor(TFT_WHITE, TFT_BLUE);
        }
        else if (i == 13)
        {
            tft.setTextColor(TFT_RED, TFT_WHITE);
        }
        else if (i == 14)
        {
            tft.setTextColor(TFT_GREEN, TFT_WHITE);
        }
        else
        {
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
        }

        tft.println(keys[i]);
    }
    pinMode(BUTTON_PIN, INPUT);
}

void onPressed()
{
    switch (currentView)
    {
    case CLOCK:
        break;
    case NOVEL:
        displayText(getNextLine());
        break;
    case GOTO_LINE:
        gotoLineViewCurrentSelection++;
        if (gotoLineViewCurrentSelection > 13)
        {
            gotoLineViewCurrentSelection = 0;
        }
        renderGotoLine();
        Serial.println(gotoLineViewCurrentSelection);
        break;
    default:
        break;
    }
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

void onTriplePressed()
{
    switch (currentView)
    {
    case CLOCK:
        break;
    case NOVEL:
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
        break;
    }
    case GOTO_LINE:
        // int targetLine = currentLine > 50 ? currentLine - 50 : 0;
        // storyFile.seek(0, SeekSet);

        // for (int i = 0; i < targetLine; i++)
        // {
        //     storyFile.readStringUntil('\n');
        // }
        // currentLine = targetLine;

        // renderNovel();
        break;
    }
}

void onLongPressedForTwoSeconds()
{
    if (!openStoryFile())
        return;

    switch (currentView)
    {
    case CLOCK:
        renderNovel();
        break;
    case NOVEL:
        gotoLineViewInput = currentLine + 1;
        renderGotoLine();
        break;
    case GOTO_LINE:
    {
        Serial.println("gotoLineViewInput: " + String(gotoLineViewInput));
        // BACK
        if (gotoLineViewCurrentSelection == 12)
        {
            currentLine-=2;
            renderNovel();
        }
        // GO
        else if (gotoLineViewCurrentSelection == 13)
        {
            currentLine = gotoLineViewInput - 2;
            renderNovel();
        }
        // DEL
        else if (gotoLineViewCurrentSelection == 10)
        {
            gotoLineViewInput /= 10;
            renderGotoLine();
        }
        // AUTO
        else if (gotoLineViewCurrentSelection == 11)
        {
            gotoLineViewInput = currentLine + 1;
            renderGotoLine();
        }
        // numbers
        else
        {
            gotoLineViewInput = gotoLineViewInput * 10 + (gotoLineViewCurrentSelection + 1);
            renderGotoLine();
        }
        break;
    }
    }
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
    flashButton.onSequence(3, 500, onTriplePressed);

    pinMode(BUTTON_PIN, INPUT);

    _xFont->reInitZhiku("/x_simsun.ttc_12_b64.font");
    // tft.setRotation(0);
    // tft.fillScreen(TFT_BLACK);
    // tft.setTextColor(TFT_WHITE, TFT_BLACK);
    // tft.setCursor(0, 0);
    // tft.setTextSize(4);
    // tft.print("11:45");
    // _xFont->DrawChinese(0, 0, "启动成功, 按下flash键可以激活系统。另外的, 白屏请重启...", TFT_WHITE);
    renderClock();
}

void loop()
{
    flashButton.read();
}