//
// Created by victor on 8/26/20.
//

#ifndef CNAVCOMP_CMODULE_H
#define CNAVCOMP_CMODULE_H
#include <string>

#include "../Canvas.h"

class CModule {
public:
    CModule(double x, double y, double w, double h, bool responsive);
    virtual ~CModule();

    virtual void draw(Canvas* cv) = 0;

    CModule* next = nullptr;
    bool responsivePos = false;
    bool responsiveSize = false;
    double x = 0, y = 0, w = 0, h = 0;
    double absx=0, absy=0, absw=0, absh=0;
protected:
    CModule* children = nullptr;
};

struct CPoint2D{
    const int n = 2;
    double x = 0, y = 0;

    CPoint2D* next = nullptr;
    CModule* shape = nullptr;

    double operator[] (unsigned int i){
        if(i==0) return x;
        else if(i==1) return y;
        return 0;
    };
};
struct CPoint3D{
    const int n = 3;
    double x = 0, y = 0, z = 0;

    CPoint3D* next = nullptr;
    CModule* shape = nullptr;

    double operator[] (unsigned int i){
        if(i==0) return x;
        else if(i==1) return y;
        else if(i==3) return z;
        return 0;
    };
};

template <typename T>
concept LinkedPoint = requires (T p, unsigned int i) { p[i]; } && requires (T* p) { p->next; } && requires (T* p) { p->shape; } && requires (T* p) { p->n; };

template <LinkedPoint P>
struct CPlotSeries{
    unsigned int shapesize = 5;
    std::string color = "#fcae1e";

    bool drawlines = true;

    P* points = nullptr;
    //CModule* lines = nullptr;

    CPlotSeries<P>* nextseries = nullptr;

    P* operator[] (unsigned int i){
        P* p = points;
        for(int j = 0; j < i, p; j++, p = p->next);
        return p;
    };

    void append(P* pts){
        if(!points){
            points = pts;
            return;
        }
        P* p;
        for(p = points; p->next; p = p->next);
        p->next = pts;
    };

    /*void appendLine(CModule* line){
        if(!lines){
            lines = line;
            return;
        }
        CModule* l;
        for(l = lines; l->next; l = l->next);
        l->next = line;
    }*/
};

class CPlot2D : public CModule{
public:
    CPlot2D(double x, double y, double w, double h, bool responsive);
    ~CPlot2D();

    void update_series(Canvas* cv);
    void update_children(Canvas *cv);
    void draw(Canvas* cv) override;

    void addPoint(unsigned int series, double x, double y);

    void setPointColor(unsigned int series, std::string color);

    CPlotSeries<CPoint2D>* dataseries = nullptr;
    double minX = -10, maxX = 10, minY = -10, maxY = 10;
    bool drawborder = true;
    std::string borderclr, textclr, font;

private:
    void appendPointToSeries(CPlotSeries<CPoint2D>* p, double x, double y);

    unsigned int dataseries_size = 1;
};

class CPlot3D : public CModule {
public:
    CPlot3D(double x, double y, double w, double h, bool responsive);
    ~CPlot3D();

    void update_series(Canvas* cv);
    void update_children(Canvas *cv);
    void draw(Canvas* cv) override;

    void addPoint(unsigned int series, double x, double y, double z);

    void setPointColor(unsigned int series, std::string color);

    CPlotSeries<CPoint3D>* dataseries = nullptr;
    bool perspectiveSizes = false;
    bool drawborder = true;
    bool drawLines = false;
    std::string borderclr, textclr, font;

    struct Camera {
        double x = 0, y = 0, z = 0;
        double theta = 0, phy = 0, psi = 0;
        double farclip = 1000, nearclip = 0.5, fov = 60;

        //void setPos(double _x, double _y, double _z){ x = _x; y = _y; z = _z; }
    };
    Camera cam;
private:
    void appendPointToSeries(CPlotSeries<CPoint3D>* p, double x, double y, double z);

    bool mapToPlot(CPoint3D* pt, double* x_view, double* y_view, double* distsqr);

    unsigned int dataseries_size = 1;
};

class CTextBox : public CModule{
public:
    CTextBox(double x, double y, double w, double h, bool responsive);
    ~CTextBox();

    void update_children(Canvas *cv);
    void draw(Canvas* cv) override;

    std::string text;
    int padding[4] = {10,10,10,10};
    int lspacing = 20;
    bool drawborder = true;
    std::string borderclr, textclr, font;
private:
    std::string cache;
};

#endif //CNAVCOMP_CMODULE_H
