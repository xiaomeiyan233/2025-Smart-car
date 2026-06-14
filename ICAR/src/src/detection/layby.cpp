#pragma once
/**
 * @file layby.cpp
 * @author HC (sasu@saishukeji.com)
 * @brief 临时停车区
 * @version 0.1
 * @date 2025/03/04 20:29:04
 * @copyright  :Copyright (c) 2024
 * @note 具体功能模块:
 */

 #include <fstream>
 #include <iostream>
 #include <cmath>
 #include <opencv2/highgui.hpp>
 #include <opencv2/opencv.hpp>
 #include "../../include/common.hpp"
 #include "../../include/detection.hpp"
 #include "../recognition/tracking.cpp"
 #include <opencv2/ximgproc.hpp>
 #include "../include/json.hpp"


using namespace cv;
using namespace std;

class Layby
{
public:


    struct Params 
    {

        uint16_t momentL; // 左停车时机
        uint16_t momentR; // 右停车时机

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Params, momentL,momentR);
    };
    Params params;

    Layby() 
    {
        std::ifstream f("../src/config/layby.json");
        if (!f) 
        {
            std::cout << "Error: Cannot open layby.json" << std::endl;
            exit(-1);
        }
        nlohmann::json j;
        f >> j;
        params = j.get<Params>();      
        //用于检测图像中的直线段（线条）  
        lsd = cv::ximgproc::createFastLineDetector();
    }


    bool stopEnable = false;        // 停车使能标志
    uint16_t moment;   // 当前生效的 moment
    Ptr<cv::ximgproc::FastLineDetector> lsd;

    //只检测图像中虚线 y = 150 到 y = 230 的部分
    int roi_y_start = 150;
    int roi_height = 70;
    float lsd_length_thresh = 30; //只有长度 ≥ 40 像素的线段
    int lsd_frame_consistency = 1;//只要 1 帧看到虚线就停车
    int valid_line_frames = 0;    //当前连续帧中检测到有效线段的计数器

    
    void laybyCheck(vector<PredictResult> predict) 
    {
        if (laybyEnable) return;

        for (const auto& p : predict) {
            if (((p.type == LABEL_SCHOOL || p.type == LABEL_COMPANY) && p.score > 0.6) /*&& (p.y + p.height) > ROWSIMAGE * 0.3*/) {
                counterRec++;
                leftEnable = (p.x < COLSIMAGE / 2);//检测左 or 右标志
                break;
            }
        }

        if (counterRec) counterSession++;

        if (counterRec >= 2 && counterSession < 8) {
            counterRec = 0;
            counterSession = 0;
            laybyEnable = true;
            counterS = 0 ;
        } else if (counterSession >= 8) {
            counterRec = 0;
            counterSession = 0;
            counterS = 0 ;
        }
    }


    bool process(Tracking &track, Mat &image, vector<PredictResult> predict)
    {
        if (!laybyEnable) return false;
    
        // 根据标志牌方向切换 moment
        if (leftEnable) 
            moment = params.momentL;
        else 
            moment = params.momentR;
        
    
        counterS++;
        std::cout << "时间: " << counterS << " | moment=" << moment << std::endl;
    
        // === ROI + LSD 检测替代原 Hough ===
        // 你可以保留单车道缩线（可选）
        curtailTracking(track, leftEnable);
    
        //ROI 区域
        Rect roi_rect(0, roi_y_start, COLSIMAGE, roi_height);
        Mat roi = image(roi_rect).clone();
    
        //LSD 检测
        vector<Vec4f> lsd_lines;
        lsd->detect(roi, lsd_lines);
    
        //预筛选 + 还原 ROI 坐标
        vector<Vec4i> filtered_lines;
        for (auto &line : lsd_lines) {
            float x1 = line[0], y1 = line[1], x2 = line[2], y2 = line[3];
            float dx = x2 - x1, dy = y2 - y1;
            float length = sqrt(dx*dx + dy*dy);
            float slope = fabs(dy / (dx + 1e-5));
            if (length >= lsd_length_thresh && slope < 0.5) {
                filtered_lines.push_back(Vec4i(
                    int(x1), int(y1) + roi_y_start,
                    int(x2), int(y2) + roi_y_start
                ));
            }
        }
    
        //合并线段
        mergedLines.clear();
        sort(filtered_lines.begin(), filtered_lines.end(),
             [](const Vec4i &a, const Vec4i &b) {
                 return (a[1] + a[3]) / 2 < (b[1] + b[3]) / 2;
             });
        for (const Vec4i &line : filtered_lines) {
            int midY = (line[1] + line[3]) / 2;
            bool shouldMerge = false;
            for (Vec4i &merged : mergedLines) {
                int y1 = (merged[1] + merged[3]) / 2;
                if (abs(y1 - midY) < 20) {
                    int minX = min(min(merged[0], merged[2]), min(line[0], line[2]));
                    int maxX = max(max(merged[0], merged[2]), max(line[0], line[2]));
                    int avgY = (y1 + midY) / 2;
                    merged[0] = minX;
                    merged[1] = avgY;
                    merged[2] = maxX;
                    merged[3] = avgY;
                    shouldMerge = true;
                    break;
                }
            }
            if (!shouldMerge) mergedLines.push_back(line);
        }
    
        //连帧一致性
        if (!mergedLines.empty()) valid_line_frames++;
        else valid_line_frames = 0;
    
//停车时机判断   使用 counterS + moment + searchingLine 控制
        if (counterS > moment + 20)
        {
            counterS = 0;
            counterSession = 0;
            laybyEnable = false;
            stopEnable = false;
            searchingLine = false;
        }
        else if (mergedLines.size() >= 1 && valid_line_frames >= lsd_frame_consistency) 
        {
            if (counterS > moment) 
            {

                for(int i = 0;i<10;i++)
                {
                    std::cout<<"=============开始临时停车=============="<<std::endl;
                }
                stopEnable = true;
            }
        }
    
        return true;
    }
     

    /**
     * @brief 识别结果图像绘制
     *
     */
    void drawImage(Tracking track, Mat &image)
    {
        // 赛道边缘
        for (size_t i = 0; i < track.pointsEdgeLeft.size(); i++)
        {
            circle(image, Point(track.pointsEdgeLeft[i].y, track.pointsEdgeLeft[i].x), 1,
                   Scalar(0, 255, 0), -1); // 绿色点
        }
        for (size_t i = 0; i < track.pointsEdgeRight.size(); i++)
        {
            circle(image, Point(track.pointsEdgeRight[i].y, track.pointsEdgeRight[i].x), 1,
                   Scalar(0, 255, 255), -1); // 黄色点
        }
        
        // 绘制合并后的结果
        for(const Vec4i &line : mergedLines) 
        {
            cv::line(image, Point(line[0], line[1]),Point(line[2], line[3]), Scalar(0, 0, 255), 2);
        }
        
        if (laybyEnable)
            putText(image, "[1] Layby - ENABLE", Point(COLSIMAGE / 2 - 30, 10), cv::FONT_HERSHEY_TRIPLEX, 0.3, cv::Scalar(0, 255, 0), 1, CV_AA);
    }

    /**
     * @brief 缩减优化车道线（双车道→单车道）
     *
     * @param track
     * @param left
     */
    void curtailTracking(Tracking &track, bool left)
    {
        if (left) // 向左侧缩进
        {
            if (track.pointsEdgeRight.size() > track.pointsEdgeLeft.size())
                track.pointsEdgeRight.resize(track.pointsEdgeLeft.size());

            for (size_t i = 0; i < track.pointsEdgeRight.size(); i++)
            {
                track.pointsEdgeRight[i].y = (track.pointsEdgeRight[i].y * 0.55+ track.pointsEdgeLeft[i].y * 0.45) ;
            }
        }
        else // 向右侧缩进
        {
            if (track.pointsEdgeRight.size() < track.pointsEdgeLeft.size())
                track.pointsEdgeLeft.resize(track.pointsEdgeRight.size());

            for (size_t i = 0; i < track.pointsEdgeLeft.size(); i++)
            {
                track.pointsEdgeLeft[i].y = (track.pointsEdgeRight[i].y *0.45 + track.pointsEdgeLeft[i].y *0.55) ;
            }
        }
    }
private:

    uint16_t counterSession = 0;    // 图像场次计数器
    uint16_t counterRec = 0;        // 标识牌检测计数器
    bool laybyEnable = false;       // 临时停车区域使能标志
    bool leftEnable = true;         // 标识牌在左侧
    bool searchingLine = false;     // 搜索直线标志
    vector<Vec4i> mergedLines;      // 合并后的线段
    int stopTime = 250;             // 停车时间 40帧
    int counterS = 0;
};








