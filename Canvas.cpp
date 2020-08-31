//
// Created by victor on 8/24/20.
//
#include <X11/Xlib.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
//#include <fontconfig/fontconfig.h>

#include "Canvas.h"

#define UTF_INVALID 0xFFFD
#define UTF_SIZ     4

static const unsigned char utfbyte[UTF_SIZ + 1] = {0x80,    0, 0xC0, 0xE0, 0xF0};
static const unsigned char utfmask[UTF_SIZ + 1] = {0xC0, 0x80, 0xE0, 0xF0, 0xF8};
static const long utfmin[UTF_SIZ + 1] = {       0,    0,  0x80,  0x800,  0x10000};
static const long utfmax[UTF_SIZ + 1] = {0x10FFFF, 0x7F, 0x7FF, 0xFFFF, 0x10FFFF};

/* ===== STATIC FUNCTIONS ===== */
static long
utf8decodebyte(const char c, size_t *i)
{
    for (*i = 0; *i < (UTF_SIZ + 1); ++(*i))
        if (((unsigned char)c & utfmask[*i]) == utfbyte[*i])
            return (unsigned char)c & ~utfmask[*i];
    return 0;
}

static size_t
utf8validate(long *u, size_t i)
{
    if (!BETWEEN(*u, utfmin[i], utfmax[i]) || BETWEEN(*u, 0xD800, 0xDFFF))
        *u = UTF_INVALID;
    for (i = 1; *u > utfmax[i]; ++i)
        ;
    return i;
}

static size_t
utf8decode(const char *c, long *u, size_t clen)
{
    size_t i, j, len, type;
    long udecoded;

    *u = UTF_INVALID;
    if (!clen)
        return 0;
    udecoded = utf8decodebyte(c[0], &len);
    if (!BETWEEN(len, 1, UTF_SIZ))
        return 1;
    for (i = 1, j = 1; i < clen && j < len; ++i, ++j) {
        udecoded = (udecoded << 6) | utf8decodebyte(c[i], &type);
        if (type)
            return j;
    }
    if (j < len)
        return 0;
    *u = udecoded;
    utf8validate(u, len);

    return len;
}

/* This function is an implementation detail. Library users should use
 * fontset_create instead.
 */
static Fnt *
xfont_create(Canvas *drw, const char *fontname, FcPattern *fontpattern)
{
    Fnt *font;
    XftFont *xfont = NULL;
    FcPattern *pattern = NULL;

    if (fontname) {
        /* Using the pattern found at font->xfont->pattern does not yield the
         * same substitution results as using the pattern returned by
         * FcNameParse; using the latter results in the desired fallback
         * behaviour whereas the former just results in missing-character
         * rectangles being drawn, at least with some fonts. */
        if (!(xfont = XftFontOpenName(drw->dpy, drw->screen, fontname))) {
            fprintf(stderr, "error, cannot load font from name: '%s'\n", fontname);
            return NULL;
        }
        if (!(pattern = FcNameParse((FcChar8 *) fontname))) {
            fprintf(stderr, "error, cannot parse font name to pattern: '%s'\n", fontname);
            XftFontClose(drw->dpy, xfont);
            return NULL;
        }
    } else if (fontpattern) {
        if (!(xfont = XftFontOpenPattern(drw->dpy, fontpattern))) {
            fprintf(stderr, "error, cannot load font from pattern.\n");
            return NULL;
        }
    } else {
        //die("no font specified.");
        fprintf(stderr, "no font specified.");
        exit(1);
    }

    /* Do not allow using color fonts. This is a workaround for a BadLength
     * error from Xft with color glyphs. Modelled on the Xterm workaround. See
     * https://bugzilla.redhat.com/show_bug.cgi?id=1498269
     * https://lists.suckless.org/dev/1701/30932.html
     * https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=916349
     * and lots more all over the internet.
     */
    FcBool iscol;
    if(FcPatternGetBool(xfont->pattern, FC_COLOR, 0, &iscol) == FcResultMatch && iscol) {
        XftFontClose(drw->dpy, xfont);
        return NULL;
    }

    font = (Fnt*)calloc(1, sizeof(Fnt));
    font->xfont = xfont;
    font->pattern = pattern;
    font->h = xfont->ascent + xfont->descent;
    font->dpy = drw->dpy;

    return font;
}

static void
xfont_free(Fnt *font)
{
    if (!font)
        return;
    if (font->pattern)
        FcPatternDestroy(font->pattern);
    XftFontClose(font->dpy, font->xfont);
    free(font);
}

/* ===== CANVAS CLASS IMPLEMENTATION ===== */
Canvas::Canvas(Display *dpy, int screen, Window win, unsigned int w, unsigned int h) {
    this->dpy = dpy;
    this->screen = screen;
    this->root = win;
    this->w = w;
    this->h = h;

    drawable = XCreatePixmap(dpy, root, w, h, DefaultDepth(dpy, screen));
    gc = XCreateGC(dpy, root, 0, NULL);
    XSetLineAttributes(dpy, gc, 1, LineSolid, CapButt, JoinMiter);
}

Canvas::~Canvas() {
    XFreePixmap(dpy, drawable);
    XFreeGC(dpy, gc);
    if(fonts) fontset_free(fonts);
}

void Canvas::resize(unsigned int w, unsigned int h) {
    this->w = w;
    this->h = h;

    Drawable tmp = drawable;
    drawable = XCreatePixmap(dpy, root, w, h, DefaultDepth(dpy, screen));

    if(tmp) {
        XCopyArea(dpy, tmp, drawable, gc, 0, 0, w, h, 0, 0);
        XFreePixmap(dpy, tmp);
        XSync(dpy, False);
    }
}

void Canvas::rect(int x, int y, unsigned int w, unsigned int h, int filled, int invert) {
    if (!scheme)
        return;
    XSetForeground(dpy, gc, invert ? scheme[ColBg].pixel : scheme[ColFg].pixel);
    if (filled)
        XFillRectangle(dpy, drawable, gc, x, y, w, h);
    else
        XDrawRectangle(dpy, drawable, gc, x, y, w - 1, h - 1);
}

void Canvas::line(int x1, int y1, int x2, int y2) {
    if (!scheme)
        return;
    XSetForeground(dpy, gc, scheme[ColFg].pixel);

    XDrawLine(dpy, drawable, gc, x1, y1, x2, y2);
}

void Canvas::arc(int x, int y, unsigned int w, unsigned int h, int ang1, int ang2, int filled, int invert) {
    if (!scheme)
        return;
    XSetForeground(dpy, gc, invert ? scheme[ColBg].pixel : scheme[ColFg].pixel);
    if (filled)
        XFillArc(dpy, drawable, gc, x, y, w, h, ang1, ang2);
    else
        XDrawArc(dpy, drawable, gc, x, y, w, h, ang1, ang2);
}

int Canvas::text(int x, int y, unsigned int w, unsigned int h, unsigned int lpad, const char *text, int invert) {
    char buf[1024];
    int ty;
    unsigned int ew;
    XftDraw *d = NULL;
    Fnt *usedfont, *curfont, *nextfont;
    size_t i, len;
    int utf8strlen, utf8charlen, render = x || y || w || h;
    long utf8codepoint = 0;
    const char *utf8str;
    FcCharSet *fccharset;
    FcPattern *fcpattern;
    FcPattern *match;
    XftResult result;
    int charexists = 0;

    if ((render && !scheme) || !text || !fonts)
        return 0;

    if (!render) {
        w = ~w;
    } else {
        XSetForeground(dpy, gc, scheme[invert ? ColFg : ColBg].pixel);
        XFillRectangle(dpy, drawable, gc, x, y, w, h);
        d = XftDrawCreate(dpy, drawable,
                          DefaultVisual(dpy, screen),
                          DefaultColormap(dpy, screen));
        x += lpad;
        w -= lpad;
    }

    usedfont = fonts;
    while (1) {
        utf8strlen = 0;
        utf8str = text;
        nextfont = NULL;
        while (*text) {
            utf8charlen = utf8decode(text, &utf8codepoint, UTF_SIZ);
            for (curfont = fonts; curfont; curfont = curfont->next) {
                charexists = charexists || XftCharExists(dpy, curfont->xfont, utf8codepoint);
                if (charexists) {
                    if (curfont == usedfont) {
                        utf8strlen += utf8charlen;
                        text += utf8charlen;
                    } else {
                        nextfont = curfont;
                    }
                    break;
                }
            }

            if (!charexists || nextfont)
                break;
            else
                charexists = 0;
        }

        if (utf8strlen) {
            font_getexts(usedfont, utf8str, utf8strlen, &ew, NULL);
            /* shorten text if necessary */
            for (len = MIN(utf8strlen, sizeof(buf) - 1); len && ew > w; len--)
                font_getexts(usedfont, utf8str, len, &ew, NULL);

            if (len) {
                memcpy(buf, utf8str, len);
                buf[len] = '\0';
                if (len < utf8strlen)
                    for (i = len; i && i > len - 3; buf[--i] = '.')
                        ; /* NOP */

                if (render) {
                    ty = y + (h - usedfont->h) / 2 + usedfont->xfont->ascent;
                    XftDrawStringUtf8(d, &scheme[invert ? ColBg : ColFg],
                                      usedfont->xfont, x, ty, (XftChar8 *)buf, len);
                }
                x += ew;
                w -= ew;
            }
        }

        if (!*text) {
            break;
        } else if (nextfont) {
            charexists = 0;
            usedfont = nextfont;
        } else {
            /* Regardless of whether or not a fallback font is found, the
             * character must be drawn. */
            charexists = 1;

            fccharset = FcCharSetCreate();
            FcCharSetAddChar(fccharset, utf8codepoint);

            if (!fonts->pattern) {
                /* Refer to the comment in xfont_create for more information. */
                //die("the first font in the cache must be loaded from a font string.");
                fprintf(stderr, "the first font in the cache must be loaded from a font string.");
                exit(1);
            }

            fcpattern = FcPatternDuplicate(fonts->pattern);
            FcPatternAddCharSet(fcpattern, FC_CHARSET, fccharset);
            FcPatternAddBool(fcpattern, FC_SCALABLE, FcTrue);
            FcPatternAddBool(fcpattern, FC_COLOR, FcFalse);

            FcConfigSubstitute(NULL, fcpattern, FcMatchPattern);
            FcDefaultSubstitute(fcpattern);
            match = XftFontMatch(dpy, screen, fcpattern, &result);

            FcCharSetDestroy(fccharset);
            FcPatternDestroy(fcpattern);

            if (match) {
                usedfont = xfont_create(this, NULL, match);
                if (usedfont && XftCharExists(dpy, usedfont->xfont, utf8codepoint)) {
                    for (curfont = fonts; curfont->next; curfont = curfont->next)
                        ; /* NOP */
                    curfont->next = usedfont;
                } else {
                    xfont_free(usedfont);
                    usedfont = fonts;
                }
            }
        }
    }
    if (d)
        XftDrawDestroy(d);

    return x + (render ? w : 0);
}

void Canvas::setfontset(Fnt *set) {
    fonts = set;
}

void Canvas::setscheme(Clr *scm) {
    scheme = scm;
}

Fnt* Canvas::fontset_create(char **fonts, size_t fontcount) {
    Fnt *cur, *ret = nullptr;
    size_t i;

    if (!fonts)
        return nullptr;

    for (i = 1; i <= fontcount; i++) {
        if ((cur = xfont_create(this, fonts[fontcount - i], nullptr))) {
            cur->next = ret;
            ret = cur;
        }
    }
    return (this->fonts = ret);
}

void Canvas::fontset_free(Fnt *set) {
    if (set) {
        fontset_free(set->next);
        xfont_free(set);
    }
}

unsigned int Canvas::fontset_getwidth(const char *text) {
    if (!fonts || !text)
        return 0;
    return this->text(0, 0, 0, 0, 0, text, 0);
}

void Canvas::font_getexts(Fnt *font, const char *text, unsigned int len, unsigned int *w, unsigned int *h) {
    XGlyphInfo ext;

    if (!font || !text)
        return;

    XftTextExtentsUtf8(font->dpy, font->xfont, (XftChar8 *)text, len, &ext);
    if (w)
        *w = ext.xOff;
    if (h)
        *h = font->h;
}

void Canvas::clr_create(Clr *dest, const char *clrname) {
    if (!dest || !clrname)
        return;

    if (!XftColorAllocName(dpy, DefaultVisual(dpy, screen),
                           DefaultColormap(dpy, screen),
                           clrname, dest)) {
        fprintf(stderr, "error, cannot allocate color '%s'", clrname);
        exit(1);
    }
}

Clr *Canvas::scm_create(char **clrnames, size_t clrcount) {
    size_t i;
    Clr *ret;

    /* need at least two colors for a scheme */
    if (!clrnames || clrcount < 2 || !(ret = (XftColor*)calloc(clrcount, sizeof(XftColor))))
        return NULL;

    for (i = 0; i < clrcount; i++)
        clr_create(&ret[i], clrnames[i]);
    return ret;
}

Cur *Canvas::cur_create(int shape) {
    Cur *cur;

    if (!(cur = (Cur*)calloc(1, sizeof(Cur))))
        return NULL;

    cur->cursor = XCreateFontCursor(dpy, shape);

    return cur;
}

void Canvas::cur_free(Cur *cursor) {
    if (!cursor)
        return;

    XFreeCursor(dpy, cursor->cursor);
    free(cursor);
}

void Canvas::map(int x, int y, unsigned int w, unsigned int h) {
    XCopyArea(dpy, drawable, root, gc, x, y, w, h, x, y);
    XSync(dpy, False);
}

unsigned int Canvas::getWidth() {
    return w;
}

unsigned int Canvas::getHeight() {
    return h;
}
