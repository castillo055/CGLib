//
// Created by victor on 8/26/20.
//
#include <iostream>
#include "CShape.h"

CShape::CShape(double x, double y, double w, double h, bool responsive) : CModule(x, y, w, h, responsive) {

}
CShape::~CShape(){
    free(colorscheme);
}

void CShape::update_clrscheme(Canvas *cv) {
    if(!colorscheme || color1 != color1_cache || color2 != color2_cache) {
        color1_cache = color1;
        color2_cache = color2;

        if(colorscheme) free(colorscheme);
        char *clrs[] = {(char*)color1.c_str(),
                        (char*)color2.c_str()};
        colorscheme = cv->scm_create(clrs, 2);
    }
    cv->setscheme(colorscheme);
}

// ===== RECTANGLE CLASS IMPLEMENTATION =====
CRectangle::CRectangle(double x, double y, double w, double h, bool responsive) : CShape(x, y, w, h, responsive) {
}
CRectangle::~CRectangle(){}

void CRectangle::draw(Canvas *cv) {
    update_clrscheme(cv);

    cv->rect(responsivePos ? x * cv->getWidth() : x,
             responsivePos ? y * cv->getHeight() : y,
             responsiveSize ? w * cv->getWidth() : w,
             responsiveSize ? h * cv->getHeight() : h,
             filled, invert);
}

void CRectangle::setColor(std::string color) {
    color1 = color;
}

// ===== CIRCLE CLASS IMPLEMENTATION =====
CCircle::CCircle(double x, double y, double w, double h, bool responsive) : CShape(x, y, w, h, responsive) {
}
CCircle::~CCircle(){}

void CCircle::draw(Canvas *cv) {
    update_clrscheme(cv);

    cv->arc(responsivePos ? x * cv->getWidth() : x,
            responsivePos ? y * cv->getHeight() : y,
            responsiveSize ? w * cv->getWidth() : w,
            responsiveSize ? h * cv->getHeight() : h,
            0, 64 * 360, filled, invert);
}

void CCircle::setColor(std::string color) {
    color1 = color;
}

// ===== LINE CLASS IMPLEMENTATION =====
CLine::CLine(double x, double y, double w, double h, bool responsive) : CShape(x, y, w, h, responsive) {
}
CLine::~CLine(){}

void CLine::draw(Canvas *cv) {
    update_clrscheme(cv);

    cv->line(responsivePos ? x * cv->getWidth() : x,
             responsivePos ? y * cv->getHeight() : y,
             responsiveSize ? w * cv->getWidth() : w,
             responsiveSize ? h * cv->getHeight() : h);
}

void CLine::setColor(std::string color) {
    color1 = color;
}

// ===== TEXT CLASS IMPLEMENTATION =====
CText::CText(std::string text, double x, double y, double w, double h, bool responsive) : CShape(x, y, w, h, responsive), text(text) {
}
CText::~CText() {
    if(fontset) Canvas::fontset_free(fontset);
}

void CText::draw(Canvas *cv) {
    update_clrscheme(cv);
    update_fonts(cv);

    cv->text(responsivePos ? x * cv->getWidth() : x,
             responsivePos ? y * cv->getHeight() : y,
             responsiveSize ? w * cv->getWidth() : w,
             responsiveSize ? h * cv->getHeight() : h,
             lpad, text.c_str(), invert);

    cv->setfontset(nullptr);
}

void CText::setColor(std::string color) {
    color1 = color;
}

unsigned int CText::getTextWidth(Canvas *cv) {
    update_fonts(cv);
    unsigned int tectw = cv->fontset_getwidth(text.c_str());
    return tectw;
}

void CText::update_fonts(Canvas *cv) {
    if(!fontset || font != font_cache){
        if(fontset) Canvas::fontset_free(fontset);

        font_cache = font;

        char *fonts[] = {(char*)font.c_str()};
        fontset = cv->fontset_create(fonts, 1);
    }

    cv->setfontset(fontset);
}