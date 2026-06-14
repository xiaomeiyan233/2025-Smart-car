#pragma once
/**
 
 * @file catering.cpp
 * @author niu 
 * @brief 餐饮区
 * @version 0.1
 * @date 2025/03/03 09:53:17
 * @copyright  :Copyright (c) 2024
 *
 */

#include <fstream>
#include <iostream>
#include <cmath>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "../../include/common.hpp"
#include "../../include/detection.hpp"
#include "../recognition/tracking.cpp"
#include "../include/uart.hpp"
#include "../include/json.hpp"

extern std::shared_ptr<Uart> uart;
using namespace cv;
using namespace std;

class Catering
{
private:
    uint16_t counterSession = 0;    // 图像场次计数器
    uint16_t counterRec = 0;        // 汉堡标志检测计数器
    bool cateringEnable = false;    // 岔路区域使能标志
    bool burgerLeft = true;         // 汉堡在左侧
    bool burgerRight = true;        // 汉堡在右侧
    bool turning = true;            // 转向标志
    int burgerY = 0;                // 汉堡高度
    int truningTime = 25;           // 转弯时间 25帧
    int travelTime = 10;            // 行驶时间 10帧 在斜线路段的行驶时间
    int stopTime = 25;              // 停车时间 25帧
    POINT lp ;
    POINT rp;
public:

    struct Params 
    {
        uint16_t L_inside;
        uint16_t R_inside;
        uint16_t Cater_stoptime;
        uint16_t safe_L;
        uint16_t safe_R;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Params, L_inside, R_inside, Cater_stoptime, safe_L, safe_R);
    };

    Params params;

    Catering() 
    {
        std::ifstream f("../src/config/cater.json");
        if (!f) 
        {
            std::cout << "Error: Cannot open cater.json" << std::endl;
            exit(-1);
        }
        nlohmann::json j;
        f >> j;
        params = j.get<Params>();
    }





    bool stopEnable = false;        // 停车使能标志
    bool noRing = false;            // 用来区分环岛路段
    float cater_enter = 0;
    //bool cateringEnable = false;    // 多线程状态标志


    /**
    * @brief 餐饮店状态
    *
    */
    enum Cater
    {
        Cater_0 = 0,
        Cater_1,
        Cater_2,
        Cater_3,
        Cater_4

    };
    Cater cater = Cater::Cater_0;

    /**
    * @brief 餐饮店类型
    *
    */
   enum CaterType
   {
       None = 0,
       Cater_L,
       Cater_R

   };
   CaterType catertype = CaterType::None; 


    void cateringCheck(vector<PredictResult> predict) 
    {
        if (cateringEnable) return;
    
        bool foundBurger = false;
        float bestScore = 0.0;
        PredictResult bestBurger;
    
        for (const auto& p : predict) 
        {
            if (p.type == LABEL_BURGER && p.score > 0.4 && p.y > 10)
            {
                if (p.score > bestScore) {
                    bestScore = p.score;
                    bestBurger = p;
                    foundBurger = true;
                }
            }
        }
    
        if (foundBurger) 
        {
            counterRec++;
            noRing = true;
    
            if (bestBurger.x < COLSIMAGE / 2) 
            {
                std::cout << "检测到汉堡标志在左侧" << std::endl;
                catertype = CaterType::Cater_L;
            }
            else 
            {
                std::cout << "检测到汉堡标志在右侧" << std::endl;
                catertype = CaterType::Cater_R;
            }
        }
    
        counterSession++; // 每帧都加一
    
        if (counterRec >= 3 && counterSession < 9) 
        {
            counterRec = 0;
            counterSession = 0;
            cateringEnable = true;
            std::cout << "进入catering" << std::endl;
        } 
        else if (counterSession >= 13) 
        {
            counterRec = 0;
            counterSession = 0;
        }
    }
    
    //辅助函数1
    /**
     * @brief 搜索环岛岔路点（左下）
     *
     * @param pointStartBlock
     * @param pointEndBlock
     * @return uint16_t
     */
    uint16_t searchLeftP(vector<POINT>pointsEdgeLeft,
                                 uint16_t row_small, 
                                 uint16_t row_large)
                                 
    {    
        uint16_t rowBreakLeftDown = 0;
        for (int i = row_small ; i < row_large - 20 ; i++) // 寻找左边跳变点，从上往下
        {   
            if(pointsEdgeLeft[i].y > pointsEdgeLeft[rowBreakLeftDown].y ) 
            {
                rowBreakLeftDown = i ;
            
            }
            else if(pointsEdgeLeft[i].y < pointsEdgeLeft[rowBreakLeftDown].y )
            {
                
                        return rowBreakLeftDown;
                   
            }     
        }         
        return 0;
    } 

    //辅助函数3
    /**
     * @brief 搜索环岛岔路点（右下）
     *
     * @param pointStartBlock
     * @param pointEndBlock
     * @return uint16_t
     */
    uint16_t searchRightP(vector<POINT>pointsEdgeRight,
                                  uint16_t row_small, 
                                  uint16_t row_large)
                                  
        {

            uint16_t rowBreakRightDown = 0;
            uint16_t counter = 0;
            for (int i = row_small ; i < row_large - 20 ; i++) // 寻找左边跳变点，从上往下
            {   
                if(pointsEdgeRight[i].y < pointsEdgeRight[rowBreakRightDown].y ) 
                {
                    rowBreakRightDown = i ;
                    counter = 0;
                }
                else if(pointsEdgeRight[i].y > pointsEdgeRight[rowBreakRightDown].y )
                {
                    
                            return rowBreakRightDown;   
                }     
            }         
            return 0;
        }

    //处理进入餐饮区域的逻辑
    bool process(Tracking &track, Mat &image, vector<PredictResult> predict)
    {
        //cateringCheck(predict);
        if(!cateringEnable) return false; // 如果餐饮区未使能，直接返回
        if (cateringEnable && cater == Cater::Cater_0)
        {
            int count = 0;
            bool add = false;
            if(catertype == CaterType::Cater_L)//汉堡在左侧
            {
                /*********************************************************************/
                std::cout<<"left catering"<<std::endl; 
                for(int i = 0; i < 100; i++)//告诉下位机进入右餐饮区阶段，准备用陀螺仪
                {
                    static int a = 0;
                    if(uart != nullptr && a < 100)
                    {
                        uart->send_cater_enter(1);
                        std::cout<<"send success"<<std::endl; 
                        a++;
                    }
                }
                for(int i = 0; i < 10; i++)
                {
                    std::cout << "cater_enter =  " << uart->cater_enter << std::endl;
                }
                /*********************************************************************/
                for(int i = 0; i < track.pointsEdgeRight.size(); i++)
                {
                    if(track.pointsEdgeRight[track.pointsEdgeRight.size()-i].y > 300)
                    {
                       count++;
                       if( count > 70)
                       {
                           add = true;
                           
                       } 
                    }
                    if (add)
                    {
                        turning = true; // 开启转向标志
                        cater = Cater::Cater_1; // 进入补线状态
                    }
                }
            }
            else if(catertype == CaterType::Cater_R)//汉堡在右侧
            {
                /*********************************************************************/
                std::cout<<"right catering"<<std::endl; 
                for(int i = 0; i < 100; i++)//告诉下位机进入左餐饮区阶段，准备用陀螺仪
                {
                    static int a = 0;
                    if(uart != nullptr && a < 100)
                    {
                        uart->send_cater_enter(2);
                        std::cout<<"send success"<<std::endl; 
                        a++;
                    }
                 }
                 for(int i = 0;i<10;i++)
                 {
                    std::cout << "cater_enter =  " << uart->cater_enter << std::endl;
                 }
                /*********************************************************************/
                for(int i = 0; i < track.pointsEdgeLeft.size(); i++)
                {
                    if(track.pointsEdgeLeft[track.pointsEdgeLeft.size()-i].y < 10)
                    {
                       count++;
                       if(count > 70)
                       {
                           add = true;
                           
                       } 
                    }
                    if (add)
                    {
                        turning = true; // 开启转向标志
                        cater = Cater::Cater_1; // 进入补线状态}
                    }
                }
            }   
        }
        if (cater == Cater::Cater_1)
        {   
            if(catertype == CaterType::Cater_L)
            {   
                vector<POINT> smooth;
                POINT p1 =POINT (110,0);
                POINT p2 =POINT (55,160);
                POINT p3 =POINT (0,320);
                bezier3P(p1,p2 , p3, smooth);
                track.pointsEdgeLeft.clear();
                track.pointsEdgeLeft.insert(track.pointsEdgeLeft.end(), smooth.begin(), smooth.end());
               
                    
                
                /*********************************************************************/
                if(uart != nullptr)
                {
                    if(uart->cater_enter == 3 )
                    {
                        static int count_Cater_1 = 0; //count_Cater_1
                        count_Cater_1++;               // 每帧 +1
                        if(count_Cater_1 > params.L_inside) // 停车前持续时间L_inside = 10
                        cater = Cater::Cater_2; // 进入转向状态//切换下一个状态
                    }  
                }
                /*********************************************************************/
            }
            else if(catertype == CaterType::Cater_R)
            {   
                vector<POINT> smooth;
                POINT p1 =POINT (110,320);
                POINT p2 =POINT (55,160);
                POINT p3 =POINT (0,0);
                bezier3P(p1,p2 , p3, smooth);
                track.pointsEdgeRight.clear();
                track.pointsEdgeRight.insert(track.pointsEdgeRight.end(), smooth.begin(), smooth.end());
                
                
                /*********************************************************************/
                if(uart != nullptr)
                {
                    if(uart->cater_enter == 3 )
                    {   
                        static int count_Cater_1 = 0; // static 保持跨帧记忆
                        count_Cater_1++;               // 每帧 +1
                        if(count_Cater_1 > params.R_inside)// 停车前持续时间R_inside = 15
                        cater = Cater::Cater_2; // 进入转向状态//切换下一个状态
                    }  
                }
                /*********************************************************************/  


            }
            
        
        }
        if(cater == Cater::Cater_2)
        {   
            turning = false;   // 关闭转向标志
            stopEnable = true;  // 停车使能   
            static int count_Cater_2 = 0;  // static 保持跨帧记忆
            //static int count_Cater_3 = 0;  // static 保持跨帧记忆
            count_Cater_2++;               // 每帧 +1
            //count_Cater_3++;
            
            turning = false;   // 关闭转向标志
            stopEnable = true;  // 停车使能 

            

            if(count_Cater_2 >= params.Cater_stoptime)  //  = 10 30帧大约1秒（FPS=30）
            {
                stopEnable = false; // 解除停车
                cater = Cater::Cater_3; // 切到下一个状态
                count_Cater_2 = 0; // 重置
            }
        }
        if(cater == Cater::Cater_3)
        {
            if(catertype == CaterType::Cater_L)
            {
                
                /*uint16_t rowLeftP = searchLeftP(track.pointsEdgeLeft, 0, track.pointsEdgeLeft.size()); // 寻找左下点
                std::cout<<"rowLeftP: "<<rowLeftP<<std::endl;
                std::cout<<"rowLeftP: "<<rowLeftP<<std::endl;
                std::cout<<"rowLeftP: "<<rowLeftP<<std::endl;
                lp = POINT(track.pointsEdgeLeft[rowLeftP].x,track.pointsEdgeLeft[rowLeftP].y);
                if(rowLeftP < 100)
                return false;*/
                static int count_Cater_3 = 0;  // static 保持跨帧记忆
                count_Cater_3++;               // 每帧 +1

                if(count_Cater_3 >= params.safe_L)  // = 25  30帧大约1秒（FPS=30）
                {
                    
                    count_Cater_3 = 0; // 重置
                    cater = Cater::Cater_0; 
                    catertype = CaterType::None; // 重置餐饮类型
                    uart->cater_enter = 0; // 重置下位机状态
                    cateringEnable = false;
                    return false;
                }


            }
            else if(catertype == CaterType::Cater_R)
            {
                
                /*uint16_t rowRightP = searchRightP(track.pointsEdgeRight, 0, track.pointsEdgeRight.size()); // 寻找右下点
                std::cout<<"rowRightP: "<<rowRightP<<std::endl;
                std::cout<<"rowRightP: "<<rowRightP<<std::endl;
                std::cout<<"rowRightP: "<<rowRightP<<std::endl;
                rp = POINT(track.pointsEdgeRight[rowRightP].x,track.pointsEdgeRight[rowRightP].y);
                if(rowRightP < 100)
                return false;*/
                static int count_Cater_3 = 0;  // static 保持跨帧记忆
                count_Cater_3++;               // 每帧 +1

                if(count_Cater_3 >= params.safe_R)  //  safe_R = 5 30帧大约1秒（FPS=30）
                {
                    
                    count_Cater_3 = 0; // 重置
                    cater = Cater::Cater_0; 
                    catertype = CaterType::None; // 重置餐饮类型
                    uart->cater_enter = 0; // 重置下位机状态
                    cateringEnable = false;
                    return false;
                }

            }
            

        }

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
        {
            circle(image, Point(lp.y, lp.x), 5, Scalar(255, 0, 255), -1); // **上补线点：粉色**
            circle(image, Point(rp.y, rp.x), 5, Scalar(226, 43, 138), -1); // **中补线点：紫色**
            
        }
        if (cateringEnable)
            putText(image, "[1] Burger - ENABLE", Point(COLSIMAGE / 2 - 30, 10), cv::FONT_HERSHEY_TRIPLEX, 0.3, cv::Scalar(0, 255, 0), 1, CV_AA);
    }
    
};
