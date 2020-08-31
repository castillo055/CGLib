//
// Created by victor on 8/26/20.
//

#ifndef CNAVCOMP_CSHAPE_H
#define CNAVCOMP_CSHAPE_H
#include <string>

#include "../Canvas.h"
#include "CModule.h"

class CShape : public CModule {
public:
    CShape(double x, double y, double w, double h, bool responsive);
    virtual ~CShape();

    virtual void setColor(std::string color) = 0;

    bool invert = false;
protected:
    std::string color1 = "#ffffff";
    std::string color2 = "#ff0000";

    void update_clrscheme(Canvas* cv);
    Clr* colorscheme = nullptr;
    std::string color1_cache, color2_cache;
};

class CRectangle : public CShape{
public:
    CRectangle(double x, double y, double w, double h, bool responsive);
    ~CRectangle();

    void setColor(std::string color) override;

    void draw(Canvas* cv) override;
    bool filled = true;
};

class CCircle : public CShape{
public:
    CCircle(double x, double y, double w, double h, bool responsive);
    ~CCircle();

    void setColor(std::string color) override;

    void draw(Canvas* cv) override;
    bool filled = true;
};

class CLine : public CShape{
public:
    CLine(double x, double y, double w, double h, bool responsive);
    ~CLine();

    void setColor(std::string color) override;

    void draw(Canvas* cv) override;
};

class CText : public CShape{
public:
    CText(std::string text, double x, double y, double w, double h, bool responsive);
    ~CText();

    void setColor(std::string color) override;

    void draw(Canvas* cv) override;

    unsigned int getTextWidth(Canvas* cv);

    std::string text;
    unsigned int lpad = 5;
    std::string font = "Iosevka Term:style=Bold:size=9:antialias=true:autohint:true";

private:
    void update_fonts(Canvas* cv);

    std::string font_cache;
    Fnt* fontset = nullptr;
};

#endif //CNAVCOMP_CSHAPE_H
