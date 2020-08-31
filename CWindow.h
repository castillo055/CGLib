//
// Created by victor on 8/25/20.
//

#ifndef CNAVCOMP_CWINDOW_H
#define CNAVCOMP_CWINDOW_H

#include "CModules/CShape.h"
#include "Canvas.h"

class CWindow {
public:
    CWindow(Display* dpy, int x, int y, unsigned int w, unsigned int h);
    ~CWindow();

    void show();
    void hide();

    void redraw();

    void addShape(CModule* shape);

    void setBackground(char color[]);

    void captureMouse();
    void moveCursor(int dx, int dy);

    Canvas* canvas;
private:
    Display* dpy;
    Window w;
    int s;

    CModule* root;
};


#endif //CNAVCOMP_CWINDOW_H
