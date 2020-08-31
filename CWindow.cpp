//
// Created by victor on 8/25/20.
//
#include <X11/Xlib.h>
#include <iostream>
#include "CWindow.h"

CWindow::CWindow(Display *dpy, int x, int y, unsigned int w, unsigned int h) {
    this->dpy = dpy;

    s = DefaultScreen(dpy);
    this->w = XCreateSimpleWindow(dpy, RootWindow(dpy, s), x, y, w, h, 0, BlackPixel(dpy, s), 0xaaaaaa);//0x06060a);

    XSelectInput(dpy, this->w, ExposureMask | KeyPressMask | PointerMotionMask | ButtonPressMask);

    canvas = new Canvas(this ->dpy, s, this->w, w, h);

    root = new CRectangle(0, 0, 1, 1, true);

    show();
}
CWindow::~CWindow() {
    CModule* shape;
    while(root->next) {
        for (shape = root; shape->next->next; shape = shape->next);
        delete shape->next;
        shape->next = nullptr;
    }
    delete root;
    delete canvas;
    XUngrabPointer(dpy, CurrentTime);
}

void CWindow::show() {
    XMapWindow(dpy, w);
}

void CWindow::hide() {
    XUnmapWindow(dpy, w);
}

void CWindow::redraw() {
    XWindowAttributes xa;
    if(!XGetWindowAttributes(dpy, w, &xa)) return;
    canvas->resize(xa.width, xa.height);

    root->draw(canvas);

    CModule* shape = root->next;
    while(shape){
        shape->draw(canvas);
        shape = shape->next;
    }

    canvas->map(0, 0, xa.width, xa.height);
}

void CWindow::addShape(CModule *shape) {
    CModule* m;
    for(m = root; m->next; m = m->next);
    m->next = shape;
}

void CWindow::setBackground(char *color) {
    ((CShape*)root)->setColor(color);
}

void CWindow::captureMouse(){
    XGrabPointer(dpy, this->w, false, PointerMotionMask, GrabModeAsync, GrabModeAsync, this->w, None, CurrentTime);
}

XEvent tmp;
void CWindow::moveCursor(int dx, int dy) {
    XWindowAttributes xa;
    if(!XGetWindowAttributes(dpy, w, &xa)) return;
    XWarpPointer(dpy, None,  None, 0, 0, 0, 0, dx, dy);
}
