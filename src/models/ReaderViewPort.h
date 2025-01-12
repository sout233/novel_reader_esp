#include <TFT_eSPI.h>
#include <xfont.h>

class ReaderViewPort
{
public:
    ReaderViewPort(XFont *xFont, TFT_eSPI *tft);

    void setLineNum(int lineNum);
    void render(String text);

private:
    int _lineNum;
    XFont *_xFont;
    TFT_eSPI *_tft;
};