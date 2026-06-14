/**

 * @file motion.cpp
 * @author Du
 * @brief 运动控制器：PD姿态控制||速度控制
 * @version 0.1
 * @date 2023-12-26
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <fstream>
#include <iostream>
#include <cmath>
#include "../include/common.hpp"
#include "../include/json.hpp"
#include "../include/fruzzy_pid.hpp"
#include "fruzzy_pid.cpp"
#include "controlcenter.cpp"

using namespace std;
using namespace cv;

//模糊控制中的语言变量，用于描述误差或误差微分的等级
#define NB -3 //负大
#define NM -2 //负中
#define NS -1 //负小
#define ZO 0 //零
#define PS 1 //正小
#define PM 2 //正中
#define PB 3 //正大

//kp_m为动态的Kp最大值，kd_m为动态的最大Kd值

float Fuzzy_Kp(int e,int ec);//模糊pid控制器：计算比例系数kp
float Fuzzy_Kd(int e,int ec);//模糊pid控制器：计算微分系数kd
void lishudu(int e,int ec);

extern float ringstep; // 环岛状态变量
extern float parkingstep; // 停车场状态变量
extern float stdevLeft; // 左边缘标准差
extern float stdevRight; // 右边缘标准差

const float M_PI_f = 3.14159265358979323846f;
/**
 * @brief 运动控制器
 *
 */
class Motion
{
private:
    int countShift = 0; // 变速计数器

public:
    
    int Index[6]={1,4,8,12,16,18};//误差补偿索引数组
    //输出量U语言值特征点


public:
    /**
     * @brief 初始化：加载配置文件
     *
     */
    Motion()
    {
        string jsonPath = "../src/config/config.json";
        std::ifstream config_is(jsonPath);
        if (!config_is.good())
        {
            std::cout << "Error: Params file path:[" << jsonPath
                      << "] not find .\n";
            exit(-1);
        }

        nlohmann::json js_value;
        config_is >> js_value;

        try
        {
            params = js_value.get<Params>();
        }
        catch (const nlohmann::detail::exception &e)
        {
            std::cerr << "Json Params Parse failed :" << e.what() << '\n';
            exit(-1);
        }

        speed = params.speedLow;
        cout << "--- speedLow:" << params.speedLow << "m/s  |  speedHigh:" << params.speedHigh << "m/s" << endl;
    };

    /**
     * @brief 控制器核心参数
     *
     */
    struct Params
    {
        float speedLow = 0.8;                              // 智能车最低速
        float speedHigh = 0.8;                             // 智能车最高速
        float speedBridge = 0.6;                           // 坡道速度
        float speedCatering = 0.6;                         // 快餐店速度
        float speedLayby = 0.6;                            // 临时停车区速度
        float speedObstacle = 0.6;                         // 障碍区速度
        float speedParking = 0.6;                          // 停车场速度       

        float speedDown = 0.5;                             // 特殊区域降速速度
        float speedRing = 0.3;     

        float runP1 = 0.9;                                 // 直道比例系数
        float runP2 = 0.018;                               // 十字比例系数
        float runP3 = 0.0;                                 // 左环岛比例系数
        float turnP = 3.5;    //未调用
        float turnD1 = 0;                                // 直道微分系数
        float turnD2 = 3.5;                                // 十字微分系数
        float turnD3 = 3.5;                                // 左环岛微分系数
        float obstacleP = 0.5;                            // 障碍区比例系数
        float obstacleD = 0.5;                            // 障碍区微分系数
        float parkingP = 0.5;                             //停车区KP
        float parkingD = 0.5;                             //停车区KD
        float sigma_mid = 60;//中间sigma值

        float KP0 = 1.9;//模糊pid:kp参数
        float KP1 = 3.8;
        float KP2 = 4.5;
        float KP3 = 5.0;
        float KP4 = 5.6;
        float KP5 = 6.7;
        float KP6 = 7.0;
        
        float KD0 = 0;//糊pid:kd参数
        float KD1 = 0.25;
        float KD2 = 0.5;
        float KD3 = 0.75;
        float KD4 = 1.0;
        float KD5 = 1.25;
        float KD6 = 1.5;

        float kp1 = 5.3;//双kp kd
        float kp2 = 2;
        float kd1 = 0.7;
        float kd2 = -1.5;


        uint16_t angle = 35;        
        bool debug = false;                                // 调试模式使能
        bool saveImg = false;                              // 存图使能
        uint16_t rowCutUp = 10;                            // 图像顶部切行
        uint16_t rowCutBottom = 10;                        // 图像顶部切行

        bool bridge = true;                                // 坡道区使能
        bool catering = true;       // 快餐店使能
        bool layby = true;          // 临时停车区使能
        bool obstacle = true;       // 障碍区使能
        bool stop = true;           // 停车区使能
        bool parking = true;        // 停车场使能
        bool ring = true;                                  // 环岛使能
        bool cross = true;                                 // 十字道路使能

        float score = 0.5;                                 // AI检测置信度
        float e_max = 160;//模糊pid：误差最大值
        float e_min = -160;
        float de_max = 160;//模糊pid：最大误差微分
        float de_min = -160;

        string model = "../res/model/yolov3_mobilenet_v1"; // 模型路径
        string video = "../res/samples/demo.mp4";          // 视频路径
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Params, speedLow, speedHigh, speedBridge, speedDown,speedCatering, speedLayby, speedObstacle,speedParking,
                                         speedRing,
                                        runP1, runP2, runP3,
                                       turnP, turnD1,turnD2,turnD3,sigma_mid,angle, debug, saveImg, rowCutUp, rowCutBottom, bridge, catering, layby, obstacle,
                                       parking, stop, ring, cross, score, model, video,e_max,
                                       de_max,
                                       KP0,KP1,KP2,KP3,KP4,KP5,KP6,KD0,KD1,KD2,KD3,KD4,KD5,KD6,kd2,obstacleP,obstacleD,kp1,kp2,kd1,parkingP,parkingD); // 添加构造函数，可直接在json文件里修改
    };

    Params params;                   // 读取控制参数
    Scene sce;
    uint16_t servoPwm = PWMSERVOMID; // 发送给舵机的PWM
    float speed = 0.3;               // 发送给电机的速度
    int i = 0;//临时变量
    float lastactual = 0;//记录上一次的实际值

    int Abs(int data)//求绝对值
    {
        if(data <= 0)
            data = -data;
        return data;
    }

    int xianfu(int pwm)
    {
        if(pwm >= 1820)
            pwm = 1820;
        if(pwm <= 1180)
            pwm = 1180;
        return pwm;
    }
    
    /**
     * @brief 姿态PD控制器
     *
     * @param controlCenter 智能车控制中心
     */
    // 舵机PWM最大值（左）1840 舵机PWM中值 1500 舵机PWM最小值（右）1160
    void poseCtrl(int controlCenter,ControlCenter control,Tracking &track ,Scene scene = Scene::NormalScene)
    {
        static unsigned int index = 0;
        static int count= 0;
        float E=0,EC=0;//误差及误差微分
        static float errorLast = 0;          // 记录前一次的偏差
        float sigmaValue = abs(control.sigmaCenter);//获取sigma值    (sigma值为拟合的中线的方差)                               
        float Kp,Kd,output;//pid参数及输出
        E  = controlCenter - 161; // 控制中心与图像中心的偏差，目的为把车s道中心控制在计算出的控制中心
        EC = E - errorLast;//误差的微分，即误差的变化率

        std::cout << "control center: " << controlCenter << std::endl;
        std::cout << "err: " << E << std::endl;
        std::cout << "derr: " << EC << std::endl;

        if(E > params.e_max)
        E = params.e_max;
        if(E < params.e_min)
        E = params.e_min;
        if(EC > params.de_max)
        EC = params.de_max;
        if(EC < params.de_min)
        EC = params.de_min;

        /*if(scene == Scene::RingScene && ringstep == 0)//入环前减小kp kd
        {
            std::cout<<"PID of before_Ring"<<std::endl;
            output = (E * 1.5) + (EC) * 0.5;
            int pwmDiff = (int)(output);
            errorLast = E;
            servoPwm = xianfu((uint16_t)(PWMSERVOMID + pwmDiff)); // PWM转换
            return;
        }
        */
        if(scene == Scene::RingScene && (ringstep == 2 || ringstep == 3 || ringstep == 4) )//环中大kp
        {
            std::cout<<"PID of Ring"<<std::endl;
            output = (E * params.runP3) + (EC) * params.turnD3;
            int pwmDiff = (int)(output);
            errorLast = E;
            servoPwm = xianfu((uint16_t)(PWMSERVOMID + pwmDiff)); // PWM转换
            return;
        }
        else if(scene == Scene::ParkingScene && (parkingstep == 5 || parkingstep == 6 || parkingstep == 7 || parkingstep == 4) )//停车区pid
        {
            std::cout<<"PID of Parking"<<std::endl;
            output = (E * params.parkingP) + (EC) * params.parkingD;
            int pwmDiff = (int)(output);
            errorLast = E;
            servoPwm = xianfu((uint16_t)(PWMSERVOMID + pwmDiff)); // PWM转换
            return;
        }
        else if(scene == Scene::ParkingScene && (parkingstep == 1 ))//停车区前小kp
        {
            std::cout<<"PID of before_Parking"<<std::endl;
            output = (E * 1.2) + (EC) * 0;
            int pwmDiff = (int)(output);
            errorLast = E;
            servoPwm = xianfu((uint16_t)(PWMSERVOMID + pwmDiff)); // PWM转换
            return;
        }
        else if(scene == Scene::ObstacleScene)//障碍区pid
        {
            std::cout<<"PID of Obstacle"<<std::endl;
            output = (E * params.obstacleP) + (EC) * params.obstacleD;
            int pwmDiff = (int)(output);
            errorLast = E;
            servoPwm = xianfu((uint16_t)(PWMSERVOMID + pwmDiff)); // PWM转换
            return;
        }//&& (stdevLeft <= params.angle && stdevRight <= params.angle) abs(E) > 5 && control.stright
        else if(sigmaValue <= params.sigma_mid  && scene == Scene::NormalScene)//直道pid
        {
            output = (E * params.runP1) + (EC) * params.turnD1;
            cout << "直道!!!" << endl;
            int pwmDiff = (int)(output);
            errorLast = E;
            servoPwm = (uint16_t)(PWMSERVOMID + pwmDiff); // PWM转换
            return;
        }
       else
       {
        
        lishudu(E,EC);
        Kp = Fuzzy_Kp(E,EC);
        Kd = Fuzzy_Kd(E,EC);
        std::cout << "kp: " << Kp << std::endl;
        std::cout << "kd: " << Kd << std::endl;
        output = Kp*E+Kd*(EC);
        int pwmDiff = (int)(output);
        errorLast = E;
        servoPwm = xianfu((uint16_t)(PWMSERVOMID + pwmDiff)); // PWM转换
        
       /*
        output = params.kp1*E+(params.kp2/100)*E*Abs(E)+params.kd1*(EC);
        int pwmDiff = (int)(output);
        errorLast = E;
        servoPwm = (uint16_t)(PWMSERVOMID + pwmDiff); // PWM转换
        */
        }
    }
    /**
     * @brief 变加速控制
     *
     * @param enable 加速使能
     * @param control
     */
    void speedCtrl(bool enable, bool slowDown, ControlCenter control,bool ring=false) {
    // 定义速度控制参数
    uint8_t controlLow = 0;   // 速度控制下限
    uint8_t controlMid = 5;   // 中间速度控制点
    uint8_t controlHigh = 10; // 速度控制上限

    // 定义速度参数
    float speedLow = params.speedLow;  // 最低速度
    float speedHigh = params.speedHigh; // 最高速度

    // 初始化速度和控制位移

    uint8_t countShift = controlLow;
    if(ring){
        speed = params.speedRing;
        return;
    }
    if (slowDown) {
        // 如果需要减速，直接设置为最低速度
        speed = speedLow;
    } else if (enable) {
        // 如果中心线点集过少保持最低俗
        if (control.centerEdge.size() < 10 || 
            control.centerEdge[control.centerEdge.size() - 1].x > ROWSIMAGE / 2) {
            // 特定条件下，保持低速
            countShift = controlLow;
            std::cout << "this is the special scene" << std::endl;
        } else {
            // 根据sigma值调整速度
            float sigmaValue = abs(control.sigmaCenter);
            if (sigmaValue <= 60.0f) {
                // 当sigmaValue在0到90之间时，进行线性插值
                countShift = static_cast<uint8_t>(controlLow + 
                    (controlHigh - controlLow) * (1.0f - (sigmaValue / 60.0f)));
            } else {
                // 当sigmaValue超过50时，按最低速度跑
                countShift = controlLow;
            }

            // 确保countShift在控制范围内
            countShift = std::min(controlHigh, std::max(controlLow, countShift));
        }

        // 根据countShift计算速度
        speed = speedLow + (speedHigh - speedLow) * (countShift / static_cast<float>(controlHigh));
    } else {
        // 如果未使能，保持低速
        speed = speedLow;
        countShift = controlLow;
        std::cout<<"unable"<<std::endl;
    }
    std::cout<<"the speed is "<<speed<<std::endl;
        // 此处可以添加额外的代码，例如更新速度控制变量或通知其他系统组件
        // 例如：updateSpeed(speed); // 假设的函数，用于更新速度
    }

private:

        //UFF_P、UFF_D数组：分别定义了输出量U语言值关于Kp和Kd的特征点，这些特征点用于后续确定隶属度函数以及反模糊化过程中的计算。例如，float UFF_P[7]={1.05,1.15, 1.35, 1.55, 1.75, 2.05, 3.05}; 定义了Kp相关的语言值特征点。
        //float UFF_P[7]={params.KP0,params.KP1,params.KP2,params.KP3,params.KP4,params.KP5,params.KP6};//140       //输出增量kp的隶属值
        //float UFF_D[7]={params.KD0,params.KD1,params.KD2,params.KD3,params.KD4,params.KD5,params.KD6};//{0,1,2,3,4,5,6};//KD_max均分七分//140       //输出增量kd的隶属值
        //rule_p、rule_d二维数组：这两个二维数组分别表示Kp和Kd的模糊控制规则表。规则表定义了在不同的误差和误差变化率组合下，对应的输出隶属度标号。例如，rule_p[7][7]中的元素值确定了Kp在各种误差和误差变化率情况下的规则映射。
        //p规则表
        unsigned int rule_p[7][7]={
        //误差变化率 -3,-2,-1, 0, 1, 2, 3     // 误差      误差决定行，误差变化量决定列
                    {6, 5, 4, 3, 2, 0, 0,},   //   -3
                    {5, 4, 3, 2, 1, 0, 1,},   //   -2
                    {4, 3, 2, 1, 0, 1, 2,},   //   -1
                    {3, 2, 1, 0, 1, 2, 3,},   //    0
                    {2, 1, 0, 1, 2, 3, 4,},   //    1
                    {1, 0, 1, 2, 3, 4, 5,},   //    2
                    {0, 0, 2, 3, 4, 5, 6}};   //    3
        /*
        unsigned int rule_p[7][7]={
        //误差变化率 -3,-2,-1, 0, 1, 2, 3     // 误差      误差决定行，误差变化量决定列
                    {6, 5, 4, 3, 2, 0, 0,},   //   -3
                    {5, 4, 3, 2, 1, 0, 1,},   //   -2
                    {4, 3, 2, 1, 0, 1, 2,},   //   -1
                    {3, 2, 1, 0, 1, 2, 3,},   //    0
                    {2, 1, 0, 1, 2, 3, 4,},   //    1
                    {1, 0, 1, 2, 3, 4, 5,},   //    2
                    {0, 0, 2, 3, 4, 5, 6}};   //    3
*/

        //d规则表
        
        unsigned int rule_d[7][7]={
        //误差变化率 -3,-2,-1, 0, 1, 2, 3    // 误差
                    {2, 2, 6, 5, 6, 4, 2,},   //   -3
                    {1, 2, 5, 4, 3, 1, 0,},   //   -2
                    {0, 1, 3, 3, 1, 1, 0,},   //   -1
                    {0, 1, 1, 1, 1, 1, 0,},   //    0
                    {0, 0, 0, 0, 0, 0, 0,},   //    1
                    {5, 1, 1, 1, 1, 1, 1,},   //    2
                    {6, 4, 4, 3, 3, 1, 1}};   //    3
        
        
        
        //PFF、DFF数组：用于界定误差（E）和误差变化率（EC）的变化范围点，在模糊化过程中，根据输入值与这些范围点的比较来确定相应的隶属度情况。像 int PFF[4]={0,40,80,120}; 界定了误差的范围。
        //变化范围点
        int PFF[4]={0,30,60,110};//E    //输入e的隶属值
        int DFF[4]={0,10,18,35};//EC    //输入de/dt的隶属值
        
        
        //像 PF[2]、DF[2] 分别用于存储偏差和偏差微分的隶属度，Pn、Dn 用于记录对应的角标，FMAX 定义了语言值的满幅值等。
        int   PF[2],DF[2];   //偏差,偏差微分隶属度
        int   Pn,Dn;   //角标
        const int FMAX = 100;    //语言值的满幅值

    //lishudu函数（模糊化）：该函数的作用是根据输入的误差 e 和误差变化率 ec 来确定它们各自对应的隶属度。通过将输入值与 PFF、DFF 数组中定义的范围进行比较，计算出相应的隶属度值，并确定对应的隶属度标号（如 Pn、Dn）。例如，对于误差 e，会按照不同的范围条件（如 e<=-PFF[2] 等）来计算其隶属度 PF[0] 以及确定所属的语言值标号 Pn。
//模糊化
void lishudu(int e,int ec)
{
   //根据E Ec的指定语言值获得有效隶属度
   //寻找e的隶属度
   if(e>-PFF[3] && e<PFF[3])//E的变化在幅值内                                      //Pn:  -2 -1 0 1 2 3
   {
        if(e<=-PFF[2])      //误差小于-80大于-120
        {
            Pn=-2;
            PF[0]=FMAX*((float)(-PFF[2]-e)/(PFF[3]-PFF[2]));
            //此时设定隶属度标号 Pn = -2，并通过线性插值的方式计算隶属度 PF[0]。计算公式为 PF[0] = FMAX * ((float)(-PFF[2] - e) / (PFF[3] - PFF[2]))，这里利用了当前误差与对应范围边界的差值，结合范围区间长度进行比例计算，以得出在该区间内的隶属度值，其中 FMAX 作为满幅值参与运算，确保隶属度值在合理的范围（0 到 FMAX）内。
        }
        else if(e<=-PFF[1]) //误差小于-40大于-80
        {
            Pn=-1;
            PF[0]=FMAX*((float)(-PFF[1]-e)/(PFF[2]-PFF[1]));
        }
        else if(e<=PFF[0])  //误差小于0大于-40
        {
            Pn=0;
            PF[0]=FMAX*((float)(-PFF[0]-e)/(PFF[1]-PFF[0]));
        }
        else if(e<=PFF[1])  //误差小于40大于0
        {
            Pn=1;
            PF[0]=FMAX*((float)(PFF[1]-e)/(PFF[1]-PFF[0]));
        }
        else if(e<=PFF[2])  //误差小于80大于40
        {
            Pn=2;
            PF[0]=FMAX*((float)(PFF[2]-e)/(PFF[2]-PFF[1]));
        }
        else if(e<=PFF[3])  //误差小于120大于80
        {
            Pn=3;
            PF[0]=FMAX*((float)(PFF[3]-e)/(PFF[3]-PFF[2]));
        }
   }
   else if(e<=-PFF[3])  //限幅    误差小于-120
   {
        Pn=-2;
        PF[0]=FMAX;
   }
   else if(e>=PFF[3])           //误差大于120
   {
        Pn=3;
        PF[0]=0;
   }

   PF[1]=FMAX-PF[0];//在确定了 PF[0] 后，通过 PF[1] = FMAX - PF[0] 计算出其互补的隶属度值，这符合隶属度函数的基本特性，两者之和通常应为满幅值 FMAX，从另一个角度描述了该误差相对于当前语言值的隶属情况。





   if(ec>-DFF[3] && ec<DFF[3])  //误差变化率                        //Dn:  -2 -1 0 1 2 3
   {
        if(ec<=-DFF[2])         //误差变化率小于-40
        {
           Dn=-2;
           DF[0]=FMAX*((float)(-DFF[2]-ec)/(DFF[3]-DFF[2]));
        }
        else if(ec<=-DFF[1])    //误差变化率小于-20
        {
            Dn=-1;
            DF[0]=FMAX*((float)(-DFF[1]-ec)/(DFF[2]-DFF[1]));
        }
        else if(ec<=DFF[0]) ////误差变化率小于0
        {
           Dn=0;
           DF[0]=FMAX*((float)(-DFF[0]-ec)/(DFF[1]-DFF[0]));
        }
        else if(ec<=DFF[1]) //误差变化率小于20
        {
            Dn=1;
            DF[0]=FMAX*((float)(DFF[1]-ec)/(DFF[1]-DFF[0]));
        }
        else if(ec<=DFF[2]) ////误差变化率小于40
        {
            Dn=2;
            DF[0]=FMAX*((float)(DFF[2]-ec)/(DFF[2]-DFF[1]));
        }
        else if(ec<=DFF[3])     ////误差变化率小于60
        {
            Dn=3;
            DF[0]=FMAX*((float)(DFF[3]-ec)/(DFF[3]-DFF[2]));
        }
   }
      //限幅
   else if(ec<=-DFF[3])
   {
        Dn=-2;
        DF[0]=FMAX;
   }
   else if(ec>=DFF[3])
   {
        Dn=3;
        DF[0]=0;
   }
   DF[1]=FMAX-DF[0];
}



//Kp模糊引擎
float Fuzzy_Kp(int e,int ec)
{
  float out=0 ;  //输出值的精确量
  int Un[4];//对应附近P值
  float Un_out[4];
  unsigned int UF[4]; //隶属度
  int temp1,temp2;
  float UFF_P[7]={params.KP0,params.KP1,params.KP2,params.KP3,params.KP4,params.KP5,params.KP6};//140       //输出增量kp的隶属值

  //使用误差范围优化后的规则表rule[7][7]
  //一般都是四个规则有效
  Un[0]=rule_p[Pn+2][Dn+2];
  Un[1]=rule_p[Pn+3][Dn+2];
  Un[2]=rule_p[Pn+2][Dn+3];
  Un[3]=rule_p[Pn+3][Dn+3];

  if(PF[0]<=DF[0])
    UF[0]=PF[0];
  else
    UF[0]=DF[0];

  if(PF[1]<=DF[0])
      UF[1]=PF[1];
  else
      UF[1]=DF[0];

  if(PF[0]<=DF[1])
      UF[2]=PF[0];
  else
      UF[2]=DF[1];

  if(PF[1]<=DF[1])
      UF[3]=PF[1];
  else
      UF[3]=DF[1];


  //同隶属函数输出语言值求大
  if(Un[0]==Un[1])
  {
      if(UF[0]>UF[1])
          UF[1]=0;
      else
          UF[0]=0;
  }

  if(Un[0]==Un[2])
  {
      if(UF[0]>UF[2])
        UF[2]=0;
      else
        UF[0]=0;
  }

  if(Un[0]==Un[3])
  {
      if(UF[0]>UF[3])
        UF[3]=0;
      else
        UF[0]=0;
  }

  if(Un[1]==Un[2])
  {
      if(UF[1]>UF[2])
        UF[2]=0;
      else
        UF[1]=0;
  }

  if(Un[1]==Un[3])
  {
      if(UF[1]>UF[3])
        UF[3]=0;
      else
        UF[1]=0;
  }

  if(Un[2]==Un[3])
  {
      if(UF[2]>UF[3])
        UF[3]=0;
      else
        UF[2]=0;
  }

  //重心法反模糊
  //Un[]原值为输出隶属函数标号，转换为隶属函数值
  if(Un[0]>=0)
      Un_out[0]=UFF_P[Un[0]];
  else
      Un_out[0]=-UFF_P[-Un[0]];

  if(Un[1]>=0)
      Un_out[1]=UFF_P[Un[1]];
  else
      Un_out[1]=-UFF_P[-Un[1]];

  if(Un[2]>=0)
      Un_out[2]=UFF_P[Un[2]];
  else
      Un_out[2]=-UFF_P[-Un[2]];

  if(Un[3]>=0)
      Un_out[3]=UFF_P[Un[3]];
  else
      Un_out[3]=-UFF_P[-Un[3]];


  temp1=UF[0]*Un_out[0]+UF[1]*Un_out[1]+UF[2]*Un_out[2]+UF[3]*Un_out[3];
  temp2=UF[0]+UF[1]+UF[2]+UF[3];
  out=(float)temp1/temp2;
//temp1 的取值范围是在 0 到 4 * FMAX * Max_UFF_P 之间（其中 FMAX 是语言值满幅值常量，Max_UFF_P 是 UFF_P 数组元素的最大值）           4*100*3
  //temp2 的取值范围是在 0 到 4 * FMAX 之间                   0到400
  return out;
}

//Kd模糊引擎
float Fuzzy_Kd(int e,int ec)
{
  float out;  //输出值的精确量
  int Un[4]; //输出隶属度标记
  float Un_out[4];
  unsigned int   UF[4]; //隶属度
  int   temp1,temp2;
  float UFF_D[7]={params.KD0,params.KD1,params.KD2,params.KD3,params.KD4,params.KD5,params.KD6};//{0,1,2,3,4,5,6};//KD_max均分七分//140       //输出增量kd的隶属值

  //使用误差范围优化后的规则表rule[7][7]
  //一般都是四个规则有效
  Un[0]=rule_d[Pn+2][Dn+2];
  Un[1]=rule_d[Pn+3][Dn+2];
  Un[2]=rule_d[Pn+2][Dn+3];
  Un[3]=rule_d[Pn+3][Dn+3];

  if(PF[0]<=DF[0])
      UF[0]=PF[0];
  else
      UF[0]=DF[0];
  if(PF[1]<=DF[0])
      UF[1]=PF[1];
  else
      UF[1]=DF[0];
  if(PF[0]<=DF[1])
      UF[2]=PF[0];
  else
      UF[2]=DF[1];
  if(PF[1]<=DF[1])
      UF[3]=PF[1];
  else
      UF[3]=DF[1];

  //同隶属函数输出语言值求大

  if(Un[0]==Un[1])
  {
      if(UF[0]>UF[1])
          UF[1]=0;
      else
          UF[0]=0;
  }
  if(Un[0]==Un[2])
  {
      if(UF[0]>UF[2])
          UF[2]=0;
      else
          UF[0]=0;
  }
  if(Un[0]==Un[3])
  {
      if(UF[0]>UF[3])
          UF[3]=0;
      else
          UF[0]=0;
  }
  if(Un[1]==Un[2])
  {
      if(UF[1]>UF[2])

          UF[2]=0;
      else
          UF[1]=0;
  }
  if(Un[1]==Un[3])
  {
      if(UF[1]>UF[3])
          UF[3]=0;
      else
          UF[1]=0;
  }
  if(Un[2]==Un[3])
  {
      if(UF[2]>UF[3])
          UF[3]=0;
      else
          UF[2]=0;
  }

  //重心法反模糊
  //Un[]原值为输出隶属函数标号，转换为隶属函数值
  if(Un[0]>=0)
      Un_out[0]=UFF_D[Un[0]];
  else
      Un_out[0]=-UFF_D[-Un[0]];
  if(Un[1]>=0)
      Un_out[1]=UFF_D[Un[1]];
  else
      Un_out[1]=-UFF_D[-Un[1]];
  if(Un[2]>=0)
      Un_out[2]=UFF_D[Un[2]];
  else
      Un_out[2]=-UFF_D[-Un[2]];
  if(Un[3]>=0)
      Un_out[3]=UFF_D[Un[3]];
  else
      Un_out[3]=-UFF_D[-Un[3]];

  temp1=UF[0]*Un_out[0]+UF[1]*Un_out[1]+UF[2]*Un_out[2]+UF[3]*Un_out[3];
  temp2=UF[0]+UF[1]+UF[2]+UF[3];
  out=(float)temp1/temp2;

  return out;
}


/*
    //使用的模糊PID程序，有几个特殊步骤，输入的参数E为err，EC为err的微分，即这次的err减去上次的err
    float KP_Fuzzy(float E,float EC)
    {
    
        int rule_p[7][7]=
        {
            { 6 , 5 , 4 , 4 , 3 , 0 , 0},//-36
            { 6 , 4 , 3 , 3 , 2 , 0 , 0},//-24
            { 4 , 3 , 2 , 1 , 0 , 1 , 2},//-12
            { 2 , 1 , 1 , 0 , 1 , 1 , 2},//0
            { 2 , 1 , 0 , 1 , 2 , 3 , 4},//12
            { 0 , 0 , 2 , 3 , 3 , 4 , 6},//24
            { 0 , 1 , 3 , 4 , 4 , 5 , 6},//36
        };//模糊规则表 P
    
        unsigned char i2;
        //输入量P语言值特征点 表示输入误差E的语言级别
        float EFF[7]={-110,-60,-40,0,40,60,110};
        //输入量D语言值特征点 表示输入误差微分EC的语言级别
        float DFF[7]={-35,-18,-10,0,10,18,35};
        //输出量U语言值特征点(根据赛道类型选择不同的输出值)
        float UFF[7] = {params.KP0,params.KP1,params.KP2,params.KP3,params.KP4,params.KP5,params.KP6};
    
        // for(i2=0;i2<7;i2++)
        //     UFF[i2]=params.kp_m/6*i2;
    
    
        float U=0;  //偏差,偏差微分以及输出值的精确量
        float PF[2]={0},DF[2]={0},UF[4]={0};//PF数组为E的隶属度 DF数组为EC的隶属度 UF数组为kp的隶属度
        //偏差,偏差微分以及输出值的隶属度
        int Pn=0,Dn=0,Un[4]={0};//pn为误差E的语言级别 dn为误差微分EC的语言级别 Un数组存储经过误差及误差等级模糊推理后，从规则表中查找出来的kp规则
        float t1=0,t2=0,t3=0,t4=0,temp1=0,temp2=0;
        //隶属度的确定
        //对于给定的 E 或 EC，会根据它落在EFF数组和DFF数组的哪个区间内计算出相应的隶属度
        //计算误差E的隶属度
        if(E>EFF[0] && E<EFF[6]) 
        {
            if(E<=EFF[1])
            {
                Pn=-2;//确定误差所处的语言等级
                PF[0]=(EFF[1]-E)/(EFF[1]-EFF[0]);//计算隶属度
            }
            else if(E<=EFF[2])
            {
                Pn=-1;
                PF[0]=(EFF[2]-E)/(EFF[2]-EFF[1]);
            }
            else if(E<=EFF[3])
            {
                Pn=0;
                PF[0]=(EFF[3]-E)/(EFF[3]-EFF[2]);
            }
            else if(E<=EFF[4])
            {
                Pn=1;
                PF[0]=(EFF[4]-E)/(EFF[4]-EFF[3]);
            }
            else if(E<=EFF[5])
            {
                Pn=2;
                PF[0]=(EFF[5]-E)/(EFF[5]-EFF[4]);
            }
            else if(E<=EFF[6])
            {
                Pn=3;
                PF[0]=(EFF[6]-E)/(EFF[6]-EFF[5]);//判断条件重合E=EFF[6]
            }
        }
    
        else if(E<=EFF[0])
        {
            Pn=-2;
            PF[0]=1;
        }
        else if(E>=EFF[6])//判断条件重合E=EFF[6]
        {
            Pn=3;
            PF[0]=0;
        }
    
        PF[1]=1-PF[0];//表示该输入在前一个语言变量上的隶属度 例如本次输入的E＜EFF[0]则Pf[0]代表其在负中的隶属度，Pf[1]为负大的隶属度
    
    
        //计算误差微分EC的隶属度
        if(EC>DFF[0]&&EC<DFF[6])
        {
            if(EC<=DFF[1])
            {
                Dn=-2;
                DF[0]=(DFF[1]-EC)/(DFF[1]-DFF[0]);
            }
            else if(EC<=DFF[2])
            {
                Dn=-1;
                DF[0]=(DFF[2]-EC)/(DFF[2]-DFF[1]);
            }
            else if(EC<=DFF[3])
            {
                Dn=0;
                DF[0]=(DFF[3]-EC)/(DFF[3]-DFF[2]);
            }
            else if(EC<=DFF[4])
            {
                Dn=1;
                DF[0]=(DFF[4]-EC)/(DFF[4]-DFF[3]);
            }
            else if(EC<=DFF[5])
            {
                Dn=2;
                DF[0]=(DFF[5]-EC)/(DFF[5]-DFF[4]);
            }
            else if(EC<=DFF[6])
            {
                Dn=3;
                DF[0]=(DFF[6]-EC)/(DFF[6]-DFF[5]);
            }
        }
        //不在给定的区间内
        else if (EC<=DFF[0])
        {
            Dn=-2;
            DF[0]=1;
        }
        else if(EC>=DFF[6])
        {
            Dn=3;
            DF[0]=0;
        }
    
        DF[1]=1-DF[0];
    
        //使用误差范围优化后的规则表rule[7][7]
        //输出值使用13个隶属函数,中心值由UFF[7]指定
        //一般都是四个规则有效//
        Un[0]=rule_p[Pn+2][Dn+2];//根据误差和误差微分的语言级别在规则表中取出kp的四个规则
        Un[1]=rule_p[Pn+3][Dn+2];
        Un[2]=rule_p[Pn+2][Dn+3];
        Un[3]=rule_p[Pn+3][Dn+3];
    
        if(PF[0]<=DF[0])    //求小
            UF[0]=PF[0];
        else
            UF[0]=DF[0];
        if(PF[1]<=DF[0])
            UF[1]=PF[1];
        else
            UF[1]=DF[0];
        if(PF[0]<=DF[1])
            UF[2]=PF[0];
        else
            UF[2]=DF[1];
        if(PF[1]<=DF[1])
            UF[3]=PF[1];
        else
            UF[3]=DF[1];
        //同隶属函数输出语言值求大
        if(Un[0]==Un[1])
        {
            if(UF[0]>UF[1])
                UF[1]=0;
            else
                UF[0]=0;
        }
        if(Un[0]==Un[2])
        {
            if(UF[0]>UF[2])
                UF[2]=0;
            else
                UF[0]=0;
        }
        if(Un[0]==Un[3])
        {
            if(UF[0]>UF[3])
                UF[3]=0;
            else
                UF[0]=0;
        }
        if(Un[1]==Un[2])
        {
            if(UF[1]>UF[2])
                UF[2]=0;
            else
                UF[1]=0;
        }
        if(Un[1]==Un[3])
        {
            if(UF[1]>UF[3])
                UF[3]=0;
            else
                UF[1]=0;
        }
        if(Un[2]==Un[3])
        {
            if(UF[2]>UF[3])
                UF[3]=0;
            else
                UF[2]=0;
        }
        t1=UF[0]*UFF[Un[0]];
        t2=UF[1]*UFF[Un[1]];
        t3=UF[2]*UFF[Un[2]];
        t4=UF[3]*UFF[Un[3]];
        temp1=t1+t2+t3+t4;
        temp2=UF[0]+UF[1]+UF[2]+UF[3];//模糊量输出
        if(temp2!=0)
            U=temp1/temp2;
        else {
            U=0;
        }
    //    temp1=PF[0]*UFF[Un[0]]+PF[1]*UFF[Un[1]]+PF[0]*UFF[Un[2]]+PF[1]*UFF[Un[3]]+DF[0]*UFF[Un[0]]+DF[0]*UFF[Un[1]]+DF[1]*UFF[Un[2]]+DF[0]*UFF[Un[3]];
    //    U=temp1;
        return U;
    }
    int rule_d[7] = { 6 , 5 , 3 , 2 , 3 , 5 , 6};//模糊规则表 D
    float Kd_Fuzzy(float EC)
    {
        float out=0;
        unsigned char i=0;
        float degree_left = 0,degree_right = 0;
        unsigned char degree_left_index = 0,degree_right_index = 0;
        float DFF[7]={-35,-18,-10,0,10,18,35};

        float UFF[7] = {params.KD0,params.KD1,params.KD2,params.KD3,params.KD4,params.KD5,params.KD6};
    
        // for(i=0;i<7;i++)
        //         UFF[i]=params.kd_m/6*i;
    
        if(EC<DFF[0])
        {
            degree_left = 1;
            degree_right = 0;
            degree_left_index = 0;
        }
        else if (EC>DFF[6]) {
            degree_left = 1;
            degree_right = 0;
            degree_left_index = 6;
        }
        else {
            for(i=0;i<6;i++)
            {
                if(EC>=DFF[i]&&EC<DFF[i+1])
                {
                    degree_left = (float)(DFF[i+1] - EC)/(DFF[i+1] - DFF[i]);
                    degree_right = 1 - degree_left;
                    degree_left_index = i;
                    degree_right_index = i+1;
                    break;
                }
            }
        }
    
        out = UFF[rule_d[degree_left_index]]*degree_left+UFF[rule_d[degree_right_index]]*degree_right;

        return out;
    }
*/

  
};