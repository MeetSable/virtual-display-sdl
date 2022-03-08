//cpp libs
#include <iostream>
#include <math.h>
#include <string>
#include <vector>
#include <map>
//SDL libs
#include <SDL.h>
#include <SDL_render.h>
//#include <SDL_ttf.h>
//imgui libs
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"


#include "vDisp.h"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

SDL_Window* window = SDL_CreateWindow("SDLBasic", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

vDisp disp(renderer, 20, WINDOW_WIDTH, WINDOW_HEIGHT);

bool isRunning = true;
SDL_Event event;

SDL_Color color = { 255,255,255,255 };
SDL_Point startPoint, endPoint;
int startX = 0, startY = 0, endX = 0, endY = 0;

//Imgui----------------------
ImVec4 color_selection = ImVec4(1.f, 1.f, 1.f, 1.f);
std::map<int, aLine> linesArray;
std::map<int, aPolygon> polygonArray;
int total_lines = 0, curr_editing = -1, currenteditor = 0;
int total_polygons = 0, curr_poly_editing = -1, recordedPoint = -1;
bool startPointFlag = false, endPointFlag = false, editingLine = false, editingPolygon = false, recordPolygonPoint = false;
int x, y;

enum EditorTab
{
    lineEditor = 0,
    polygonEditor
};

void ProcessEvent() {
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        switch (event.type)
        {
        case SDL_QUIT:
            isRunning = false;
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                if (startPointFlag) {
                    SDL_GetMouseState(&x, &y);
                    int PixelSize = disp.GetPixelSize();
                    startX = x / PixelSize;
                    startY = y / PixelSize;

                    startPointFlag = false;
                }
                else if (endPointFlag) {
                    SDL_GetMouseState(&x, &y);
                    int PixelSize = disp.GetPixelSize();
                    endX = x / PixelSize;
                    endY = y / PixelSize;

                    endPointFlag = false;
                }
                else if (recordPolygonPoint) {
                    SDL_GetMouseState(&x, &y);
                    int PixelSize = disp.GetPixelSize();
                    polygonArray[curr_poly_editing].cornerPoints[recordedPoint].x = x/PixelSize;
                    polygonArray[curr_poly_editing].cornerPoints[recordedPoint].y = y/PixelSize;

                    recordPolygonPoint = false;
                }
            }
            break;

        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
            {
                isRunning = false;
            }
            break;

        }


    }
}

void imGui() {
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();

    ImGui::Begin("Editor");
    if (ImGui::Button("Line"))
        currenteditor = lineEditor;
    ImGui::SameLine();
    if (ImGui::Button("Polygon"))
        currenteditor = EditorTab::polygonEditor;
    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    switch (currenteditor)
    {
    case lineEditor:
        if (ImGui::Button("Add Line")) {
            total_lines++;
        }
        if (total_lines > linesArray.size())
            for (int i = 0; i < total_lines; i++)
                if (!linesArray[i].alreadyExisted && linesArray.size() == total_lines)
                    break;

        for (auto& i : linesArray) {
            ImGui::Text("Line %d", i.first);
            ImGui::SameLine();
            std::string edit = "Edit ##" + std::to_string(i.first);
            std::string delete_line = "Delete ##" + std::to_string(i.first);
            if (ImGui::Button(edit.c_str())) {
                curr_editing = i.first;
                editingLine = true;
                startX = i.second.startPoint.x;
                startY = i.second.startPoint.y;
                endX = i.second.endPoint.x;
                endY = i.second.endPoint.y;
                color_selection = { i.second.color.r / 255.f,i.second.color.g / 255.f,i.second.color.b / 255.f, i.second.color.a / 255.f };
                break;
            }
            ImGui::SameLine();
            if (ImGui::Button(delete_line.c_str())) {
                editingLine = false;
                curr_editing = -1;
                total_lines--;
                linesArray.erase(i.first);
                break;
            }
            ImGui::SameLine();
            std::string colr_id = "##" + std::to_string(i.first);
            float colr[] = { i.second.color.r / 255.f, i.second.color.g / 255.f , i.second.color.b / 255.f };
            ImGui::ColorEdit3(colr_id.c_str(), colr, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
        }
        if (editingLine && curr_editing != -1) {
            std::string title = "Line #" + std::to_string(curr_editing) + " editor";
            aLine* ref = &linesArray[curr_editing];
            ImGui::Begin(title.c_str());
            ImGui::Text("Choose Color");
            ImGui::ColorEdit3("", (float*)&color_selection);
            std::string delete_line = "Delete ##" + std::to_string(curr_editing);
            //StartPoint
            if (ImGui::Button((startPointFlag ? "Listening..." : "Record Start Point"))) {
                startPointFlag = true;
            }
            ImGui::Text("x: %d y: %d", startX, startY);
            //EndPoint
            if (ImGui::Button((endPointFlag ? "Listening..." : "Record End Point"))) {
                endPointFlag = true;
            }
            ImGui::Text("x: %d y: %d", endX, endY);
            if (ImGui::Button("Draw Line")) {
                ref->startPoint.x = startX, ref->startPoint.y = startY;
                ref->endPoint.x = endX, ref->endPoint.y = endY;
                ref->color.r = color_selection.x * 255;
                ref->color.g = color_selection.y * 255;
                ref->color.b = color_selection.z * 255;
            }
            if (ImGui::Button(delete_line.c_str())) {
                editingLine = false;
                linesArray.erase(curr_editing);
                curr_editing = -1;
                total_lines--;
            }
            if (ImGui::Button("Close"))
                editingLine = false, curr_editing = -1;
            ImGui::End();
        }
        break;

    case polygonEditor:
        {
        if (ImGui::Button("Add Polygon"))
            total_polygons++;
        if (total_polygons > polygonArray.size())
            for (int i = 0; i < total_polygons; i++)
                if (!polygonArray[i].alreadyExisted && polygonArray.size() == total_polygons)
                    break;
        for (auto& p : polygonArray) {
            ImGui::Text("Polygon %d", p.first);
            ImGui::SameLine();
            std::string edit = "Edit ##" + std::to_string(p.first);
            std::string delete_poly = "Delete ##" + std::to_string(p.first);
            if (ImGui::Button(edit.c_str())) {
                curr_poly_editing = p.first;
                editingPolygon = true;
            }
            ImGui::SameLine();
            if (ImGui::Button(delete_poly.c_str())) {
                editingPolygon = false;
                curr_poly_editing = -1;
                total_polygons--;
                polygonArray.erase(p.first);
                break;
            }

        }
        if (editingPolygon && curr_poly_editing != -1) {
            std::string title = "Polygon #" + std::to_string(curr_poly_editing) + " editor";
            std::string button_id, delete_button;
            aPolygon* ref = &polygonArray[curr_poly_editing];
            ImGui::Begin(title.c_str());
            ImGui::Text("Choose Fill color:");
            ImGui::ColorEdit3("", (float*) & color_selection);
            ref->fillColor.r = color_selection.x * 255;
            ref->fillColor.g = color_selection.y * 255;
            ref->fillColor.b = color_selection.z * 255;
            ref->fillColor.a = color_selection.w * 255;
            ImGui::Checkbox("Fill the polygon?",&(ref->fill));
            ImGui::Checkbox("Show Borderlines?", &(ref->borders));
            if (ImGui::Button("Add Point")) {
                ref->noOfPoints++;
                ref->cornerPoints.insert(ref->cornerPoints.begin(), { 0,0 });
            }
            for (int i = 0; i < ref->noOfPoints; i++) 
            {
                button_id = ((recordPolygonPoint && i == recordedPoint)?"Listening ##" : "Record Point #") + std::to_string(i);
                if (ImGui::Button(button_id.c_str())) {
                    recordedPoint = i;
                    recordPolygonPoint = true;
                }
                if (ref->noOfPoints > 3) {
                    ImGui::SameLine();
                    delete_button = "delete ##" + std::to_string(i);
                    if (ImGui::Button(delete_button.c_str())) {
                        ref->cornerPoints.erase(ref->cornerPoints.begin() + i);
                        ref->noOfPoints--;
                    }
                }
                ImGui::Text("X: %d  Y: %d", ref->cornerPoints[i].x, ref->cornerPoints[i].y);
            }
            if (ImGui::Button("Delete")) {
                polygonArray.erase(curr_poly_editing);
                editingPolygon = false;
                total_polygons--;
                curr_poly_editing = -1;
            }

            if (ImGui::Button("Close"))
                editingPolygon = false, curr_poly_editing = -1;
            ImGui::End();
        }
        }
        break;

    default:
        break;
    }
    ImGui::End();
}

void render() {
    ImGui::Render();
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    disp.DrawGrid({ 255,255,255,255 });
    //virtual display rendering
    for (auto& line : linesArray)
        disp.DrawLine(line.second);

    for (auto& poly : polygonArray)
        disp.DrawPolygon(poly.second);

    if (editingLine) {
        disp.DrawSelected(startX, startY);
        disp.DrawSelected(endX, endY);
    }
    if (editingPolygon) {
        for (auto& point :  polygonArray[curr_poly_editing].cornerPoints) {
            disp.DrawSelected(point.x, point.y);
        }
    }


    //SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_EVERYTHING);

    //Imgui----------------------
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window,renderer);
    ImGui_ImplSDLRenderer_Init(renderer);

    while (isRunning)
    {
        ProcessEvent();

        imGui();

        render();

    }

    // Cleanup
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();


       

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
} 

