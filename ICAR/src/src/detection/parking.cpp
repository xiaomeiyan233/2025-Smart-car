#pragma once
#include <fstream>
#include <iostream>
#include <cmath>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "../../include/common.hpp"
#include "../../include/detection.hpp"
#include "../recognition/tracking.cpp"
#include "../include/json.hpp"

using namespace cv;
using namespace std;

/**
 * @brief 充电停车场检测（支持双库 + 左右镜像）
 */
class Parking
{
public:


    struct Params 
    {

        uint16_t Right_turning1; // 右1车库入库时机
        uint16_t Right_turning2; // 右2车库入库时机
        uint16_t Right_turningtime1; // 右1库入库时间
        uint16_t Right_turningtime2; // 右2库入库时间

        uint16_t Left_turning1; // 左1车库入库时机
        uint16_t Left_turning2; // 左2车库入库时机
        uint16_t Left_turningtime1; // 左1库入库时间
        uint16_t Left_turningtime2; // 左2库入库时间

        uint16_t stopTime;       // 停车时间
        uint16_t trackoutTime;   // 出库时间

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Params, Right_turning1, Right_turning2, Right_turningtime1, Right_turningtime2, 
                                               Left_turning1,Left_turning2,Left_turningtime1,Left_turningtime2,
                                               stopTime, trackoutTime);
    };

    Params params;

    Parking() 
    {
        std::ifstream f("../src/config/parking.json");
        if (!f) 
        {
            std::cout << "Error: Cannot open parking.json" << std::endl;
            exit(-1);
        }
        nlohmann::json j;
        f >> j;
        params = j.get<Params>();
    }


    /**
     * @brief 停车步骤
     *
     */
    enum ParkStep
    {
        none = 0,  // 初始状态
        enable,    // 停车场使能
        before_turning,
        find_garget, // 寻找入库点
        turning,   // 入库转向
        stop,      // 停车
        trackout,   // 倒车出库
        exit_protect // 出库保护
    };

    POINT pointD;
    POINT pointU;

    ParkStep step = ParkStep::none; // 当前步骤
    //辅助函数1
    /**
     * @brief 搜索停车场岔路点（左下）
     *
     * @param pointStartBlock
     * @param pointEndBlock
     * @return uint16_t
     */
    uint16_t search_LeftDown(vector<POINT>pointsEdgeLeft,
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
     * @brief 搜索停车场岔路点（右下）
     *
     * @param pointStartBlock
     * @param pointEndBlock
     * @return uint16_t
     */
    uint16_t search_RightDown(vector<POINT>pointsEdgeRight,
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

    //辅助函数3
    /**
     * @brief 搜索环岛岔路点（右上）从上往下找
     *
     * @param pointStartBlock
     * @param pointEndBlock
     * @return uint16_t
     */
    uint16_t search_RightUp(vector<POINT>pointsEdgeRight,
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
                else if(pointsEdgeRight[i].y <= 260  && abs(pointsEdgeRight[i].y - pointsEdgeRight[rowBreakRightUp].y) > 20 && i > 130)
                {
                    rowBreakRightUp = i;
                    return rowBreakRightUp;
                }  
            }  
            return 0;
        }

    //辅助函数4
    /**
     * @brief 搜索停车场岔路点（左上）
     *
     * @param pointStartBlock
     * @param pointEndBlock
     * @return uint16_t
     */
    uint16_t search_LeftUp(vector<POINT>pointsEdgeLeft,
        uint16_t row_small,
        uint16_t row_large)       
    {   
        uint16_t rowBreakLeftUp = 0;
        for (int i = row_small ; i < row_large - 1; i++) // 按数组序号找
        {
            if(pointsEdgeLeft[rowBreakLeftUp].y <= 20)
            {
                rowBreakLeftUp = i;
            }
            else if(pointsEdgeLeft[i].y > 40 && i >= 120 )//&& abs(pointsEdgeLeft[i].y - pointsEdgeLeft[i-1].y) >= 20
            {
                rowBreakLeftUp = i;
                return rowBreakLeftUp;
            }  
        }
        return 0;
    }

    /**
     * @brief AI标志检测，进入 enable 状态
     */
    void parkingCheck(const vector<PredictResult> &predict)
    {
        if (step != ParkStep::none) return;

        for (const auto &p : predict)
        {
            if (p.type == LABEL_BATTERY && p.score > 0.4)
            {
                counterRec++;
                break;
            }
        }

        if (counterRec) counterSession++;

        if (counterRec >= 2 && counterSession < 15)
        {
            counterRec = 0;
            counterSession = 0;
            step = ParkStep::enable;
            cout << "进入停车场" << endl;
        }
        else if (counterSession >= 8)
        {
            counterRec = 0;
            counterSession = 0;
        }
    }

    /**
     * @brief 状态机主处理
     */
    bool process(Tracking &track, Mat &image, vector<PredictResult> predict)
    {
        switch (step)
        {
            case ParkStep::enable:
            {
                int carY = ROWSIMAGE;
                int batteryY = ROWSIMAGE;
                int batteryX = COLSIMAGE;
                for (const auto &p : predict)
                {
                    if (p.type == LABEL_CAR && p.score > 0.6)
                        carY = (p.y + p.height) / 2;
                    else if (p.type == LABEL_BATTERY && p.score > 0.6)
                        batteryY = p.y;
                        batteryX = p.x;
                }

                if(batteryX > COLSIMAGE/2)//判断左右停车场
                {
                    RightEnable = true;
                    for(int i = 0;i<10;i++)
                    {
                        std::cout<<"=============right parking=============="<<std::endl;
                    }
                    step = ParkStep::before_turning;
                    break;
                }
                else
                {
                    RightEnable = false;
                    for(int i = 0;i<10;i++)
                    {
                        std::cout<<"=============left parking=============="<<std::endl;
                    }
                    step = ParkStep::before_turning;
                    break;
                }
                break;
            }

            case ParkStep::before_turning:
            {
                if(RightEnable == true)
                {
                    uint16_t row_RightDown = search_RightDown(track.pointsEdgeRight,
                                                                    0, 
                                                                    track.pointsEdgeRight.size());
                    pointD = track.pointsEdgeRight[row_RightDown];
                    if(row_RightDown > 150)
                    {
                        step = ParkStep::find_garget; // 寻找入库点
                    }

                }
                else//左边
                {
                    uint16_t row_LeftDown = search_LeftDown(track.pointsEdgeLeft,
                                                            0, 
                                                            track.pointsEdgeLeft.size());
                    pointD = track.pointsEdgeLeft[row_LeftDown];
                    if(row_LeftDown > 150)
                    {
                        step = ParkStep::find_garget; // 寻找入库点
                    }
                }
               break;
            }

            case ParkStep::find_garget: // 寻找入库点
            {
                //继续寻找蓝车的坐标
                int carY = ROWSIMAGE;
                for (const auto &p : predict)
                {
                    if (p.type == LABEL_CAR && p.score > 0.6)
                        carY = (p.y + p.height) / 2;
                }
                if(RightEnable == true)
                {
                    uint16_t row_RightDown = search_RightDown(track.pointsEdgeRight,
                                                                                 0, 
                                                        track.pointsEdgeRight.size());
                    pointD = track.pointsEdgeRight[row_RightDown];
                    static int count_car = 0;
                    std::cout<<"row_RightDown: "<<row_RightDown<<std::endl;
                    if(row_RightDown < 100 && row_RightDown != 0 )//这个时间开始比较蓝车与down点的大小来判断入哪个库
                    {
                        if(carY != 240) 
                        {
                            for(int i = 0;i<10;i++)
                            {
                                std::cout<<"carY:"<<carY<<std::endl;
                            } 
                            if(carY > 40)  
                            {
                                garageFirst = false;//二号库
                                for(int i = 0;i<10;i++)
                                {
                                    std::cout<<"=============garaget 2=============="<<std::endl;
                                }
                                step = ParkStep::turning;
                                pointD = POINT(0,0);
                                break;
                            }
                            else if(carY < 40)
                            {
                                garageFirst = true;//一号库
                                for(int i = 0;i<10;i++)
                                {
                                    std::cout<<"=============garaget 1=============="<<std::endl;
                                }
                                step = ParkStep::turning;
                                pointD = POINT(0,0);
                                break;
                            }

                        } 
                    }
                    if(row_RightDown < 50 && row_RightDown != 0 )//这个时间点还没有检测到车，就判断为空车库
                    {
                        garageFirst = true;//一号库
                        for(int i = 0;i<10;i++)
                        {
                            std::cout<<"=============garaget 1=============="<<std::endl;
                        }
                        step = ParkStep::turning;
                        pointD = POINT(0,0);
                        break;                        
                    }
                }
                else if(RightEnable == false)
                {
                    uint16_t row_LeftDown = search_LeftDown(track.pointsEdgeLeft,
                                                            0, 
                                                            track.pointsEdgeLeft.size());
                    pointD = track.pointsEdgeLeft[row_LeftDown];            
                    static int find_count = 0;
                    static int find_flag = 0;       
                    std::cout<<"row_LeftDown: "<<row_LeftDown<<std::endl;
                    if(row_LeftDown < 80 && row_LeftDown != 0 )//这个时间开始比较蓝车与down点的大小来判断入哪个库
                    {
                        find_count++;
                        if(carY != 240) 
                        {     
                            find_count = 0;//找到蓝车后计数清零                   
                            for(int i = 0;i<10;i++)
                            {
                                std::cout<<"carY:"<<carY<<std::endl;
                            } 
                            if(carY > 30)  
                            {
                                garageFirst = false;//二号库
                                for(int i = 0;i<10;i++)
                                {
                                    std::cout<<"=============garaget 2=============="<<std::endl;
                                }
                                step = ParkStep::turning;
                                pointD = POINT(0,0);
                                break;
                            }
                            else if(carY < 30)
                            {
                                garageFirst = true;//一号库
                                for(int i = 0;i<10;i++)
                                {
                                    std::cout<<"=============garaget 1=============="<<std::endl;
                                }
                                step = ParkStep::turning;
                                pointD = POINT(0,0);
                                break;
                            }
                        }
                        if(find_count > 5)//连续5帧没看到蓝车
                        {
                            garageFirst = true;//一号库
                            for(int i = 0;i<10;i++)
                            {
                                std::cout<<"=============garaget 1=============="<<std::endl;
                            }
                            step = ParkStep::turning;
                            pointD = POINT(0,0);
                            break;
                            }
                    }
                    //左空车位判断（不稳定）
                    /*if(row_LeftDown < 20 && row_LeftDown != 0 )
                    {
                        garageFirst = false;//二号库
                        for(int i = 0;i<10;i++)
                        {
                            std::cout<<"=============garaget 2=============="<<std::endl;
                        }
                        step = ParkStep::turning;
                        pointD = POINT(0,0);
                        break;                        
                    }*/
                }
                
               break;
            }

            case ParkStep::turning: // 入库转向
            {
                if(RightEnable == true)
                {
                    if(garageFirst == true)  //一号车库的入库时机靠D点
                    {
                        if(!flag_pairline)
                        {
                            //继续找dowm点
                            uint16_t row_RightDown = search_RightDown(track.pointsEdgeRight,
                                                                            0, 
                                                                            track.pointsEdgeRight.size());
                            pointD = track.pointsEdgeRight[row_RightDown];
                            std::cout<<"row_RightDown: "<<row_RightDown<<std::endl;
                            //这个时机开始找补线 Right_turning1
                            if(row_RightDown < params.Right_turning1 && row_RightDown!=0)
                            {
                                pointD = POINT(0,0);
                                flag_pairline = true;       
                            }
                        }
                        if(flag_pairline)
                        {
                            for(int i = 0;i<10;i++)
                            {
                                std::cout<<"=============pair line=============="<<std::endl;
                            }
                            //右一车库补线
                            vector<POINT> smooth;
                            POINT p1 =POINT (110,0);
                            POINT p2 =POINT (65,160);
                            POINT p3 =POINT (45,320);
                            bezier3P(p1,p2 , p3, smooth);
                            track.pointsEdgeLeft.clear();
                            track.pointsEdgeLeft.insert(track.pointsEdgeLeft.end(), smooth.begin(), smooth.end());

/*                          vector<POINT> smooth2;
                            POINT p1_1 =POINT (50,310);
                            POINT p2_2 =POINT (120,310);
                            POINT p3_3 =POINT (240,310);
                            bezier3P(p1_1 ,p2_2 , p3_3, smooth2);
                            track.pointsEdgeRight.clear();
                            track.pointsEdgeRight.insert(track.pointsEdgeRight.end(), smooth2.begin(), smooth2.end());
*/
                            static int count_rightgarget_first = 0; // static 保持跨帧记忆
                            count_rightgarget_first++;               // 每帧 +1
                            if(count_rightgarget_first > params.Right_turningtime1)     //入1号库时间持续20帧 Right_turningtime1 = 20
                            {
                                count_rightgarget_first = 0;
                                flag_pairline = false;
                                step = ParkStep::stop; //切换下一个状态
                                break;
                            }
                        }
                    }
                    else if(garageFirst == false) //二号车库的入库时机靠UP点
                    {
                        if(!flag_pairline)
                        {
                            //找up点
                            uint16_t row_RightUp = search_RightUp(track.pointsEdgeRight,
                                                                            0, 
                                                                            track.pointsEdgeRight.size());
                            pointU = track.pointsEdgeRight[row_RightUp];
                            std::cout<<"row_Rightup: "<<row_RightUp<<std::endl;
                            if(row_RightUp > 185)
                            {
                                flag = true;
                                for(int i = 0;i<10;i++)
                                {
                                    std::cout<<"=============flag = true=============="<<std::endl;
                                }
                            }
                            if(flag)
                            {
                                if (row_RightUp < params.Right_turning2 && row_RightUp != 0 ) // 二号车库的补线时机（二号库入库时机）row_RightUp < 180 && !row_RightUp
                                {
                                    flag_pairline = true;
                                    pointU = POINT(0,0);
                                    flag = false; // 重置标志位
                                }
                            }
                        }
                        if(flag_pairline)
                        {
                            for(int i = 0;i<10;i++)
                            {
                                std::cout<<"=============pair line=============="<<std::endl;
                            }
                            //右二车库补线
                            vector<POINT> smooth;
                            POINT p1 =POINT (110,0);
                            POINT p2 =POINT (65,160);
                            POINT p3 =POINT (45,320);
                            bezier3P(p1,p2 , p3, smooth);
                            track.pointsEdgeLeft.clear();
                            track.pointsEdgeLeft.insert(track.pointsEdgeLeft.end(), smooth.begin(), smooth.end());

/*                          vector<POINT> smooth2;
                            POINT p1_1 =POINT (0,310);
                            POINT p2_2 =POINT (120,310);
                            POINT p3_3 =POINT (240,310);
                            bezier3P(p1_1 ,p2_2 , p3_3, smooth2);
                            track.pointsEdgeRight.clear();
                            track.pointsEdgeRight.insert(track.pointsEdgeRight.end(), smooth2.begin(), smooth2.end());
*/
                            static int count_rightgarget_second = 0; // static 保持跨帧记忆
                            count_rightgarget_second++;               // 每帧 +1
                            if(count_rightgarget_second > params.Right_turningtime2)    //入二号库时间持续27帧 = Right_turningtime2
                            {
                                count_rightgarget_second = 0;
                                step = ParkStep::stop; //切换下一个状态
                                flag_pairline = false;
                                break;
                            }
                        }
                    }   
                }
                else if(RightEnable == false)
                {
                    if(garageFirst == true)  //一号车库的入库时机靠D点
                    {
                        if(!flag_pairline)
                        {
                            //继续找dowm点
                            uint16_t row_LeftDown = search_LeftDown(track.pointsEdgeLeft,
                                                                    0, 
                                                                    track.pointsEdgeLeft.size());
                            pointD = track.pointsEdgeLeft[row_LeftDown];
                            std::cout<<"row_LeftDown: "<<row_LeftDown<<std::endl;
                            //这个时机开始找补线 Left_turning1
                            if(row_LeftDown <= params.Left_turning1 && row_LeftDown!=0)
                            {
                                flag_pairline = true; // 设置标志位                            
                                pointD = POINT(0,0);       
                            }
                        }
                        if(flag_pairline) // 如果标志位为真，说明已经找到了入库时机
                        {
                            for(int i = 0;i<10;i++)
                            {
                                std::cout<<"=============pair line=============="<<std::endl;
                            }
                            //左一车库补线
                            vector<POINT> smooth;
                            POINT p1 =POINT (170,320);
                            POINT p2 =POINT (85,160);
                            POINT p3 =POINT (0,130);
                            bezier3P(p1,p2 , p3, smooth);
                            track.pointsEdgeRight.clear();
                            track.pointsEdgeRight.insert(track.pointsEdgeRight.end(), smooth.begin(), smooth.end());
    
                            vector<POINT> smooth2;
                            POINT p1_1 =POINT (240,10);
                            POINT p2_2 =POINT (120,10);
                            POINT p3_3 =POINT (0,10);
                            bezier3P(p1_1 ,p2_2 , p3_3, smooth2);
                            track.pointsEdgeLeft.clear();
                            track.pointsEdgeLeft.insert(track.pointsEdgeLeft.end(), smooth2.begin(), smooth2.end());
    
                            static int count_leftgarget_first = 0; // static 保持跨帧记忆
                            count_leftgarget_first++;               // 每帧 +1
    
                            if(count_leftgarget_first > params.Left_turningtime1)     //入1号库时间持续20帧 Right_turningtime1 = 20
                            {
                                count_leftgarget_first = 0;
                                flag_pairline = false; // 重置标志位
                                step = ParkStep::stop; //切换下一个状态
                                break; // 跳出循环，进入下一个状态
                            }
    
                        }
                    }
                    else if(garageFirst == false) //二号车库的入库时机靠UP点
                    {
                        if(!flag_pairline)
                        {
                            //找up点
                            uint16_t row_LeftUp = search_LeftUp(track.pointsEdgeLeft,
                                                                0, 
                                                                track.pointsEdgeLeft.size());
                            pointU = track.pointsEdgeLeft[row_LeftUp];
                            std::cout<<"row_Leftup: "<<row_LeftUp<<std::endl;
                            if(row_LeftUp >= 180)
                            {
                                flag = true;
                                for(int i = 0;i<10;i++)
                                {
                                    std::cout<<"=============flag = true=============="<<std::endl;
                                }
                            }
                            if(flag)
                            {
                                if (row_LeftUp <= params.Left_turning2 && row_LeftUp != 0 && row_LeftUp > 100) // 二号车库的补线时机（二号库入库时机）row_RightUp < 180 && !row_RightUp
                                {
                                    flag_pairline = true;
                                    pointU = POINT(0,0);
                                    flag = false; // 重置标志位
                                }
                            }
                        }
                        if(flag_pairline)
                        {
                            for(int i = 0;i<10;i++)
                            {
                                std::cout<<"=============pair line=============="<<std::endl;
                            }
                            //左二车库补线
                            vector<POINT> smooth;
                            POINT p1 =POINT (110,320);
                            POINT p2 =POINT (65,160);
                            POINT p3 =POINT (45,0);
                            bezier3P(p1,p2 , p3, smooth);
                            track.pointsEdgeRight.clear();
                            track.pointsEdgeRight.insert(track.pointsEdgeRight.end(), smooth.begin(), smooth.end());
    
                            vector<POINT> smooth2;
                            POINT p1_1 =POINT (240,10);
                            POINT p2_2 =POINT (120,10);
                            POINT p3_3 =POINT (0,10);
                            bezier3P(p1_1 ,p2_2 , p3_3, smooth2);
                            track.pointsEdgeLeft.clear();
                            track.pointsEdgeLeft.insert(track.pointsEdgeLeft.end(), smooth2.begin(), smooth2.end());
                            
    
                            static int count_leftgarget_second = 0; // static 保持跨帧记忆
                            count_leftgarget_second++;               // 每帧 +1
                            if(count_leftgarget_second > params.Left_turningtime2)    //入二号库时间持续27帧 = Right_turningtime2
                            {
                                count_leftgarget_second = 0;
                                step = ParkStep::stop; //切换下一个状态
                                flag_pairline = false; // 重置标志位
                                break;
                            }                            
                        }
                    } 
                }
               break;
            }

            case ParkStep::stop:
            {
                static int count_stop = 0; // static 保持跨帧记忆
                        count_stop++;               // 每帧 +1
                if (count_stop > params.stopTime)//   stopTime = 停30帧
                {
                    step = ParkStep::trackout;
                    count_stop = 0; // 重置计数器
                }
                break;
            }

            case ParkStep::trackout:  //出库阶段
            {
                if(RightEnable == false)
                {
                    //补原出库线1
                    vector<POINT> smooth;
                    POINT p1 =POINT (110,320);
                    POINT p2 =POINT (65,160);
                    POINT p3 =POINT (45,0);
                    bezier3P(p1,p2 , p3, smooth);
                    track.pointsEdgeRight.clear();
                    track.pointsEdgeRight.insert(track.pointsEdgeRight.end(), smooth.begin(), smooth.end());

                    //补原出库线2
                    vector<POINT> smooth2;
                    POINT p1_1 =POINT (0,10);
                    POINT p2_2 =POINT (120,10);
                    POINT p3_3 =POINT (240,10);
                    bezier3P(p1_1,p2_2 , p3_3, smooth2);
                    track.pointsEdgeLeft.clear();
                    track.pointsEdgeLeft.insert(track.pointsEdgeLeft.end(), smooth2.begin(), smooth2.end());

                    static int count_out = 0; // static 保持跨帧记忆
                    count_out++;               // 每帧 +1
                    if(count_out > params.trackoutTime) // trackoutTime出库时间 = 18帧
                    {
                        count_out = 0;
                        step = ParkStep::exit_protect;// 出库完成 
                        break;
                    }
                }
                else
                {
                    //补原出库线1
                    vector<POINT> smooth;
                    POINT p1 =POINT (110,0);
                    POINT p2 =POINT (65,160);
                    POINT p3 =POINT (45,320);
                    bezier3P(p1,p2 , p3, smooth);
                    track.pointsEdgeLeft.clear();
                    track.pointsEdgeLeft.insert(track.pointsEdgeLeft.end(), smooth.begin(), smooth.end());

                    //补原出库线2
                    vector<POINT> smooth2;
                    POINT p1_1 =POINT (10,310);
                    POINT p2_2 =POINT (110,310);
                    POINT p3_3 =POINT (220,310);
                    bezier3P(p1_1,p2_2 , p3_3, smooth2);
                    track.pointsEdgeRight.clear();
                    track.pointsEdgeRight.insert(track.pointsEdgeRight.end(), smooth2.begin(), smooth2.end());   

                    static int count_out = 0; // static 保持跨帧记忆
                    count_out++;               // 每帧 +1
                    if(count_out > params.trackoutTime) // trackoutTime出库时间 = 18帧
                    {
                        count_out = 0;
                        step = ParkStep::exit_protect;// 出库完成 
                         break;
                    }                    
                }  
                break;
            }

            case  ParkStep::exit_protect:   // 出库完成
            {
                static int count_exit = 0; // static 保持跨帧记忆
                count_exit++;               // 每帧 +1
                if (count_exit > 35) // 出库完成后等待10帧
                {
                    count_exit = 0;
                    resetState();
                }
                break;
            }
            default:
                break;
        }

        return step != ParkStep::none;
    }

    /**
     * @brief 绘制结果
     */
    void drawImage(Tracking track, Mat &image)
    {
        for (const auto &pt : track.pointsEdgeLeft)
            circle(image, Point(pt.y, pt.x), 1, Scalar(0, 255, 0), -1);// 绿色点
        for (const auto &pt : track.pointsEdgeRight)
            circle(image, Point(pt.y, pt.x), 1, Scalar(0, 255, 255), -1);// 黄色点

            circle(image, Point(pointU.y, pointU.x), 5, Scalar(255, 0, 255), -1); // **上补线点：粉色**
            circle(image, Point(pointD.y, pointD.x), 5, Scalar(226, 43, 138), -1); // **下补线点：紫色**

        if (step != ParkStep::none)
            putText(image, "[1] BATTERY - ENABLE", Point(COLSIMAGE / 2 - 30, 10),
                    cv::FONT_HERSHEY_TRIPLEX, 0.3, Scalar(0, 255, 0), 1, CV_AA);
    }



private:
    uint16_t counterSession = 0;    // 图像场次计数器
    bool garageFirst = true;        // 进入一号车库
    bool RightEnable = false;
    uint16_t counterRec = 0;        // 加油站标志检测计数器
    bool flag = false;
    bool flag_pairline = false; // 左一车库补线标志位

    // 重置状态
    void resetState()
    {
        counterSession = 0;
        counterRec = 0;
        step = ParkStep::none;
        garageFirst = true;
        RightEnable = false;
    }
};








