//
// Created by victor on 8/24/20.
//

#ifndef CNAVCOMP_CANVAS_H
#define CNAVCOMP_CANVAS_H

#include <X11/Xft/Xft.h>

#define MAX(A, B)               ((A) > (B) ? (A) : (B))
#define MIN(A, B)               ((A) < (B) ? (A) : (B))
#define BETWEEN(X, A, B)        ((A) <= (X) && (X) <= (B))

typedef struct {
    Cursor cursor;
} Cur;

typedef struct Fnt {
    Display *dpy;
    unsigned int h;
    XftFont *xfont;
    FcPattern *pattern;
    struct Fnt *next;
} Fnt;

enum { ColFg, ColBg, ColBorder }; /* Clr scheme index */
typedef XftColor Clr;

class Canvas {
public:
    Display *dpy;
    int screen;

    Canvas(Display *dpy, int screen, Window win, unsigned int w, unsigned int h);
    ~Canvas();

    void resize(unsigned int w, unsigned int h);

    void rect(int x, int y, unsigned int w, unsigned int h, int filled, int invert);
    void line(int x1, int y1, int x2, int y2);
    void arc(int x, int y, unsigned int w, unsigned int h, int ang1, int ang2, int filled, int invert);
    int text(int x, int y, unsigned int w, unsigned int h, unsigned int lpad, const char *text, int invert);

    /* Drawing context manipulation */
    void setfontset(Fnt *set);
    void setscheme(Clr *scm);

    /* Map functions */
    void map(int x, int y, unsigned int w, unsigned int h);

    Clr *scm_create(char **clrnames, size_t clrcount);

    Fnt *fontset_create(char **fonts, size_t fontcount);
    static void fontset_free(Fnt* set);
    unsigned int fontset_getwidth(const char *text);

    unsigned int getWidth();
    unsigned int getHeight();
private:
    /* Fnt abstraction */
    void font_getexts(Fnt *font, const char *text, unsigned int len, unsigned int *w, unsigned int *h);

    /* Colorscheme abstraction */
    void clr_create(Clr *dest, const char *clrname);

    /* Cursor abstraction */
    Cur *cur_create(int shape);
    void cur_free(Cur *cursor);

    unsigned int w, h;
    Window root;
    Drawable drawable;
    GC gc;
    Clr *scheme;
    Fnt *fonts = nullptr;
};


#endif //CNAVCOMP_CANVAS_H
