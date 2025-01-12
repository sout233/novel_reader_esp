#include "ReaderViewPort.h"
#include <Arduino.h>

ReaderViewPort::ReaderViewPort(XFont *xFont, TFT_eSPI *tft)
{
    _xFont = xFont;
    _tft = tft;
}

void ReaderViewPort::setLineNum(int ln)
{
    _lineNum = ln;
}

void ReaderViewPort::render(String text){
    pinMode(0, OUTPUT);
    _xFont->CleanSout();
    _tft->fillScreen(TFT_BLACK);
    _xFont->DrawChinese(0, 0, text, TFT_WHITE);

    _tft->setTextColor(TFT_WHITE, TFT_BLACK);
    _tft->setCursor(0, _tft->height() - 10);
    _tft->printf("ln:%d", _lineNum + 1);        
    pinMode(0, INPUT);
}
