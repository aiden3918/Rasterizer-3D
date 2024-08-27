#define OLC_PGE_APPLICATION
#define OLC_PGEX_QUICKGUI

#include <Windows.h>
#include <algorithm>
#include <string>
#include <iostream>
#include <vector>
#include <math.h> 
#include <regex>

// #include "olcConsoleGameEngine.h"
#include "headers/olcPixelGameEngine.h"
#include "headers/olcPGEX_QuickGUI.h"

#include "headers/plane3D.hpp"
#include "headers/defintions.hpp"
#include "headers/parametric.hpp"

class olc3DEngine : public olc::PixelGameEngine {
public:
    olc3DEngine()
    {
        sAppName = "3D Parametric Rasterizer";
    }

public:

    bool OnUserCreate() override
    {
        // INIT SPRITES --------------------------------------------------------------------------------------------
        axesRef = std::make_unique<olc::Sprite>("assets/axesreference.png");
        // ---------------------------------------------------------------------------------------------------------
        
        // INIT PLANE ----------------------------------------------------------------------------------------------
        graph3DPlane.lines = {
            {-10.0f, 0.0f, 0.0f, 1.0f,     10.0f, 0.0f, 0.0f, 1.0f },
            {0.0f, -10.0f, 0.0f, 1.0f,     0.0f, 10.0f, 0.0f, 1.0f },
            {0.0f, 0.0f, -10.0f, 1.0f,     0.0f, 0.0f, 10.0f, 1.0f },
        };
        // gray increment lines
        // for x-axis lines, use loop to change y and z
        // for y-axis lines, use loop to change x and z
        // for z-axis lines, use loop to change x and y
        for (float x = -10.0f; x <= 10.0f; x++) {
            graph3DPlane.lines.push_back({ x, -0.2f, 0.0f, 1.0f,     x, 0.2f, 0.0f, 1.0f }); 
        }

        for (float y = -10.0f; y <= 10.0f; y++) {
            graph3DPlane.lines.push_back({ -0.2f, y, 0.0f, 1.0f,     0.2f, y, 0.0f, 1.0f });
        }

        for (float z = -10.0f; z <= 10.0f; z++) {
            graph3DPlane.lines.push_back({ -0.2f, 0.0f, z, 1.0f,     0.2f, 0.0f, z, 1.0f });
        }

        // af, 0, 0, 0
        // 0,  f, 0, 0
        // 0,  0, -q, 1
        // 0,  0, -znearq, 0
        projectionMatrix = Matrix_MakeProjection(90.0f, (float)ScreenHeight() / (float)ScreenWidth(), 0.1f, 1000.0f);
        // ---------------------------------------------------------------------------------------------------------

        // INIT BUTTONS --------------------------------------------------------------------------------------------
        xTextInput = new olc::QuickGUI::TextBox(guiManager, "", { 137, 5 }, { 80, 25 }); 
        yTextInput = new olc::QuickGUI::TextBox(guiManager, "", { 232, 5 }, { 80, 25 });
        zTextInput = new olc::QuickGUI::TextBox(guiManager, "", { 327, 5 }, { 80, 25 });
        calcBtn = new olc::QuickGUI::Button(guiManager, "Rasterize", { 137, 35 }, { 100, 25 });

        findPtTextInput = new olc::QuickGUI::TextBox(guiManager, "", { 600, 5 }, { 80, 25 }); 
        findPtBtn = new olc::QuickGUI::Button(guiManager, "Find", { 600, 35 }, { 50, 25 }); 
        ptPropertiesLabel = new olc::QuickGUI::Label(guiManager, "", { 10, 120 }, { 300, 100 }); 
        ptPropertiesLabel->nAlign = olc::QuickGUI::Label::Alignment::Left;

        invalidMsgLabel = new olc::QuickGUI::Label(guiManager, "", { 10, 60 }, { 200, 30 });
        invalidMsgLabel->nAlign = olc::QuickGUI::Label::Alignment::Left;
        // ---------------------------------------------------------------------------------------------------------
        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        // logical stuff in here first
        guiManager.Update(this);

        if (GetKey(olc::A).bHeld || GetKey(olc::LEFT).bHeld) fYaw -= fElapsedTime;
        if (GetKey(olc::D).bHeld || GetKey(olc::RIGHT).bHeld) fYaw += fElapsedTime;
        if (GetKey(olc::W).bHeld || GetKey(olc::UP).bHeld) fPitch -= fElapsedTime;
        if (GetKey(olc::S).bHeld || GetKey(olc::DOWN).bHeld) fPitch += fElapsedTime;
        if (GetKey(olc::R).bHeld) { fPitch = 0.0f; fYaw = 0.0f; }

        // check for valid inputs, then update graph if its good
        if (calcBtn->bPressed) {
            setExpression(0, xTextInput->sText);
            setExpression(1, yTextInput->sText);
            setExpression(2, zTextInput->sText);
            if (validExpressionInput(getExpressions())) {
                resetGraphFunction();
                updateParametricPoints();
                findPtTextInput->sText = "";
                ptPropertiesLabel->sText = "";
                invalidMsgLabel->sText = "";
                tPoint.active = false;
            }
            else {
                invalidMsgLabel->sText = "Invalid input(s). Please try again";
                resetExpressions();
            }
        }

        // check for valid input, then get properties of function at t
        if (findPtBtn->bPressed) {
            if (validNumInput(findPtTextInput->sText) && validExpressionInput(getExpressions())) {
                tPoint = getPtData(std::stof(findPtTextInput->sText), getExpressions());
                setPtDataLabel(tPoint);
                tPoint.active = true;
                invalidMsgLabel->sText = "";
            }
            else {
                invalidMsgLabel->sText = "Invalid input(s). Please try again";
            }
        }

        // drawing stuff from here on out
        Clear(olc::BLACK);

        // 0. reset screen
        // 1. mathematically rotate
        // 2. mathematically translate
        // 3. check if it should be displayed on screen via dot product of normalk
        // 4. project 3d coords into 2d space
        // 5. visually scale it to the screen
        // 6. transform it in screenspace
        // 7. visually draw

        // rotation matrices
        matrix4x4 matRotZ = Matrix_MakeRotationZ(fRoll);
        matrix4x4 matRotY = Matrix_MakeRotationY(fYaw);
        matrix4x4 matRotX = Matrix_MakeRotationX(fPitch);

        // translation matrix
        matrix4x4 matTrans = Matrix_MakeTranslation(0.0f, 0.0f, 17.5f);

        // matrix that handles both rotations and translations before projection that will be multiplied with vector
        matrix4x4 matWorldTransformations = Matrix_MakeIdentity();
        matWorldTransformations = Matrix_MultiplyMatrix(matRotZ, matRotX);
        matWorldTransformations = Matrix_MultiplyMatrix(matWorldTransformations, matRotY);
        matWorldTransformations = Matrix_MultiplyMatrix(matWorldTransformations, matTrans);

        // used to offset item in screen space after projection (in pixels)
        vec3D postProjOffset = { 0, 50, 0 };

        // PLANE STUFF --------------------------------------------------------------------------------------------
        graph transformedGraph = transformPlane(graph3DPlane, matWorldTransformations);
        graph projectedGraph = projectPlane(transformedGraph, projectionMatrix, (float)ScreenWidth(), (float)ScreenHeight());
        graph postTransGraph = postProjTransPlane(projectedGraph, postProjOffset);

        DrawLine(postTransGraph.lines[0].point[0].x, postTransGraph.lines[0].point[0].y, postTransGraph.lines[0].point[1].x, postTransGraph.lines[0].point[1].y, olc::RED);
        for (int i = 3; i <= 23; i++) {
            DrawLine(postTransGraph.lines[i].point[0].x, postTransGraph.lines[i].point[0].y, postTransGraph.lines[i].point[1].x, postTransGraph.lines[i].point[1].y, olc::RED);
        }

        DrawLine(postTransGraph.lines[1].point[0].x, postTransGraph.lines[1].point[0].y, postTransGraph.lines[1].point[1].x, postTransGraph.lines[1].point[1].y, olc::GREEN);
        for (int i = 24; i <= 44; i++) {
            DrawLine(postTransGraph.lines[i].point[0].x, postTransGraph.lines[i].point[0].y, postTransGraph.lines[i].point[1].x, postTransGraph.lines[i].point[1].y, olc::GREEN);
        }

        DrawLine(postTransGraph.lines[2].point[0].x, postTransGraph.lines[2].point[0].y, postTransGraph.lines[2].point[1].x, postTransGraph.lines[2].point[1].y, olc::BLUE);
        for (int i = 45; i <= 65; i++) {
            DrawLine(postTransGraph.lines[i].point[0].x, postTransGraph.lines[i].point[0].y, postTransGraph.lines[i].point[1].x, postTransGraph.lines[i].point[1].y, olc::BLUE);
        }
        // --------------------------------------------------------------------------------------------------------

        // GRAPH STUFF --------------------------------------------------------------------------------------------
        
        // DO NOT evaluate points every frame
        // evaluate points only when needed (button click or startup)
        // this should only be for rendering 
        function transformedFn = transformPoints(graphFunction, matWorldTransformations);
        function projectedFn = projectPoints(transformedFn, projectionMatrix, (float)ScreenWidth(), (float)ScreenHeight());
        function postTransFn = postProjTransFunction(projectedFn, postProjOffset);

        if (postTransFn.points.size() > 1) {
            for (int i = 0; i < postTransFn.points.size() - 1; i++) {
                DrawLine(postTransFn.points[i].x, postTransFn.points[i].y, postTransFn.points[i].x, postTransFn.points[i].y, olc::WHITE);
            }
        }

        // if user is looking for that one "t", highlight it
        // lazy boundary checking and unoptimal methods because :)
        if (tPoint.active && (abs(tPoint.pos.x) < 10.0f &&
            abs(tPoint.pos.y) < 10.0f && abs(tPoint.pos.z) < 10.0f)) {

            vec3D transformedTPoint = Matrix_MultiplyVector(matWorldTransformations, tPoint.mathPos);
            vec3D projectedTPoint = Matrix_MultiplyVector(projectionMatrix, transformedTPoint);
            projectedTPoint = Vector_Div(projectedTPoint, projectedTPoint.z);

            vec3D vOffset = { 1.0f, 1.0f, 0.0f };
            projectedTPoint = Vector_Add(projectedTPoint, vOffset);

            projectedTPoint.x *= 0.5f * (float)ScreenWidth();
            projectedTPoint.y *= 0.5f * (float)ScreenHeight();

            vec3D postTransTPoint = Vector_Add(projectedTPoint, postProjOffset);

            FillCircle({ (int)postTransTPoint.x, (int)postTransTPoint.y }, 3, olc::YELLOW);
        }       
        // --------------------------------------------------------------------------------------------------------

        // GUI STUFF ----------------------------------------------------------------------------------------------
        updateGUI();
        // --------------------------------------------------------------------------------------------------------

        return true;
    }

    bool OnUserDestroy() override {
        return true;
    }

private:
    matrix4x4 projectionMatrix;

    // camera rotating;
    float fYaw = 0.0f; // rotate left/right
    float fPitch = 0.0f; // rotate up/down
    float fRoll = 0.0f; // rotate forward/backward

    float fTheta = 0.0f;

    pointAdvanced tPoint;
    graph graph3DPlane;
    function graphFunction;
    std::string expressions[3];

    std::unique_ptr<olc::Sprite> axesRef;

    olc::QuickGUI::Manager guiManager;
    olc::QuickGUI::Label* parametricFnLabel;
    olc::QuickGUI::TextBox* xTextInput;
    olc::QuickGUI::TextBox* zTextInput;
    olc::QuickGUI::TextBox* yTextInput;
    olc::QuickGUI::Button* calcBtn;
    olc::QuickGUI::Label* findPtLabel;
    olc::QuickGUI::TextBox* findPtTextInput;
    olc::QuickGUI::Button* findPtBtn;
    olc::QuickGUI::Label* ptPropertiesLabel;
    olc::QuickGUI::Label* invalidMsgLabel;

    void updateGUI() {
        // gray bar
        FillRect( 0, 0, ScreenWidth() - 1, 120, olc::DARK_GREY);
        // axis on bot left
        DrawSprite( olc::vi2d(0, 400), axesRef.get());
        // draw all gui stuff
        guiManager.DrawDecal(this);
        // top bar text
        DrawString({ 10, 10 }, "f(t) = <     ,     ,     >", olc::WHITE, 2);
        DrawString({ 500, 10 }, "At t = ", olc::WHITE, 2);
        // copyright (OLC)
        DrawString({ 480, 580 }, "Copyright 2018 - 2024 OneLoneCoder.com", olc::WHITE, 1);
    }

    void updateParametricPoints() {
        // vecXYZ evaluatedPts = parametric3DWithTInterval(getExpressions(), 0, 5, 0.1f);
        vecXYZ evaluatedPts = parametric3DWithMinMaxXYZ(getExpressions(), -10.0f, -10.0f, -10.0f, 10.0f, 10.0f, 10.0f, 0.01f);
        // note: must invert y (computer graphics -> mathematical conventions)
        for (int i = 0; i < evaluatedPts[0].size(); i++) {
            appendToGraphFunction({ (float)evaluatedPts[0][i], -1.0f * (float)evaluatedPts[1][i], (float)evaluatedPts[2][i] });
        }

        /*for (auto j : getGraphFunction().points) {
            std::cout << "( " << j.x << ", " << j.y << ", " << j.z << " )" << std::endl;
        }*/
    }


    // expression functions
    void setExpression(int index, std::string expression) { expressions[index] = expression; }
    void resetExpressions() { for (auto i : expressions) i = ""; }
    std::string* getExpressions() { return expressions; }

    // graph's function, functions
    void appendToGraphFunction(vec3D vec) { graphFunction.points.push_back(vec); }
    function getGraphFunction() { return graphFunction; }
    void resetGraphFunction() { graphFunction.points.clear(); }

    // point functions
    vec3D getGraphPointWithIndex(int index) { return graphFunction.points[index]; }
    // solves it on the spot instead of reading a value; its not optimal, but it works :)
    vec3D getGraphPointWithT(float t, std::string* expressions) { return { evaluateExpression<float>(expressions[0], t), evaluateExpression<float>(expressions[1], t), evaluateExpression<float>(expressions[2], t) };}
    pointAdvanced getPtData(float tToFind, std::string* expressions) { // reformat this to JUST to get pt data (use return to show label and point on graph)
        pointAdvanced pointOfInterest;

        pointOfInterest.pos = getGraphPointWithT(tToFind, expressions);
        pointOfInterest.deriv.x = pseudoDerivative(expressions[0], tToFind);
        pointOfInterest.deriv.y = pseudoDerivative(expressions[1], tToFind);
        pointOfInterest.deriv.z = pseudoDerivative(expressions[2], tToFind);
        pointOfInterest.derivMag = ((pointOfInterest.deriv.x * pointOfInterest.deriv.x) + (pointOfInterest.deriv.y * pointOfInterest.deriv.y) + (pointOfInterest.deriv.z * pointOfInterest.deriv.z));

        pointOfInterest.mathPos = { pointOfInterest.pos.x, pointOfInterest.pos.y * -1.0f, pointOfInterest.pos.z, 1.0f };

        return pointOfInterest;
    }

    void setPtDataLabel(pointAdvanced pt) {
        ptPropertiesLabel->sText = "t = " + findPtTextInput->sText + "\n\n"
            + "pos: <" + std::to_string(pt.pos.x) + ", " + std::to_string(pt.pos.y) + ", " + std::to_string(pt.pos.z) + ">" + "\n\n"
            + "f'(t): <" + std::to_string(pt.deriv.x) + ", " + std::to_string(pt.deriv.y) + ", " + std::to_string(pt.deriv.z) + ">" + "\n\n"
            + "|f'(t)|: " + std::to_string(pt.derivMag);
    }

    // type checking
    bool validExpressionInput(std::string* expressions) { 
        bool xOk = !isnan(evaluateExpression<double>(expressions[0], 0));
        bool yOk = !isnan(evaluateExpression<double>(expressions[1], 0));
        bool zOk = !isnan(evaluateExpression<double>(expressions[2], 0));
        return xOk && yOk && zOk; 
    }

    bool validNumInput(std::string t) { return isFloat(t); }

};

// fancy main to get rid of console
// is there a better way to do this? maybe.
// am i lazy? yes.
//int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
//{
//    // WinMainCRTStartup
//    FreeConsole();
//    olc3DEngine rasterizer3D;
//
//    if (rasterizer3D.Construct(800, 600, 1, 1)) {
//        rasterizer3D.Start();     
//    }
//
//    return 0;
//}

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

int main(int argc, char** argv)
{
    FreeConsole();
    olc3DEngine rasterizer3D;

    if (rasterizer3D.Construct(800, 600, 1, 1)) {
        rasterizer3D.Start();
    }

    return 0;
}

// sort by providing a condition in a lambda function beginning and ending parameters
// third parameter tells algorithm to sort in ascending order based on z values
// not completely perfect (based on average z, inaccuracy errors)
//std::sort(projectedGraph.begin(), projectedGraph.end(), [](line& t1, line& t2)
//    {
//        float avgZ1 = (t1.vertices[0].z + t1.vertices[1].z + t1.vertices[2].z) / 3.0f;
//        float avgZ2 = (t2.vertices[0].z + t2.vertices[1].z + t2.vertices[2].z) / 3.0f;
//        return avgZ1 > avgZ2;
//    }
//);
