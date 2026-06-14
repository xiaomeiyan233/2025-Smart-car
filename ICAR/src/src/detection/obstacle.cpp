/**
 ********************************************************************************************************
 * @file obstacle.cpp
 * @brief 改良版：障碍区 AI 识别 + 补线 + 延时退出 + 延迟回正 + 置信度过滤（方案1）
 * @version 2025-06-21-modified
 ********************************************************************************************************/

 #include <fstream>
 #include <iostream>
 #include <cmath>
 #include <opencv2/highgui.hpp>
 #include <opencv2/opencv.hpp>
 #include "../../include/common.hpp"
 #include "../../include/detection.hpp"
 #include "../include/json.hpp"

 using namespace std;
 using namespace cv;

 class Obstacle
 {
 public:
     struct Params 
     {
         uint16_t EXIT_DELAY_FRAMES; // 右1车库入库时机

         NLOHMANN_DEFINE_TYPE_INTRUSIVE(Params, EXIT_DELAY_FRAMES);
     };

     Params params;

     Obstacle() 
     {
         std::ifstream f("../src/config/obstacle.json");
         if (!f) 
         {
             std::cout << "Error: Cannot open obstacle.json" << std::endl;
             exit(-1);
         }
         nlohmann::json j;
         f >> j;
         params = j.get<Params>();
     }

     // 置信度阈值、延时退出帧数
     static constexpr float CONF_THRESHOLD = 0.8f;
     static constexpr int HOLD_CENTER_FRAMES = 20;

     void obstacleCheck(const vector<PredictResult>& predict)
     {
         if (obstacleEnable) {
             if (counterExit > params.EXIT_DELAY_FRAMES * 2) 
             {
                 obstacleEnable = false;
                 counterExit = 0;
                 count_sign = 0;
             }
             return;
         }

         for (const auto& p : predict) {
             if (p.score > CONF_THRESHOLD &&
                 (p.type == LABEL_CONE || p.type == LABEL_BLOCK || p.type == LABEL_PEDESTRIAN) &&
                 (p.y + p.height) > 36)
             {
                 counterObstacle++;
                 break;
             }
         }

         if (counterObstacle > 0) {
             cout << "[ObstacleCheck] Detected!" << endl;
             obstacleEnable = true;
             counterExit = 0;
             counterObstacle = 0;
         } else {
             obstacleEnable = false;
         }
     }

     bool process(Tracking &track, const vector<PredictResult>& predict)
     {
         bool hasTarget = false;

         vector<PredictResult> resultsObs;
         for (const auto& p : predict) {
             if (p.score < CONF_THRESHOLD) continue;
             if ((p.type == LABEL_PEDESTRIAN && (p.y + p.height) > 53) ||
                 (p.type == LABEL_CONE       && (p.y + p.height) > 53) ||
                 (p.type == LABEL_BLOCK      && (p.y + p.height) > 53))
             {
                 resultsObs.push_back(p);
             }
         }

         if (!resultsObs.empty()) {
             int areaMax = 0, idx = 0;
             for (int i = 0; i < (int)resultsObs.size(); i++) {
                 int area = resultsObs[i].width * resultsObs[i].height;
                 if (area > areaMax) {
                     areaMax = area;
                     idx = i;
                 }
             }
             resultObs = resultsObs[idx];

             int extendY1 = 30, extendY2 = 20;
             int extendX1 = 25, extendX2 = 20;

             if (resultObs.type == LABEL_BLOCK) {
                 extendY1 = 50; extendY2 = 40; extendX1 = 35; extendX2 = 30;
             } else if (resultObs.type == LABEL_PEDESTRIAN || resultObs.type == LABEL_CONE) {
                 extendY1 = 20; extendY2 = 10; extendX1 = 15; extendX2 = 10;
             }

             int row = (int)track.pointsEdgeLeft.size() -
                       (resultObs.y + resultObs.height - track.rowCutUp);
             if (row >= 0) {
                 int disLeft  = resultObs.x + resultObs.width - track.pointsEdgeLeft[row].y;
                 int disRight = track.pointsEdgeRight[row].y - resultObs.x;

                 if (resultObs.x + resultObs.width > track.pointsEdgeLeft[row].y &&
                     track.pointsEdgeRight[row].y > resultObs.x &&
                     disLeft <= disRight)
                 {
                     vector<POINT> pts(4);
                     pts[0] = track.pointsEdgeLeft[row/2];
                     pts[1] = {resultObs.y + resultObs.height + extendY1,
                               resultObs.x + resultObs.width + extendX1};
                     pts[2] = {(resultObs.y + resultObs.height + resultObs.y)/2 + extendY2,
                               resultObs.x + resultObs.width + extendX2};
                     pts[3] = track.pointsEdgeLeft.back();
                     track.pointsEdgeLeft.resize(row/2);
                     auto repair = Bezier(0.01, pts);
                     track.pointsEdgeLeft.insert(track.pointsEdgeLeft.end(), repair.begin(), repair.end());
                     curtailTracking(track, false);
                 }
                 else if (resultObs.x + resultObs.width > track.pointsEdgeLeft[row].y &&
                          track.pointsEdgeRight[row].y > resultObs.x &&
                          disLeft > disRight)
                 {
                     vector<POINT> pts(4);
                     pts[0] = track.pointsEdgeRight[row/2];
                     pts[1] = {resultObs.y + resultObs.height + extendY1,
                               resultObs.x - resultObs.width + extendX1};
                     pts[2] = {(resultObs.y + resultObs.height + resultObs.y)/2 + extendY2,
                               resultObs.x - resultObs.width + extendX2};
                     pts[3] = track.pointsEdgeRight.back();
                     track.pointsEdgeRight.resize(row/2);
                     auto repair = Bezier(0.01, pts);
                     track.pointsEdgeRight.insert(track.pointsEdgeRight.end(), repair.begin(), repair.end());
                     curtailTracking(track, true);
                 }
             }

             obstacleEnable = true;
             counterExit = 0;
             if (holdCenterCounter == 0) {
                 if (resultObs.type == LABEL_BLOCK)
                     holdCenterCounter = 120;
                 else if (resultObs.type == LABEL_PEDESTRIAN || resultObs.type == LABEL_CONE)
                     holdCenterCounter = 15;
                 else
                     holdCenterCounter = HOLD_CENTER_FRAMES;
                 holdCenterY = (track.pointsEdgeLeft.back().y + track.pointsEdgeRight.back().y) / 2;
             }
             hasTarget = true;
         }

         if (!hasTarget && obstacleEnable) {
             counterExit++;
             if (counterExit > params.EXIT_DELAY_FRAMES) {
                 obstacleEnable = false;
                 counterExit = 0;
             }
         }

         enable = (hasTarget || obstacleEnable);
         return enable;
     }

     int getHoldControlCenter(int defaultCenter)
     {
         if (holdCenterCounter > 0) {
             holdCenterCounter--;
             return holdCenterY;
         }
         return defaultCenter;
     }

     void drawImage(Mat &img)
     {
         if (enable) {
             putText(img, "[Obstacle] ENABLE", Point(COLSIMAGE/2-30, 10),
                     FONT_HERSHEY_TRIPLEX, 0.4, Scalar(0,255,0), 1, CV_AA);
             Rect r(resultObs.x, resultObs.y, resultObs.width, resultObs.height);
             rectangle(img, r, Scalar(0,0,255), 1);
         }
     }

 private:
     bool enable = false;
     PredictResult resultObs;
     bool obstacleEnable = false;
     uint16_t counterObstacle = 0;
     uint16_t counterExit = 0;
     char count_sign = 0;

     int holdCenterCounter = 10;
     int holdCenterY = COLSIMAGE/2;

     void curtailTracking(Tracking &track, bool left)
     {
         if (left) {
             if (track.pointsEdgeRight.size() > track.pointsEdgeLeft.size())
                 track.pointsEdgeRight.resize(track.pointsEdgeLeft.size());
             for (size_t i = 0; i < track.pointsEdgeRight.size(); i++) {
                 track.pointsEdgeRight[i].y = track.pointsEdgeRight[i].y * 0.6f
                                           + track.pointsEdgeLeft[i].y * 0.4f - 60;
             }
         } else {
             if (track.pointsEdgeLeft.size() > track.pointsEdgeRight.size())
                 track.pointsEdgeLeft.resize(track.pointsEdgeRight.size());
             for (size_t i = 0; i < track.pointsEdgeLeft.size(); i++) {
                 track.pointsEdgeLeft[i].y = track.pointsEdgeLeft[i].y * 0.6f
                                           + track.pointsEdgeRight[i].y * 0.4f + 60;
             }
         }
     }
 };