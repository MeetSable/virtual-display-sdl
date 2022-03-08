#pragma once
#include "vDisp.h"

#include <math.h>
#include <iostream>
#include <algorithm>
#include "sdl_utility.h"

SDL_Color default_border_color = { 0,0,255,255 };

vDisp::vDisp(SDL_Renderer* renderer, int pixelSize, int WINDOW_WIDTH, int WINDOW_HEIGHT) : renderer(renderer), pixelSize(pixelSize), WINDOW_WIDTH(WINDOW_WIDTH), WINDOW_HEIGHT(WINDOW_HEIGHT) 
{
    width = WINDOW_WIDTH / pixelSize + 1;
    height = WINDOW_HEIGHT / pixelSize + 1;
}

void vDisp::DrawLinebase(float x, float m, float c, SDL_Color color) 
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int i = x; i < WINDOW_WIDTH; i++) 
    {
        int X = i;
        int Y = m * i + c;
        SDL_RenderDrawPoint(renderer, X, Y);
    }
}

void vDisp::DrawLinebase(SDL_Point start, SDL_Point end, SDL_Color color) 
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    if (end.x == start.x) {
        for (int i = start.y; i <= end.y; i++) 
        {
            SDL_RenderDrawPoint(renderer, start.x, i);
        }
    }
    else {
        float slope = (end.y - start.y) / (end.x - start.x);
        for (int i = start.x; i <= end.x; i++) 
        {
            SDL_RenderDrawPoint(renderer, i, (i * slope + start.y));
        }
    }
}

void vDisp::DrawRectbase(SDL_Point pos, SDL_Point dim, SDL_Color color) 
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int i = pos.x; i <= pos.x + dim.x; i++)
        for (int j = pos.y; j <= pos.y + dim.y; j++)
            SDL_RenderDrawPoint(renderer, i, j);
}

void vDisp::DrawRectbase(SDL_Point pos, float l, SDL_Color color) 
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int i = pos.x; i <= pos.x + l; i++)
        for (int j = pos.y; j <= pos.y + l; j++)
            SDL_RenderDrawPoint(renderer, i, j);
}

int vDisp::GetPixelSize() { return pixelSize; }

void vDisp::DrawPixel(int x, int y, SDL_Color color) 
{
    int left = x * pixelSize, top = y * pixelSize;
    DrawRectbase({ left + 1, top + 1 }, pixelSize - 2, color);
}

void vDisp::DrawGrid(SDL_Color color) 
{
    for (int i = 0; i < width; i++)
        DrawLinebase({ i * pixelSize, 0 }, { i * pixelSize, WINDOW_HEIGHT }, color);
    for (int i = 0; i < height; i++)
        DrawLinebase({ 0, i * pixelSize }, { WINDOW_WIDTH, i * pixelSize }, color);
}

void vDisp::DrawSelected(int x, int y) 
{
    int left = x * pixelSize, top = y * pixelSize;
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawLine(renderer, (left + 4), (top + 4), (left + pixelSize - 4), (top + pixelSize - 4));
    SDL_RenderDrawLine(renderer, (left + pixelSize - 4), (top + 4), (left + 4), (top + pixelSize - 4));
}

void vDisp::DrawLine(const aLine& line) 
{
    SDL_Point startPoint = line.startPoint;
    SDL_Point endPoint = line.endPoint;
    SDL_Color color = line.color;

    if (startPoint.x == endPoint.x && startPoint.y == endPoint.y) {
        DrawPixel(startPoint.x, startPoint.y, color);
    }
    else {

        float dx = endPoint.x - startPoint.x, dy = endPoint.y - startPoint.y;

        if (std::fabs(dx) > std::fabs(dy)) {
            int x_step = std::signbit(dx) ? -1 : 1;
            int y_step = std::signbit(dy) ? -1 : 1;
            dx = dx * x_step;
            dy = dy * y_step;
            float d = dy - (dx * 0.5f);
            int x = startPoint.x, y = startPoint.y;
            DrawPixel(x, y, color);
            while (x != endPoint.x) 
            {
                x = x + x_step;
                if (d < 0)
                    d = d + dy;
                else {
                    d = d + dy - dx;
                    y = y + y_step;
                }
                DrawPixel(x, y, color);
            }
        }
        else {
            int x_step = std::signbit(dx) ? -1 : 1;
            int y_step = std::signbit(dy) ? -1 : 1;
            dx = dx * x_step;
            dy = dy * y_step;
            float d = dx - (dy / 2.f);
            int x = startPoint.x, y = startPoint.y;
            DrawPixel(x, y, color);
            while (y != endPoint.y) 
            {
                y = y + y_step;
                if (d < 0)
                    d = d + dx;
                else {
                    d = d + dx - dy;
                    x = x + x_step;
                }
                DrawPixel(x, y, color);
            }
        }
    }

}

void vDisp::DrawLinePolygon(const aLine& line, std::map<int, std::priority_queue<int, std::vector<int>, std::greater<int>>>* edges)
{
    SDL_Point startPoint = line.startPoint;
    SDL_Point endPoint = line.endPoint;
    if (startPoint.x == endPoint.x && startPoint.y == endPoint.y) 
    {
        (*edges)[startPoint.y].push(startPoint.x);
    }
    else 
    {
        int increment = (endPoint.y > startPoint.y ? 1 : -1);
        int i = startPoint.y;
        if (startPoint.x != endPoint.x)
        {
            double slope = (double)(endPoint.x - startPoint.x) / (double)(endPoint.y - startPoint.y);
            while (i != endPoint.y)
            {
                (*edges)[i].push((i - startPoint.y) * slope + startPoint.x);
                i += increment;
            }
        }
        else
        {
            while (i != endPoint.y)
            {
                (*edges)[i].push(startPoint.x);
                i += increment;
            }
        }
    }
}

void vDisp::DrawPolygon(const aPolygon& polygon) 
{
    int n = polygon.noOfPoints;
    SDL_Color color = polygon.fillColor;
    SDL_Point min = { INT_MAX,INT_MAX }, max = {0,0};
    aLine tempLine;
    std::map<int, std::priority_queue<int, std::vector<int>, std::greater<int>>> edgeBuff;
    for (int i = 0; i < n; i++) 
    {
        
        tempLine.startPoint = polygon.cornerPoints[MOD(i, n)];
        tempLine.endPoint = polygon.cornerPoints[MOD(i + 1, n)];
        DrawLinePolygon(tempLine, &edgeBuff);
        min.x = MIN(min.x, tempLine.startPoint.x);
        min.y = MIN(min.y, tempLine.startPoint.y);
        max.x = MAX(max.x, tempLine.endPoint.x);
        max.y = MAX(max.y, tempLine.endPoint.y);
    }
    int edgePoint;
    std::vector<SDL_Point> tempPoints;
    SDL_Point currPoint;
    for (int y = min.y; y <= max.y; y++)
    {
        edgePoint = edgeBuff[y].top();
        //polygon->cornerPoints;
        for (int x = min.x; x <= max.x; x++)
        {
            if (x == edgePoint)
            {
                currPoint = { x,y };
                auto tempIt = std::find(polygon.cornerPoints.begin(), polygon.cornerPoints.end(), currPoint);
                if (tempIt != polygon.cornerPoints.end()) {
                    int index = tempIt - polygon.cornerPoints.begin();
                    if ((y <= polygon.cornerPoints[MOD(index + 1, polygon.noOfPoints)].y && y >= polygon.cornerPoints[MOD(index - 1, polygon.noOfPoints)].y) ||
                        (y <= polygon.cornerPoints[MOD(index - 1, polygon.noOfPoints)].y && y >= polygon.cornerPoints[MOD(index + 1, polygon.noOfPoints)].y))
                    {
                        if (tempPoints.size() % 2 == 0)
                            currPoint.x++;
                        
                        
                        tempPoints.push_back(currPoint);
                    }
                }
                else
                {
                    if (tempPoints.size() % 2 == 0)
                        currPoint.x++;
                    
                    tempPoints.push_back(currPoint);
                }

                edgeBuff[y].pop();
                //SDL_Color borderColor = (polygon.borders ? default_border_color : polygon.fillColor);
                if (edgeBuff[y].empty())
                    break;
                edgePoint = edgeBuff[y].top();
            }
        }
        if(polygon.fill)
            for (int i = 0; i < tempPoints.size() / 2; i++)
            {
                if(tempPoints[i * 2 + 1].x - tempPoints[i * 2].x > 0)
                    DrawLine(aLine(tempPoints[i * 2], tempPoints[i * 2 + 1], color));
            }
        tempPoints.clear();
    }
    if (polygon.borders)
        for (int i = 0; i < n; i++)
            DrawLine(aLine(polygon.cornerPoints[MOD(i, n)], polygon.cornerPoints[MOD(i + 1, n)], default_border_color));
    

}
