//
// Created by victor on 8/26/20.
//
#include <iostream>
#include <cmath>

#include "CModule.h"
#include "CShape.h"

#define RADTODEG(X)     (X * (180.0/3.141592653589793238463))
#define DEGTORAD(X)     (X / (180.0/3.141592653589793238463))

CModule::CModule(double x, double y, double w, double h, bool responsive) {
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;

    responsivePos = responsive;
    responsiveSize = responsive;
}
CModule::~CModule(){}

// ===== CPlot2D CLASS IMPLEMENTATION =====
CPlot2D::CPlot2D(double x, double y, double w, double h, bool responsive) : CModule(x, y, w, h, responsive) {
    children = new CRectangle(x, y, w, h, responsive);
    ((CRectangle*)children)->filled = false;

    dataseries = new CPlotSeries<CPoint2D>();
}
CPlot2D::~CPlot2D(){
    CPlotSeries<CPoint2D>* p = dataseries;
    while(p){
        CPoint2D* pt = p->points;
        while(pt){
            auto ptmp = pt->next;
            if(pt->shape) delete pt->shape;
            delete pt;
            pt = ptmp;
        }
        auto tmp = p->nextseries;
        delete p;
        p = tmp;
    }
    delete children;
}

void CPlot2D::update_series(Canvas *cv) {
    CModule* pts;

    absx = responsivePos? x*cv->getWidth() : x;
    absy = responsivePos? y*cv->getHeight() : y;
    absw = responsiveSize? w*cv->getWidth() : w;
    absh = responsiveSize? h*cv->getHeight() : h;

    for(CPlotSeries<CPoint2D>* p = dataseries; p; p = p->nextseries){
        for(CPoint2D* pt = p->points; pt; pt = pt->next){
            if(BETWEEN(pt->x, minX, maxX) && BETWEEN(pt->y, minY, maxY)){
                pts = pt->shape;
                pts->w = p->shapesize;
                pts->h = p->shapesize;
                pts->x = absx + absw*((pt->x - minX)/(maxX - minX)) - (pts->w / 2);
                pts->y = absy + absh - absh*((pt->y - minY)/(maxY - minY)) - (pts->h / 2);

                pts->draw(cv);
            }
        }
    }
}

void CPlot2D::update_children(Canvas *cv){
    if(drawborder) {
        CRectangle *border = (CRectangle *) children;
        border->x = x;
        border->y = y;
        border->w = w;
        border->h = h;
        border->responsiveSize = responsiveSize;
        border->responsivePos = responsivePos;
        if(!borderclr.empty()) border->setColor(borderclr);

        border->draw(cv);
    }
}

void CPlot2D::draw(Canvas *cv) {
    update_children(cv);
    update_series(cv);
}

void CPlot2D::addPoint(unsigned int series, double x, double y) {
    if(series < dataseries_size){
        CPlotSeries<CPoint2D>* p = dataseries;
        for(unsigned int i = 0; i < series; i++) p = p->nextseries;

        appendPointToSeries(p, x, y);
    } else if(series == dataseries_size+1){
        CPlotSeries<CPoint2D>* p = dataseries;
        for(unsigned int i = 0; i < series-1; i++) p = p->nextseries;

        p->nextseries = new CPlotSeries<CPoint2D>();
        p = p->nextseries;
        dataseries_size++;

        appendPointToSeries(p, x, y);
    }
}

void CPlot2D::appendPointToSeries(CPlotSeries<CPoint2D>* p, double x, double y) {
    CPoint2D* pt = new CPoint2D();
    pt->x = x;
    pt->y = y;
    pt->shape = new CCircle(0, 0, 1, 1, false);
    ((CCircle*)pt->shape)->setColor("#ffffff");
    p->append(pt);
}

void CPlot2D::setPointColor(unsigned int series, std::string color) {
    if(series < dataseries_size){
        CPlotSeries<CPoint2D>* p = dataseries;
        for(unsigned int i = 0; i < series; i++) p = p->nextseries;

        CPoint2D* pt;
        for(pt = p->points; pt; pt = pt->next) ((CShape*)pt->shape)->setColor(color);
    }
}

// ===== CPlot3D CLASS IMPLEMENTATION =====
CPlot3D::CPlot3D(double x, double y, double w, double h, bool responsive) : CModule(x, y, w, h, responsive) {
    children = new CRectangle(x, y, w, h, responsive);
    ((CRectangle*)children)->filled = false;

    dataseries = new CPlotSeries<CPoint3D>();
}
CPlot3D::~CPlot3D(){
    CPlotSeries<CPoint3D>* p = dataseries;
    while(p){
        CPoint3D* pt = p->points;
        while(pt){
            auto ptmp = pt->next;
            if(pt->shape) delete pt->shape;
            delete pt;
            pt = ptmp;
        }
        auto tmp = p->nextseries;
        delete p;
        p = tmp;
    }
    delete children;
}

void CPlot3D::update_series(Canvas *cv) {
    CModule* pts;

    absx = responsivePos? x*cv->getWidth() : x;
    absy = responsivePos? y*cv->getHeight() : y;
    absw = responsiveSize? w*cv->getWidth() : w;
    absh = responsiveSize? h*cv->getHeight() : h;

    double x_view, y_view, dist, prex, prey;

    for(CPlotSeries<CPoint3D>* p = dataseries; p; p = p->nextseries){
        bool first = true;
        for(CPoint3D* pt = p->points; pt; pt = pt->next){
            if(mapToPlot(pt, &x_view, &y_view, &dist)) {
                pts = pt->shape;

                if (perspectiveSizes){
                    double size = 100*(double)(p->shapesize) / sqrt(dist);
                    if(size < 2.0) size = 2.0;
                    pts->w = size;
                    pts->h = size;
                }else{
                    pts->w = p->shapesize;
                    pts->h = p->shapesize;
                }

                double xv = cam.nearclip*sin(x_view);
                double yv = cam.nearclip*sin(y_view);
                pts->x = absx + absw*((xv + cam.nearclip*sin(DEGTORAD(cam.fov)/2))/(2*cam.nearclip*sin(DEGTORAD(cam.fov)/2))) - (pts->w / 2);
                pts->y = absy + absh - absh*((yv + cam.nearclip*sin(absh*DEGTORAD(cam.fov)/(2*absw)))/(2*cam.nearclip*sin(absh*DEGTORAD(cam.fov)/(2*absw)))) - (pts->h / 2);
                pts->draw(cv);

                if(p->drawlines) {
                    if (!first) {
                        cv->line(prex, prey, pts->x + pts->w / 2.0, pts->y + pts->h / 2.0);
                    }
                    prex = pts->x + pts->w / 2.0;
                    prey = pts->y + pts->h / 2.0;
                    first = false;
                }
            } else
                first = true;
        }
    }
}

void CPlot3D::update_children(Canvas *cv){
    if(drawborder) {
        CRectangle *border = (CRectangle *) children;
        border->x = x;
        border->y = y;
        border->w = w;
        border->h = h;
        border->responsiveSize = responsiveSize;
        border->responsivePos = responsivePos;
        if(!borderclr.empty()) border->setColor(borderclr);

        border->draw(cv);
    }
}

void CPlot3D::draw(Canvas *cv) {
    update_children(cv);
    update_series(cv);
}

void CPlot3D::addPoint(unsigned int series, double x, double y, double z) {
    if(series < dataseries_size){
        CPlotSeries<CPoint3D>* p = dataseries;
        for(unsigned int i = 0; i < series; i++) p = p->nextseries;

        appendPointToSeries(p, x, y, z);
    } else if(series == dataseries_size+1){
        CPlotSeries<CPoint3D>* p = dataseries;
        for(unsigned int i = 0; i < series-1; i++) p = p->nextseries;

        p->nextseries = new CPlotSeries<CPoint3D>();
        p = p->nextseries;
        dataseries_size++;

        appendPointToSeries(p, x, y, z);
    }
}

void CPlot3D::appendPointToSeries(CPlotSeries<CPoint3D>* p, double x, double y, double z) {
    /*if(p->points){
        CPoint3D* last;
        for(last = p->points; last->next; last = last->next);

    }*/

    CPoint3D* pt = new CPoint3D();
    pt->x = x;
    pt->y = y;
    pt->z = z;
    pt->shape = new CCircle(0, 0, 1, 1, false);
    ((CCircle*)pt->shape)->setColor(p->color);
    p->append(pt);
}

void CPlot3D::setPointColor(unsigned int series, std::string color) {
    if(series < dataseries_size){
        CPlotSeries<CPoint3D>* p = dataseries;
        for(unsigned int i = 0; i < series; i++) p = p->nextseries;

        CPoint3D* pt;
        for(pt = p->points; pt; pt = pt->next) ((CShape*)pt->shape)->setColor(color);
    }
}

bool CPlot3D::mapToPlot(CPoint3D* pt, double* x_view, double* y_view, double* distsqr){
    double dx = pt->x - cam.x;
    double dy = pt->y - cam.y;
    double dz = pt->z - cam.z;

    *distsqr = dx*dx + dy*dy + dz*dz;
    if(*distsqr > (cam.farclip * cam.farclip)) return false;

    double dx1 =   dx*cos(cam.theta) + dy*sin(cam.theta);
    double dy1 = - dx*sin(cam.theta) + dy*cos(cam.theta);
    double dz1 =   dz;

    double dx2 =   dx1*cos(cam.phy) + dz1*sin(cam.phy);
    double dy2 =   dy1;
    double dz2 = - dx1*sin(cam.phy) + dz1*cos(cam.phy);

    double dx3 =   dx2;
    double dy3 =   dy2*cos(cam.psi) + dz2*sin(cam.psi);
    double dz3 = - dy2*sin(cam.psi) + dz2*cos(cam.psi);

    //*x_view = RADTODEG(atan2(dy3, dx3));
    //*y_view = RADTODEG(atan2(dz3, dx3));
    *x_view = atan2(dy3, dx3);
    *y_view = atan2(dz3, dx3);

    return (BETWEEN(RADTODEG(*x_view) * 2, -cam.fov, cam.fov) && BETWEEN(RADTODEG(*y_view)*2*absw/absh, -cam.fov, cam.fov));
}

// ===== CTextBox CLASS IMPLEMENTATION =====
CTextBox::CTextBox(double x, double y, double w, double h, bool responsive) : CModule(x, y, w, h, responsive) {
    children = new CRectangle(x, y, w, h, responsive);
    ((CRectangle*)children)->filled = false;

}
CTextBox::~CTextBox(){
    CModule* m = children;
    while(m->next) {
        CModule *cm;
        for (cm = m; cm->next->next; cm = cm->next);
        delete (CText*)(cm->next);
        cm->next = nullptr;
    }
    delete (CRectangle*)m;
}

void CTextBox::update_children(Canvas *cv){
    if(drawborder) {
        CRectangle *border = (CRectangle *) children;
        border->x = x;
        border->y = y;
        border->w = w;
        border->h = h;
        border->responsiveSize = responsiveSize;
        border->responsivePos = responsivePos;
        if(!borderclr.empty()) border->setColor(borderclr);

        border->draw(cv);
    }
}

void CTextBox::draw(Canvas *cv) {
    update_children(cv);

    unsigned int texw = (responsiveSize? w*cv->getWidth() : w) - padding[0] - padding[2];
    unsigned int absx = (responsivePos? x*cv->getWidth() : x) + padding[0];
    unsigned int absy = (responsivePos? y*cv->getHeight() : y) + padding[1];

    if(text != cache){
        cache = text;

        CModule* m = children;
        while(m->next) {
            CModule *cm;
            for (cm = m; cm->next->next; cm = cm->next);
            delete (CText*)(cm->next);
            cm->next = nullptr;
        }

        size_t curi = text.find_first_of(' ');

        m->next = new CText(text.substr(0, curi) + ' ', x, y, w, 0, false);
        m = m->next;

        while(1){
            size_t ncuri = text.find(' ', curi+1);

            std::string nextword = text.substr(curi+1, ncuri-curi-1);

            m->next = new CText(nextword + ' ', 0, 0, 0, 0, false);
            m = m->next;

            curi = ncuri;
            if(ncuri == std::string::npos) break;
        }
    }

    unsigned int curx = 0;
    unsigned int cury = 10;
    for(CText* ct = (CText*)children->next; ct; ct = (CText*)ct->next){
        if(!font.empty()) ct->font = font;
        unsigned int tw = ct->getTextWidth(cv);

        ct->x = absx + curx;
        ct->y = absy + cury;

        if(curx + tw < texw) curx += tw;
        else{
            curx = 0;
            cury += lspacing;
            ct->x = absx + curx;
            ct->y = absy + cury;

            curx += tw;
        }

        ct->w = tw*1.5;
        if(!textclr.empty()) ct->setColor(textclr);
        ct->draw(cv);
    }
}
