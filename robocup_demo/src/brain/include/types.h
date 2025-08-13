/**
 * @file type.h
 * @brief 定义 brain 项目中用到的 struct 及 enum
 */
#pragma once

#include <string>
#include <vector>
#include <numeric>
#include <iterator>
#include <limits>
#include <rclcpp/rclcpp.hpp>

using namespace std;

/* ------------------ Struct ------------------------*/

// 球场尺寸信息, 所有平行于底线的是长度, 所有垂直于底线的是宽度, 与线的长度无关
struct FieldDimensions
{
    double length;            // 球场长度
    double width;             // 球场宽度
    double penaltyDist;       // 罚球点距离底线的直线距离
    double goalWidth;         // 球门的宽度
    double circleRadius;      // 中圈的半径
    double penaltyAreaLength; // 禁区的长
    double penaltyAreaWidth;  // 禁区的宽
    double goalAreaLength;    // 球门区的长
    double goalAreaWidth;     // 球门区的宽
                              // 注意: 禁区比球门区大；禁区和球门区的长与宽实际上要小。这个命名是为了与比赛规则相统一。
};
const FieldDimensions FD_KIDSIZE{9, 6, 1.5, 2.6, 0.75, 2, 5, 1, 3};
const FieldDimensions FD_ADULTSIZE{14, 9, 2.1, 2.6, 1.5, 3, 6, 1, 4};
const FieldDimensions FD_ROBOLEAGUE{22, 14, 3.6, 2.6, 2, 5, 8, 2, 5};

// Pose2D, 记录平面上的一个点以及朝向
struct Pose2D
{
    double x = 0;
    double y = 0;
    double theta = 0; // rad, 从 x 轴正方向开始, 逆时针为正
};

// 定位范围约束条件
struct PoseBox2D
{
	double xmin;
	double xmax;
	double ymin;
	double ymax;
	double thetamin;
	double thetamax;
};

// 球场标志点
struct FieldMarker
{
	char type;		   // L|T|X|P, 代表不同的标志点类型，其中 P 代表 penalty mark
	double x, y;	   // 标志点的位置 (m)
	double confidence; // 识别的信心度
};

// 定位结果
struct LocateResult
{
	bool success = false;
	int code = -1;		 // int, 0: 成功； 1: 生成新粒子失败(数量为 0); 2: 收敛后的残差不合理; 3: 未收敛; 4: Marker 数量不够; 5: 所有粒子的概率都过低; -1 初始态
	double residual = 0; // 平均残差
	Pose2D pose;
	int msecs = 0; // 定位耗时
};

// Point, 记录一个三维点
struct Point
{
    double x;
    double y;
    double z;
};

// Point2D, 记录一个二维点
struct Point2D
{
    double x;
    double y;
};

// BoundingBox
struct BoundingBox
{
    double xmin, xmax, ymin, ymax;
};

// GameObject, 用于存储比赛中的重要实体信息，如 Ball, Goalpost 等。相比于 /detect 消息中的 detection::DetectedObject，它的信息更为丰富。
struct GameObject
{
    string label;                
    string color;                
    BoundingBox boundingBox;     
    Point2D precisePixelPoint;   
    double confidence;           
    Point posToRobot;            

    Point posToField;                                 
    double range;                                    
    double pitchToRobot, yawToRobot;                  
    rclcpp::Time timePoint;                           

    int id;    
    string name;  
    double idConfidence; 
    string info; 
};

// Line
struct Line {
    double x0, y0, x1, y1;
};

enum class LineHalf {
    NA,
    Self,
    Opponent
};

enum class LineDir {
    NA,
    Horizontal,
    Vertical
};

enum class LineSide {
    NA,
    Left,
    Right
};

enum class LineType {
    NA,
    TouchLine,
    MiddleLine,
    GoalLine,
    GoalArea,
    PenaltyArea
};

struct FieldLine {
    Line posToField;
    Line posToRobot;
    Line posOnCam;
    rclcpp::Time timePoint;                           
    LineDir dir;                                       
    LineHalf half;                                      
    LineSide side;                                      
    LineType type;
    double confidence;                                      
};

struct MapMarking {
    double x;
    double y;
    string type; // TCross | LCross | XCross | PenaltyPoint
    string name; 
};

// VisionBox 用于视野的范围
struct VisionBox {
    vector<double> posToField;
    vector<double> posToRobot;
    rclcpp::Time timePoint;                           // 物体被检测到的时间
};

struct TimestampedData {
    vector<double> data;
    rclcpp::Time timePoint;
};

// 起身
struct RobotRecoveryStateData {
    uint8_t state; 
    uint8_t is_recovery_available; 
    uint8_t current_planner_index;
};

// 用于存储队友间通讯
struct TMStatus {
    string role = "not initialized"; // triker, goal_keeper
    bool isAlive = false; 
    bool ballDetected = false;
    bool ballLocationKnown = false;
    double ballConfidence = 0.;
    double ballRange = 0.;
    double cost = 0.; 
    bool isLead = true; 
    Point ballPosToField;
    Pose2D robotPoseToField;
    double kickDir = 0.; 
    double thetaRb = 0.; 
    int cmd = 0; 
    int cmdId = 0;
    rclcpp::Time timeLastCom; 
};


enum class RobotRecoveryState {
    IS_READY = 0,
    IS_FALLING = 1,
    HAS_FALLEN = 2,
    IS_GETTING_UP = 3
};
