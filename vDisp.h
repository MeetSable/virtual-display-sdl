#pragma once

#include <SDL.h>
#include <vector>
#include <map>
#include <unordered_set>
#include <set>
#include <queue>

#define MOD(a,b) (((a%b)+b)%b) 
#define MAX(a,b) (a>b?a:b)
#define MIN(a,b) (a<b?a:b)

struct aLine {
    SDL_Point startPoint;
    SDL_Point endPoint;
    SDL_Color color;
    bool alreadyExisted;
    aLine(SDL_Point startPoint, SDL_Point endPoint, SDL_Color color) : startPoint(startPoint), endPoint(endPoint), color(color), alreadyExisted(true) {};
    aLine() :color({ 0,0,0,255 }), alreadyExisted(false), startPoint({ 0,0 }), endPoint({ 0,0 }) {};
};

struct aPolygon {
    int noOfPoints;
    std::vector<SDL_Point> cornerPoints;
    SDL_Color fillColor;
    bool fill;
    bool borders;
    bool alreadyExisted;
    aPolygon(int n, std::vector<SDL_Point> points, SDL_Color color, bool fill): noOfPoints(n), cornerPoints(points), fillColor(color), fill(fill), alreadyExisted(true), borders(false) {};
    aPolygon() :noOfPoints(3), cornerPoints({ {0,0},{0,0},{0,0} }), fillColor({ 0,0,0,255 }), fill(false), alreadyExisted(false), borders(false) {};
};


class vDisp {
private:
    SDL_Renderer* renderer;
    int pixelSize;
    int width, height;
    int WINDOW_WIDTH, WINDOW_HEIGHT;

private:
    void DrawLinebase(float, float, float, SDL_Color);
    void DrawLinebase(SDL_Point, SDL_Point, SDL_Color);
    void DrawRectbase(SDL_Point, float, SDL_Color);
    void DrawRectbase(SDL_Point, SDL_Point, SDL_Color);
    void DrawLinePolygon(const aLine&, std::map<int, std::priority_queue<int, std::vector<int>, std::greater<int>>>*);

public:
    vDisp(SDL_Renderer*, int, int, int);

    int GetPixelSize();
    void DrawPixel(int, int, SDL_Color);
    void DrawGrid(SDL_Color);
    void DrawSelected(int, int);
    void DrawLine(const aLine&);
    void DrawPolygon(const aPolygon&);
};