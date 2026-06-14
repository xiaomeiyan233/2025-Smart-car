 #pragma once
/**1、识别环岛，根据左右边缘斜率判断环岛类型

 * @file ring.cpp
 * @author Niu
 * @brief 环岛识别（基于track赛道识别后）
 * @version 0.1
 * @date 2025-04-01
 *
 * 
 *
 * @note  环岛识别步骤（ringStep）：
 *          1：环岛识别（初始化）
 *          2：入环处理
 *          3：环中处理
 *          4：出环处理
 *          5：出环结束
 */

#include <fstream>
#include <iostream>
#include <cmath>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "../../include/common.hpp"
#include "tracking.cpp"
#include "../include/uart.hpp"

extern std::shared_ptr<Uart> uart;

using namespace cv;
using namespace std;


class Ring
{
    public:
    uint16_t counterShield = 0; // 环岛检测屏蔽计数器
    public:
    POINT _corner;
    uint16_t counterSpurroad = 0; // 岔路计数器
    uint16_t ring_cnt = 0; // 环岛检测确认计数器
	uint16_t counterExit = 0;	  // 异常退出计数器
    float ring_entering = 0; // 入环标志

    POINT pointBreakU;
    POINT pointBreakM;
    POINT pointBreakD;
    vector<POINT> pointsEdgeLeftLast;  // 记录前一行左边缘点集
    vector<POINT> pointsEdgeRightLast; // 记录前一行右边缘点集

    public: 

    /**
     * @brief 环岛类型
     *
     */
    enum RingType
    {
        RingNone = 0, // 未知类型
        RingLeft,     // 左入环岛
        RingRight     // 右入环岛
    };


    RingStep ringStep = RingStep::None;   // 环岛运行阶段
    RingType ringType = RingType::RingNone; // 环岛类型

    public:
    Ring()
    {
        reset();
    }
    /*
        @ brief 环岛入环步骤
    */
   enum Ring_Straight
    {
        Before_M = 0, // 中间点前
        In_M,         // 中间点
        Test_M,
        After_M        // 中间点后

    };
    Ring_Straight ringStraight = Ring_Straight::Before_M; 
    //辅助函数1
    /**
     * @brief 搜索环岛岔路点（左下）
     *
     * @param pointStartBlock
     * @param pointEndBlock
     * @return uint16_t
     */
    uint16_t searchBreakLeftDown(vector<POINT>pointsEdgeLeft,
                                 uint16_t row_small, 
                                 uint16_t row_large)
                                 
    {    
        uint16_t rowBreakLeftDown = 0;
        uint16_t counter = 0;
        for (int i = row_small ; i < row_large - 20 ; i++) // 寻找左边跳变点，从上往下
        {   
            if(pointsEdgeLeft[i].y > pointsEdgeLeft[rowBreakLeftDown].y ) 
            {
                rowBreakLeftDown = i ;
                counter = 0;
            
            }
            else if(pointsEdgeLeft[i].y < pointsEdgeLeft[rowBreakLeftDown].y )
            {
                for (int n = 0; n < 14; n++)
                {   
                    if(pointsEdgeLeft[i+n].y < 2)
                        counter++;
                    if(counter > 13)
                        return rowBreakLeftDown;
                }
                   
            }     
        }         
        return 0;
    } 


    //辅助函数2
    /**
     * @brief 搜索环岛岔路点（左上）
     *
     * @param pointStartBlock
     * @param pointEndBlock
     * @return uint16_t
     */
    uint16_t searchBreakLeftUp(vector<POINT>pointsEdgeLeft,
                               uint16_t row_small,
                               uint16_t row_large)
                               
    {   
        uint16_t rowBreakLeftUp = 0;
        for (int i = row_small ; i < row_large - 1; i++) // 按数组序号找
        {
            if(pointsEdgeLeft[rowBreakLeftUp].y < 5)
            {
                rowBreakLeftUp = i;
                
            }
            else if(pointsEdgeLeft[i].y >= 60 && rowBreakLeftUp >= 120 && abs(pointsEdgeLeft[i].y - pointsEdgeLeft[i-3].y) > 30)
            {
                    rowBreakLeftUp = i;
                    return rowBreakLeftUp;
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
    uint16_t searchBreakRightDown(vector<POINT>pointsEdgeRight,
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
                    for (int n = 0; n < 14; n++)
                    {   
                        if(pointsEdgeRight[i+n].y > 300)
                            counter++;
                        if(counter > 13)
                            return rowBreakRightDown;
                    }
                    
                }     
            }         
            return 0;
        }
    
        

        //辅助函数4
    /**
     * @brief 搜索环岛岔路点（右上）
     *
     * @param pointStartBlock
     * @param pointEndBlock
     * @return uint16_t
     */
    uint16_t searchBreakRightUp(vector<POINT>pointsEdgeRight,
                                uint16_t row_small, 
                                uint16_t row_large)
                                
        {
            uint16_t rowBreakRightUp = 0;
            for (int i = row_small ; i < row_large - 1; i++) // 按数组序号找
            {
                if(pointsEdgeRight[rowBreakRightUp].y > 300 )
                {
                    rowBreakRightUp = i;
                }
                else if(pointsEdgeRight[i].y <= 260 && rowBreakRightUp >= 120 )//&& abs(pointsEdgeRight[i].y - pointsEdgeRight[i-3].y) > 30
                {
                    rowBreakRightUp = i;
                    return rowBreakRightUp;
                }  
            }  
            return 0;
        }

        //辅助函数5
        /**
         * @brief 判断左侧是否突变
         */
        bool PointsEdgeboardLeft(Tracking &track)
        {
            int counter=0;
            if(track.pointsEdgeLeft.size()>30)
            {
                for(int i = track.pointsEdgeLeft.size()-1;i>5;i--)
                {
                    if(abs(track.pointsEdgeLeft[i].y - track.pointsEdgeLeft[i-2].y )>5)
                    {
                        counter++;
                        if(counter>2)
                        {
                            counter=0;
                            return true;
                        }
                    }
                }
            }
            else
            {
                return false;
            }
        }

        //辅助函数6
        /**
         * @brief 判断右侧是否突变
         */
        bool PointsEdgeboardRight(Tracking &track)
        {
            int counter=0;
            if(track.pointsEdgeRight.size()>30)
            {
                for(int i = track.pointsEdgeRight.size()-1;i>5;i--)
                {
                    if(abs(track.pointsEdgeRight[i].y - track.pointsEdgeRight[i-2].y) >5)
                    {
                        counter++;
                        if(counter>2)
                        {
                            counter=0;
                            return true;
                        }
                    }
                }
            }
            else
            {
                return false;
            }
        }


    //辅助函数7
    /**
     * @brief 环岛识别初始化|复位
     *
     */
    void reset(void)
    {
        ringType = RingType::RingNone; // 环岛类型
        ringStep = RingStep::None;     // 环岛处理阶段
        ringStraight = Ring_Straight::Before_M; 
        ring_cnt = 0;
        counterSpurroad = 0;
        counterShield = 0;
        /*
        uart->ring_inside = 0;
        uart->ring_enter = 0;
        uart->ring_finish = 0;
        */
    }


    //辅助函数8
    /**
     * @brief 打印环岛状态
     *
     */
    void print(void)
    {
        cout << "RingStep: " << ringStep << endl;
    }


    //辅助函数9
    /** 
     * @brief 左环圆切点 
     */
    uint16_t searchrowLeftCircle(vector<POINT> pointsEdgeLeft,uint16_t row_small,uint16_t row_large)
    {
        uint16_t rowLeftCircle = 0;//数组序号
        int counter = 0;
        int count = 0;
        for (int i = row_small; i < row_large-20; i++)
        {
            if(pointsEdgeLeft[i].y > pointsEdgeLeft[rowLeftCircle].y )
            {
                rowLeftCircle = i;
                counter = 0;
                  
            }
            else if(pointsEdgeLeft[i].y < pointsEdgeLeft[rowLeftCircle].y )
            {   
                for (int n = 0; n < 20; n++)
                {   
                    if(pointsEdgeLeft[i+n].y > 30)
                        counter++;
                    if(counter > 17)
                        count++;
                    if (count > 3)//确保稳定性
                        return rowLeftCircle;   
                }
                
            }
        }
        return 0;
    }
//辅助函数10
    /** 
     * @brief 右环圆切点 
     */
    uint16_t searchrowRightCircle(vector<POINT> pointsEdgeRight,uint16_t row_small,uint16_t row_large)
    {
        uint16_t rowRightCircle = 0;//数组序号
        int counter = 0;
        int count = 0;
        for (int i = row_small; i < row_large-20; i++)
        {
            if(pointsEdgeRight[i].y < pointsEdgeRight[rowRightCircle].y )
            {
                rowRightCircle = i;
                counter = 0;
                  
            }
            else if(pointsEdgeRight[i].y > pointsEdgeRight[rowRightCircle].y && pointsEdgeRight[i+1].y > pointsEdgeRight[i].y)
            {
                for (int n = 0; n < 20; n++)
                {   
                    if(pointsEdgeRight[i+n].y < 290)
                        counter++;
                    if(counter > 17)
                        count++;
                    if (count > 3)//确保稳定性
                        return rowRightCircle;   
                }
            }
        }
        return 0;
    }                               
//主函数
/**
     * @brief 环岛识别与行径规划
     *
     * @param track 基础赛道识别结果
     * @param imagePath 赛道路径图像
     */
    bool process(Tracking &track, Mat img_bin)
    {   
        if (track.pointsEdgeRight.size() < ROWSIMAGE / 4 
            || track.pointsEdgeLeft.size() < ROWSIMAGE / 4) //环岛有效行限制，环岛有效行小于1/4则不识别环岛
        {
            return ringType;
        }
        //环岛屏蔽计数器小于八次则不识别环岛，防止环岛识别过于频繁
        if (counterShield < 8)
        {
            counterShield++;
            return false;
        }
    
        pointBreakD = POINT(0, 0); 
        pointBreakU = POINT(0, 0);
        std::cout<<"the begining of ring"<<std::endl;
        std::cout<<"track.stdevLeft: "<<track.stdevLeft<<std::endl;
        std::cout<<"track.stdevRight: "<<track.stdevRight<<std::endl;
        uint16_t rowBreakLeftD = searchBreakLeftDown(track.pointsEdgeLeft,
                                                     40,
                                                     track.pointsEdgeLeft.size());//寻找左下点
        uint16_t rowBreakRightD = searchBreakRightDown(track.pointsEdgeRight,
                                                       40,
                                                       track.pointsEdgeRight.size());//寻找右下点
//1、判断左右环岛
        if(ringType == RingType::RingNone 
           && track.stdevLeft > 150 //左边是环岛
           && track.stdevRight < 20 //右边是直线
           )
        {
            //std::cout<<"ring STEP 1-1"<<std::endl;
            uint16_t counter = 0, step = 2;
            int counterExit=0;
            uint16_t rowBreakLeftDown = searchBreakLeftDown(track.pointsEdgeLeft,
                                                            0,
                                                            track.pointsEdgeLeft.size());
            std::cout<<"rowBreakLeftDown: "<<rowBreakLeftDown<<std::endl;
            for(int i = 0; i < track.pointsEdgeLeft.size(); i++)//从左环岛角点到图片底
            {
                
                uint16_t counterBLD =  rowBreakLeftDown;//得到突变点的序号
                if(track.pointsEdgeLeft[counterBLD].y < 100)//突变点里左边缘线很近，故以左边缘点代替
                {
                    counter++;
                        
                }
                else
                {
                    if(step % 2 == 0)
                        counter = 0;
                    else if(step % 2 == 1)
                        counter++;
                }
                if(counter > 3)
                {
                    step += 1;
                    counter = 0;
                    if(step >= 3 && rowBreakLeftDown != 0)
                    {
                        ringType = RingType::RingLeft;
                        track.RingStatus = 1;
                        std::cout<<"ringleft"<<std::endl;
                        ringStep = RingStep::None;
                        ringStraight == Ring_Straight::Before_M;
                    }
                }
            }
        }
        else if(ringType == RingType::RingNone 
                && track.stdevLeft < 25 
                && track.stdevRight > 150 )
        {
                //std::cout<<"ring STEP 1-1"<<std::endl;
                static  int counter = 0;
                uint16_t counter_2 = 0;
                uint16_t counter_3 = 0;
                uint16_t rowBreakRightDown = searchBreakRightDown(track.pointsEdgeRight,
                                                                  0, 
                                                                  track.pointsEdgeRight.size());
                std::cout<<"rowBreakRightDown: "<<rowBreakRightDown<<std::endl;
                if(track.pointsEdgeRight[rowBreakRightDown].y > 110 && rowBreakRightDown !=0)
                {
                    counter++;    
                }
                for(int i = 0; i < track.pointsEdgeRight.size(); i++)
                {
                    if(track.pointsEdgeRight[i].y > 310 )
                    {
                        counter_2++;
                    }
                }
                for(int i = 80; i < track.pointsEdgeLeft.size(); i++)
                {
                    if(track.pointsEdgeLeft[i].y < 10 )
                    {
                        counter_3++;
                    }
                } 
                if(counter > 2 && counter_2 >= 20 && counter_3 < 5)
                {
                    counter = 0;
                    counter_2 = 0;
                    ringType = RingType::RingRight;
                    track.RingStatus = 2;
                    std::cout<<"ringringt"<<std::endl;
                    ringStep = RingStep::None;
                        
                }
                        
        }
//2、入环识别阶段
        if(ringType != RingType::RingNone && ringStep == RingStep::None)
        {
            if(ringType == RingType::RingLeft)
            {   
                
                uint16_t rowBreakLeftD = searchBreakLeftDown(track.pointsEdgeLeft,
                                                            40,
                                                            track.pointsEdgeLeft.size());//寻找左下点
                uint16_t rowBreakLeftM = searchrowLeftCircle(track.pointsEdgeLeft,
                                                            40,
                                                            track.pointsEdgeLeft.size());//寻找左环圆切点
                

                uint16_t rowBreakLeftU = searchBreakLeftUp(track.pointsEdgeLeft,
                                                           40,
                                                           track.pointsEdgeLeft.size());//寻找左上点
       
                pointBreakD = track.pointsEdgeLeft[rowBreakLeftD];//把左下点赋值给pointBreakD
                pointBreakM = track.pointsEdgeLeft[rowBreakLeftM];//把左环圆切点赋值给pointMiddle 
                pointBreakU = track.pointsEdgeLeft[rowBreakLeftU];//把左上点赋值给pointBreakU
                std::cout<<"rowBreakLeftD: "<<rowBreakLeftD<<std::endl;
                std::cout<<"rowBreakLeftM: "<<rowBreakLeftM<<std::endl;
                std::cout<<"rowBreakLeftU: "<<rowBreakLeftU<<std::endl;
            
                if(ringStraight == Ring_Straight::Before_M )
                {   
                    std::cout<<"rowBreakLeftD: "<<rowBreakLeftD<<std::endl;
                    std::cout<<"rowBreakLeftM: "<<rowBreakLeftM<<std::endl;
                    //track.pointsEdgeLeft = track.predictEdgeLeft(track.pointsEdgeRight);
                    if(rowBreakLeftM != 0)
                    {
                        line(track.pointsEdgeLeft, rowBreakLeftD, rowBreakLeftM);
                    }
                    if(rowBreakLeftD <= 30)
                    {
                        rowBreakLeftD = 0;
                    }
                    if(rowBreakLeftD == 0)
                    {   
                        ringStraight = Ring_Straight::Test_M;
                        std::cout<<"____________________________________________Test_M______________________________________"<<std::endl;
                        std::cout<<"____________________________________________Test_M______________________________________"<<std::endl;
                        std::cout<<"____________________________________________Test_M______________________________________"<<std::endl;
                        std::cout<<"____________________________________________Test_M______________________________________"<<std::endl;
                        std::cout<<"____________________________________________Test_M______________________________________"<<std::endl;
                        std::cout<<"____________________________________________Test_M______________________________________"<<std::endl;
                
                    }   
                }
                else if(ringStraight == Ring_Straight::Test_M && rowBreakLeftM > 150)
                {
                    ringStraight = Ring_Straight::In_M;
                    //track.pointsEdgeLeft = track.predictEdgeLeft(track.pointsEdgeRight);
                }
                else if(ringStraight == Ring_Straight::In_M)
                {   
                    std::cout<<"rowBreakLeftM: "<<rowBreakLeftM<<std::endl;
                    //track.pointsEdgeLeft = track.predictEdgeLeft(track.pointsEdgeRight);
                    if(rowBreakLeftM < 100 && rowBreakLeftM != 0)
                    {    
                        ringStraight = Ring_Straight::After_M;                                             
                        std::cout<<"________________________________________After_M_____________________________________"<<std::endl;
                        std::cout<<"________________________________________After_M_____________________________________"<<std::endl;
                        std::cout<<"________________________________________After_M_____________________________________"<<std::endl;
                        std::cout<<"________________________________________After_M_____________________________________"<<std::endl;
                        std::cout<<"________________________________________After_M_____________________________________"<<std::endl;
                        std::cout<<"________________________________________After_M_____________________________________"<<std::endl;
                            
                    }
                }    
                                 
                else if(ringStraight == Ring_Straight::After_M)
                {   
                    //pointBreakM = track.pointsEdgeLeft[rowBreakLeftM];//把左环圆切点赋值给pointMiddle
                    //pointBreakU = track.pointsEdgeLeft[rowBreakLeftU];//把左上点赋值给pointBreakU
                    std::cout<<"rowBreakLeftM: "<<rowBreakLeftM<<std::endl;
                    std::cout<<"rowBreakLeftU: "<<rowBreakLeftU<<std::endl;
                    //track.pointsEdgeLeft = track.predictEdgeLeft(track.pointsEdgeRight);
                    if(rowBreakLeftU > 130 )
                    {
                        ringStep = RingStep::Entering;
                        std::cout<<"_________________________________Entering__________________________________"<<std::endl;
                        std::cout<<"_________________________________Entering__________________________________"<<std::endl;
                        std::cout<<"_________________________________Entering__________________________________"<<std::endl;
                        std::cout<<"_________________________________Entering__________________________________"<<std::endl;
                        std::cout<<"_________________________________Entering__________________________________"<<std::endl;
                        std::cout<<"_________________________________Entering__________________________________"<<std::endl;
                    }
                }
                else
                {
                    //track.pointsEdgeLeft = track.predictEdgeLeft_2(track.pointsEdgeRight);
                    std::cout<<"_________________________________else__________________________________"<<std::endl;
                    std::cout<<"_________________________________else__________________________________"<<std::endl;
                    std::cout<<"_________________________________else__________________________________"<<std::endl;
                    std::cout<<"_________________________________else__________________________________"<<std::endl;
                    std::cout<<"_________________________________else__________________________________"<<std::endl;
                }    
                
            }
            else if(ringType == RingType::RingRight)
            {
                uint16_t rowBreakRightD = searchBreakRightDown(track.pointsEdgeRight,
                                                               40,
                                                               track.pointsEdgeRight.size());//寻找右下点
                                                               

                uint16_t rowBreakRightM = searchrowRightCircle(track.pointsEdgeRight,
                                                               40,
                                                               track.pointsEdgeRight.size());//寻找右环圆切点

                uint16_t rowBreakRightU = searchBreakRightUp(track.pointsEdgeRight,
                                                             40,
                                                             track.pointsEdgeRight.size());//寻找右上点
                                                             
                pointBreakD = track.pointsEdgeRight[rowBreakRightD];//把右下点赋值给pointBreakD
                pointBreakM = track.pointsEdgeRight[rowBreakRightM];//把右环圆切点赋值给pointMiddle 
                pointBreakU = track.pointsEdgeRight[rowBreakRightU];//把右上点赋值给pointBreakU
                std::cout<<"rowBreakRightD: "<<rowBreakRightD<<std::endl;
                std::cout<<"rowBreakRightM: "<<rowBreakRightM<<std::endl;
                std::cout<<"rowBreakRightU: "<<rowBreakRightU<<std::endl;
                vector<POINT> smooth;
                    POINT p1 =POINT (240,310);
                    POINT p2 =POINT (120,265);
                    POINT p3 =POINT (0,210);
                    bezier3P(p1,p2 , p3, smooth);
                    track.pointsEdgeRight.clear();
                    track.pointsEdgeRight.insert(track.pointsEdgeRight.end(), smooth.begin(), smooth.end());
                
                if(ringStraight == Ring_Straight::Before_M )
                {   
                    std::cout<<"rowBreakRightD: "<<rowBreakRightD<<std::endl;
                    std::cout<<"rowBreakRightM: "<<rowBreakRightM<<std::endl;
                    //track.pointsEdgeRight = track.predictEdgeRight(track.pointsEdgeLeft);
                    if(rowBreakRightM != 0)
                    {
                        line(track.pointsEdgeRight, rowBreakRightD, rowBreakRightM);
                    }
                    
                    if(rowBreakRightD <= 45)
                    {   
                        rowBreakRightD = 0;
                    }
                    if(rowBreakRightD == 0)
                    {   
                        ringStraight = Ring_Straight::Test_M;
                        std::cout<<"____________________________________________Test_M______________________________________"<<std::endl;
                        std::cout<<"____________________________________________Test_M______________________________________"<<std::endl;
                        std::cout<<"____________________________________________Test_M______________________________________"<<std::endl;
                        std::cout<<"____________________________________________Test_M______________________________________"<<std::endl;
                        std::cout<<"____________________________________________Test_M______________________________________"<<std::endl;
                        std::cout<<"____________________________________________Test_M______________________________________"<<std::endl;
                
                    }  
                }
                if(ringStraight == Ring_Straight::Test_M && rowBreakRightM > 160)
                {
                    //track.pointsEdgeRight = track.predictEdgeRight(track.pointsEdgeLeft);
                    ringStraight = Ring_Straight::In_M;
                    
                }
                if(ringStraight == Ring_Straight::In_M )
                {   
                    std::cout<<"rowBreakRightM: "<<rowBreakRightM<<std::endl;
                    //track.pointsEdgeRight = track.predictEdgeRight(track.pointsEdgeLeft);
                    if((rowBreakRightM <= 110 && rowBreakRightM != 0)||(rowBreakRightD > 60 && rowBreakRightD <= 90))
                    {   
                        track.pointsEdgeRight = track.predictEdgeRight(track.pointsEdgeLeft); 
                        ringStraight = Ring_Straight::After_M;                                             
                        std::cout<<"________________________________________After_M_____________________________________"<<std::endl;
                        std::cout<<"________________________________________After_M_____________________________________"<<std::endl;
                        std::cout<<"________________________________________After_M_____________________________________"<<std::endl;
                        std::cout<<"________________________________________After_M_____________________________________"<<std::endl;
                        std::cout<<"________________________________________After_M_____________________________________"<<std::endl;
                        std::cout<<"________________________________________After_M_____________________________________"<<std::endl;
                            
                    }
                }    
                                 
                else if(ringStraight == Ring_Straight::After_M )
                {   
                    //pointBreakM = track.pointsEdgeLeft[rowBreakLeftM];//把左环圆切点赋值给pointMiddle
                    //pointBreakU = track.pointsEdgeLeft[rowBreakLeftU];//把左上点赋值给pointBreakU
                    std::cout<<"rowBreakRightM: "<<rowBreakRightM<<std::endl;
                    std::cout<<"rowBreakRightU: "<<rowBreakRightU<<std::endl;
                    //track.pointsEdgeRight = track.predictEdgeRight(track.pointsEdgeLeft);
                    if(rowBreakRightU > 130)
                        ringStep = RingStep::Entering;
                        std::cout<<"_________________________________Entering__________________________________"<<std::endl;
                        std::cout<<"_________________________________Entering__________________________________"<<std::endl;
                        std::cout<<"_________________________________Entering__________________________________"<<std::endl;
                        std::cout<<"_________________________________Entering__________________________________"<<std::endl;
                        std::cout<<"_________________________________Entering__________________________________"<<std::endl;
                        std::cout<<"_________________________________Entering__________________________________"<<std::endl;
                }
                else
                {
                    //track.pointsEdgeRight = track.predictEdgeRight(track.pointsEdgeLeft);
                    
                    std::cout<<"_________________________________else__________________________________"<<std::endl;
                    std::cout<<"_________________________________else__________________________________"<<std::endl;
                    std::cout<<"_________________________________else__________________________________"<<std::endl;
                    std::cout<<"_________________________________else__________________________________"<<std::endl;
                    std::cout<<"_________________________________else__________________________________"<<std::endl;
                }                 
            }
        }
//3、入环补线阶段
        if(ringStep == RingStep::Entering)
        {
            std::cout<<"ring ______________________________________________of________________________________________________ Entering"<<std::endl; 
            
            if(ringType == RingType::RingLeft)
            {
               for(int i = 0; i < 2; i++)//告诉下位机进入环岛阶段，准备用陀螺仪
                {
                    ring_entering = 1;
                    if(uart != nullptr)
                    {
                        uart->send_ring_enter(1);
                        std::cout<<"send success"<<std::endl; 
                    }
                        
                 }
                uint16_t rowBreakLeftU = searchBreakLeftUp(track.pointsEdgeLeft,
                                                           60,
                                                           track.pointsEdgeLeft.size());//寻找左上点
                
                
                pointBreakU = track.pointsEdgeLeft[rowBreakLeftU];
                if(rowBreakLeftU != 0  )
                {   
                    pointBreakU = POINT(track.pointsEdgeLeft[rowBreakLeftU].x+20,track.pointsEdgeLeft[rowBreakLeftU].y-15);//把左上点赋值给pointBreakU
                    POINT controlPoint_1 = POINT((0.5*track.pointsEdgeRight[0].x + 0.5*pointBreakU.x)-20, (0.5*track.pointsEdgeRight[0].y + 0.5*pointBreakU.y)+30);
                    POINT endpoint_1 = POINT(track.pointsEdgeLeft[rowBreakLeftU].x, 0);
                    vector<POINT> smoothLeft_1;
                    bezier4P(track.pointsEdgeRight[0], controlPoint_1, pointBreakU, endpoint_1, smoothLeft_1);
                    track.pointsEdgeRight.clear();
                    track.pointsEdgeLeft.clear();
                    track.pointsEdgeRight.insert(track.pointsEdgeRight.end(), smoothLeft_1.begin(), smoothLeft_1.end());
                    track.pointsEdgeLeft = track.predictEdgeLeft(track.pointsEdgeRight);
                }

                
                
                else
                {   
                    POINT endpoint_2 = POINT(track.pointsEdgeLeft[110].x , 0);
                    POINT controlPoint_2 = POINT((0.5*track.pointsEdgeRight[0].x + 0.5*endpoint_2.x)-30, (0.5*track.pointsEdgeRight[0].y + 0.5*endpoint_2.y)+30);    
                    vector<POINT> smoothLeft_2;
                    bezier3P(track.pointsEdgeRight[10], controlPoint_2, endpoint_2, smoothLeft_2);
                    track.pointsEdgeRight.clear();
                    track.pointsEdgeLeft.clear();
                    track.pointsEdgeRight.insert(track.pointsEdgeRight.end(), smoothLeft_2.begin(), smoothLeft_2.end());
                    track.pointsEdgeLeft = track.predictEdgeLeft(track.pointsEdgeRight);    
                    

                }
                if(uart != nullptr)
                {
                    std::cout<<"uart != nullptr"<<std::endl;
                    std::cout << "ring_inside的值为 " << uart->ring_inside << std::endl;
                    if(uart->ring_inside != 0)
                   {
                    ringStep = RingStep::Inside;
                    std::cout<<"ring____________________________ of________________________________Inside"<<std::endl;
                    
                   }
                }     
            }
            else if(ringType == RingType::RingRight)
            {
                for(int i = 0; i < 2; i++)//告诉下位机进入环岛阶段，准备用陀螺仪
                {
                    ring_entering = 1;
                    if(uart != nullptr)
                    {
                        uart->send_ring_enter(1);
                        std::cout<<"send success"<<std::endl; 
                    }
                        
                 }
                uint16_t rowBreakRightU = searchBreakRightUp(track.pointsEdgeRight,
                                                             60,
                                                             track.pointsEdgeRight.size());//寻找右上点
                
                
                pointBreakU = track.pointsEdgeRight[rowBreakRightU];
                if(rowBreakRightU != 0  )
                {   
                    pointBreakU = POINT(track.pointsEdgeRight[rowBreakRightU].x+10,track.pointsEdgeRight[rowBreakRightU].y);//把右上点赋值给pointBreakU
                    POINT controlPoint_3 = POINT((0.5*track.pointsEdgeLeft[0].x + 0.5*pointBreakU.x)-20, (0.5*track.pointsEdgeLeft[0].y + 0.5*pointBreakU.y)-30);
                    POINT endpoint_3 = POINT(track.pointsEdgeRight[rowBreakRightU].x, 320);
                    vector<POINT> smoothLeft_3;
                    bezier4P(track.pointsEdgeLeft[10], controlPoint_3, pointBreakU, endpoint_3, smoothLeft_3);
                    track.pointsEdgeRight.clear();
                    track.pointsEdgeLeft.clear();
                    track.pointsEdgeLeft.insert(track.pointsEdgeLeft.end(), smoothLeft_3.begin(), smoothLeft_3.end());
                    track.pointsEdgeRight = track.predictEdgeRight(track.pointsEdgeLeft);
                }

                else
                {
                   
                    POINT endpoint_4 = POINT(track.pointsEdgeRight[100].x , 320);
                    POINT controlPoint_4 = POINT((0.5*track.pointsEdgeLeft[0].x + 0.5*endpoint_4.x)-60, (0.5*track.pointsEdgeLeft[0].y + 0.5*endpoint_4.y)-60);    
                    vector<POINT> smoothLeft_4;
                    bezier3P(track.pointsEdgeLeft[10], controlPoint_4, endpoint_4, smoothLeft_4);
                    track.pointsEdgeRight.clear();
                    track.pointsEdgeLeft.clear();
                    track.pointsEdgeLeft.insert(track.pointsEdgeLeft.end(), smoothLeft_4.begin(), smoothLeft_4.end());
                    track.pointsEdgeRight = track.predictEdgeRight(track.pointsEdgeLeft);   
                    

                }
                if(uart != nullptr)
                {
                    std::cout<<"uart != nullptr"<<std::endl;
                    std::cout << "ring_inside的值为 " << uart->ring_inside << std::endl;
                    if(uart->ring_inside != 0)
                   {
                    ringStep = RingStep::Inside;
                    std::cout<<"ring____________________________ of________________________________Inside"<<std::endl;
                    
                   }
                }


            }
        }
        if(ringStep == RingStep::Inside)
            {
                if(ringType == RingType::RingLeft)
                {
                    if(uart != nullptr)
                    {   

                        std::cout<<"uart != nullptr"<<std::endl;
                        std::cout << "ring_finish的值为 " << uart->ring_finish << std::endl;
                        if(track.pointsEdgeRight.size() < 50)
                        {
                            POINT startpoint_1 = POINT (200,310);
                            POINT endpoint_5 = POINT( 100, 0);
                            POINT controlPoint_5 = POINT (110,160);
                            vector<POINT> smoothLeft_5;
                            bezier3P(startpoint_1, controlPoint_5, endpoint_5, smoothLeft_5);
                            track.pointsEdgeRight.clear();
                            track.pointsEdgeRight.insert(track.pointsEdgeRight.end(), smoothLeft_5.begin(), smoothLeft_5.end());
                            track.pointsEdgeLeft = track.predictEdgeLeft(track.pointsEdgeRight);

                        }
                        
                        if(uart->ring_finish != 0)
                        {
                            ringStep =  RingStep::Finish;
                            std::cout<<"ring_____________ of_____________ Finish"<<std::endl;
                        }
                    }
                }
                else if(ringType == RingType::RingRight)
                {
                    if(uart != nullptr)
                    {   
                        std::cout<<"uart != nullptr"<<std::endl;
                        std::cout << "ring_finish的值为 " << uart->ring_finish << std::endl;
                        if(track.pointsEdgeLeft.size() < 50)
                        {
                            POINT startpoint_3 = POINT (200,10);
                            POINT endpoint_7 = POINT( 80, 320);
                            POINT controlPoint_7 = POINT (110,160);
                            vector<POINT> smoothLeft_7;
                            bezier3P(startpoint_3, controlPoint_7, endpoint_7, smoothLeft_7);
                            track.pointsEdgeLeft.clear();
                            track.pointsEdgeLeft.insert(track.pointsEdgeLeft.end(), smoothLeft_7.begin(), smoothLeft_7.end());
                            track.pointsEdgeRight = track.predictEdgeRight(track.pointsEdgeLeft);

                        }
                        
                        if(uart->ring_finish != 0)
                        {
                            ringStep =  RingStep::Finish;
                            std::cout<<"ring_____________ of_____________ Finish"<<std::endl;
                        }
                    }


                }

            }
        if(ringStep == RingStep::Finish)
            {
                std::cout<<"ring_____________ Exit!!!"<<std::endl;
                reset();
                uart->ring_inside = 0;
                uart->ring_enter = 0;
                uart->ring_finish = 0;
                
            }
        if(ringType == RingType::RingNone)
            return false;
        else
            return true;
         
    }
 
    void drawImage(Tracking track, Mat &Image) 
    {   
        // **绘制左侧赛道边界点*  **绿色点（左边缘）**
        for (int i = 0; i < track.pointsEdgeLeft.size(); i++) {

            circle //OpenCV 提供的图像绘制函数，每次在图像上画一个圆
            ( 
                Image,                             //要在上面画圆的图像
                Point(track.pointsEdgeLeft[i].y,track.pointsEdgeLeft[i].x), //圆心坐标，注意是 (列，行)
                2,                                 //半径为 2 像素
                Scalar(0, 255, 0),                 //颜色：绿色 (BGR 颜圆心（-1 表示实心）色空间)
                -1                                 //填充
            );
        }

        // **绘制右侧赛道边界点*   * 黄色点（右边缘）**
        for (int i = 0; i < track.pointsEdgeRight.size(); i++) 
        {
            circle(Image, Point(track.pointsEdgeRight[i].y, track.pointsEdgeRight[i].x), 2, 
                Scalar(0, 255, 255), -1);
        }

        // **绘制岔路点（较大红点）** track.spurroad 是一个 std::vector<POINT> 类型的容器，用来存储当前图像中检测到的岔路口点
        /*for (int i = 0; i < track.spurroad.size(); i++) 
        {
            circle(Image, Point(track.spurroad[i].y, track.spurroad[i].x), 3, Scalar(0, 0, 255), -1);
        }

        // **绘制拐角点（大红点** 图像中出现岔路 → spurroad 中加入点 → 在 Entering 阶段选出一个角点_corner→ drawImage函数中把_corner标出来

        circle(Image, Point(_corner.y, _corner.x), 6, Scalar(0, 0, 255), -1);*/

        // **绘制补线点**（就是上面入环阶段找到的上下突变点）
        {
            circle(Image, Point(pointBreakU.y, pointBreakU.x), 5, Scalar(255, 0, 255), -1); // **上补线点：粉色**
            circle(Image, Point(pointBreakM.y, pointBreakM.x), 5, Scalar(0, 255, 255), -1); // **中补线点：青色**
            circle(Image, Point(pointBreakD.y, pointBreakD.x), 5, Scalar(226, 43, 138), -1); // **下补线点：紫色**
        }

        // **绘制环岛状态**
        putText(
            Image,                      // 要绘制文字的图像
            to_string(ringStep),        // 要显示的文字内容（这里是环岛阶段的数字）
            Point(COLSIMAGE / 2 - 5, ROWSIMAGE - 40), // 文字起始位置（在图像下方中间偏左）
            cv::FONT_HERSHEY_TRIPLEX,   // 字体样式
            0.5,                        // 字体大小缩放因子
            Scalar(0, 0, 155),          // 字体颜色（深红）
            1,                          // 线宽
            CV_AA                       // 抗锯齿绘制（Anti-Aliasing）
        );

        // **如果检测到左环岛**
        if (ringType == RingType::RingLeft) 
        {
            putText(Image, "Ring L", Point(COLSIMAGE / 2 - 5, 20), cv::FONT_HERSHEY_TRIPLEX, 0.5, 
                    Scalar(0, 255, 0), 1, CV_AA); // **显示赛道识别类型**
        }
        // **如果检测到右环岛**
        else if (ringType == RingType::RingRight) 
        {
            putText(Image, "Ring R", Point(COLSIMAGE / 2 - 5, 20), cv::FONT_HERSHEY_TRIPLEX, 0.5, 
                    Scalar(0, 255, 0), 1, CV_AA); // **显示赛道识别类型**
        }

        // **显示右侧赛道检测数据**
        putText(Image, to_string(track.validRowsRight) + " " + to_string(track.stdevRight), 
                Point(COLSIMAGE - 100, ROWSIMAGE - 50), FONT_HERSHEY_TRIPLEX, 0.3, Scalar(0, 0, 255), 1);

        // **显示左侧赛道检测数据**
        putText(Image, to_string(track.validRowsLeft) + " " + to_string(track.stdevLeft), 
                Point(20, ROWSIMAGE - 50), FONT_HERSHEY_TRIPLEX, 0.3, Scalar(0, 0, 255), 1);
    }

};