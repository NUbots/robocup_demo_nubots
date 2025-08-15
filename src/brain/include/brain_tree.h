#pragma once

#include <tuple>
#include <behaviortree_cpp/behavior_tree.h>
#include <behaviortree_cpp/bt_factory.h>
#include <algorithm>

#include "types.h"

class Brain;

using namespace std;
using namespace BT;

class BrainTree
{
public:
    BrainTree(Brain *argBrain) : brain(argBrain) {}

    void init();

    void tick();

    // get entry on blackboard
    template <typename T>
    inline T getEntry(const string &key)
    {
        T value;
        [[maybe_unused]] auto res = tree.rootBlackboard()->get<T>(key, value);
        return value;
    }

    // set entry on blackboard
    template <typename T>
    inline void setEntry(const string &key, const T &value)
    {
        tree.rootBlackboard()->set<T>(key, value);
    }

private:
    Tree tree;
    Brain *brain;

    /**
     * 初始化 blackboard 里的 entry，注意新加字段，在这里设置个默认值
     */
    void initEntry();
};


class CalcKickDir : public SyncActionNode 
{
public:
    CalcKickDir(const string &name, const NodeConfig &config, Brain *_brain) : SyncActionNode(name, config), brain(_brain) {}

    static PortsList providedPorts()
    {
        return {
            InputPort<double>("cross_threshold", 0.2, "可进门的角度范围小于这个值时, 则传中")
        };
    }

    NodeStatus tick() override;

private:
    Brain *brain;
};


class StrikerDecide : public SyncActionNode
{
public:
    StrikerDecide(const string &name, const NodeConfig &config, Brain *_brain) : SyncActionNode(name, config), brain(_brain) {}

    static PortsList providedPorts()
    {
        return {
            InputPort<double>("chase_threshold", 1.0, "超过这个距离, 执行追球动作"),
            InputPort<string>("decision_in", "", "用于读取上一次的 decision"),
            InputPort<string>("position", "offense", "offense | defense, 决定了向哪个方向踢球"),
            OutputPort<string>("decision_out")};
    }

    NodeStatus tick() override;

private:
    Brain *brain;
    double lastDeltaDir; 
    rclcpp::Time timeLastTick; 
};


class GoalieDecide : public SyncActionNode
{
public:
    GoalieDecide(const std::string &name, const NodeConfig &config, Brain *_brain) : SyncActionNode(name, config), brain(_brain) {}

    static BT::PortsList providedPorts()
    {
        return {
            InputPort<double>("chase_threshold", 1.0, "超过这个距离, 执行追球动作"),
            InputPort<double>("adjust_angle_tolerance", 0.1, "小于这个角度, 认为 adjust 已经成功"),
            InputPort<double>("adjust_y_tolerance", 0.1, "y 方向偏移小于这个值, 认为 y 方向 adjust 成功"),
            InputPort<string>("decision_in", "", "用于读取上一次的 decision"),
            InputPort<double>("auto_visual_kick_enable_dist_min", 2.0, "自动视觉踢球启用时球的最小距离"),
            InputPort<double>("auto_visual_kick_enable_dist_max", 3.0, "自动视觉踢球启用时球的最大距离"),
            InputPort<double>("auto_visual_kick_enable_angle", 0.785, "自动视觉踢球启用时球的角度范围"),
            InputPort<double>("auto_visual_kick_obstacle_dist_threshold", 3.0, "自动视觉踢球障碍物距离阈值，如果该距离内有障碍物，则不执行自动视觉踢球"),
            InputPort<double>("auto_visual_kick_obstacle_angle_threshold", 1.744, "自动视觉踢球障碍物在前方角度范围内的阈值，如果该角度内有障碍物，则不执行自动视觉踢球"),
            OutputPort<string>("decision_out"),
        };
    }

    BT::NodeStatus tick() override;

private:
    Brain *brain;
};


class CamTrackBall : public SyncActionNode
{
public:
    CamTrackBall(const string &name, const NodeConfig &config, Brain *_brain) : SyncActionNode(name, config), brain(_brain) {}

    static PortsList providedPorts()
    {
        return {};
    }
    NodeStatus tick() override;

private:
    Brain *brain;
};


class CamFindBall : public SyncActionNode
{
public:
    CamFindBall(const string &name, const NodeConfig &config, Brain *_brain);

    NodeStatus tick() override;

private:
    double _cmdSequence[6][2];    
    rclcpp::Time _timeLastCmd;   
    int _cmdIndex;                
    long _cmdIntervalMSec;        
    long _cmdRestartIntervalMSec; 

    Brain *brain;

};


class RobotFindBall : public StatefulActionNode
{
public:
    RobotFindBall(const string &name, const NodeConfig &config, Brain *_brain) : StatefulActionNode(name, config), brain(_brain) {}

    static PortsList providedPorts()
    {
        return {
            InputPort<double>("vyaw_limit", 1.0, "转向的速度上限"),
        };
    }

    NodeStatus onStart() override;

    NodeStatus onRunning() override;

    void onHalted() override;

private:
    double _turnDir; 
    Brain *brain;
};


class CamFastScan : public StatefulActionNode
{
public:
    CamFastScan(const string &name, const NodeConfig &config, Brain *_brain) : StatefulActionNode(name, config), brain(_brain) {}

    static PortsList providedPorts()
    {
        return {
            InputPort<double>("msecs_interval", 300, "在同一个位置停留多少毫秒"),
        };
    }

    NodeStatus onStart() override;

    NodeStatus onRunning() override;

    void onHalted() override {};

private:
    double _cmdSequence[7][2] = {
        {0.45, 1.1},
        {0.45, 0.0},
        {0.45, -1.1},
        {1.0, -1.1},
        {1.0, 0.0},
        {1.0, 1.1},
        {0.45, 0.0},
    };    
    rclcpp::Time _timeLastCmd;    
    int _cmdIndex = 0;               
    Brain *brain;
};

class TurnOnSpot : public StatefulActionNode
{
public:
    TurnOnSpot(const string &name, const NodeConfig &config, Brain *_brain) : StatefulActionNode(name, config), brain(_brain) {}

    static PortsList providedPorts()
    {
        return {
            InputPort<double>("rad", 0, "转多少弧度, 向左为正"),
            InputPort<bool>("towards_ball", false, "为 true 时, 不考虑 rad 的正负号, 而是转向上一次看到不球的方向.")
        };
    }

    NodeStatus onStart() override;

    NodeStatus onRunning() override;

    void onHalted() override {};

private:
    double _lastAngle; 
    double _angle;
    double _cumAngle; 
    double _msecLimit = 5000;  
    rclcpp::Time _timeStart;
    Brain *brain;
};


class Chase : public SyncActionNode
{
public:
    Chase(const string &name, const NodeConfig &config, Brain *_brain) : SyncActionNode(name, config), brain(_brain) {}

    static PortsList providedPorts()
    {
        return {
            InputPort<double>("vx_limit", 0.6, "追球的最大 x 速度"),
            InputPort<double>("vy_limit", 0.4, "追球的最大 y 速度"),
            InputPort<double>("vtheta_limit", 1.0, "追球时, 实时调整方向的速度不大于这个值"),
            InputPort<double>("dist", 0.1, "追球的目标是球后面多少距离"),
            InputPort<double>("safe_dist", 4.0, "circle back 时, 保持的安全距离"),
            InputPort<double>("fixed_velocity_magnitude", 0.8, "Fixed velocity magnitude when walking towards the target"),
        };
    }

    NodeStatus tick() override;

private:
    Brain *brain;
    string _state;     
    double _dir = 1.0; 
};


class Adjust : public SyncActionNode
{
public:
    Adjust(const string &name, const NodeConfig &config, Brain *_brain) : SyncActionNode(name, config), brain(_brain) {}

    static PortsList providedPorts()
    {
        return {
            InputPort<double>("turn_threshold", 3.25, "球的角度大于这个值, 机器人先转身面向球, 直线运动先暂停"),
            InputPort<double>("vx_limit", 0.05, "调整过过程中 vx 的限制 [-limit, limit]"),
            InputPort<double>("vy_limit", 0.05, "调整过过程中 vy 的限制 [-limit, limit]"),
            InputPort<double>("vtheta_limit", 0.1, "调整过过程中 vtheta 的限制 [-limit, limit]"),
            InputPort<double>("range", 2.25, "ball  range 保持这个值"),
            InputPort<double>("vtheta_factor", 3.0, "调整角度时, vtheta 的乘数, 越大转向越快"),
            InputPort<double>("tangential_speed_far", 0.2, "调整角度时, 较远时的切线速度"),
            InputPort<double>("tangential_speed_near", 0.15, "调整角度时, 较近时的切线速度"),
            InputPort<double>("near_threshold", 0.8, "距离目标小于这个值时, 使用 near speed"),
            InputPort<double>("no_turn_threshold", 0.02, "角度差小于这个值时, 不转身"),
            InputPort<double>("turn_first_threshold", 0.8, "角度差大于这个值时, 先转身, 不移动"),

        };
    }

    NodeStatus tick() override;

private:
    Brain *brain;
};

// 执行踢球动作
class Kick : public StatefulActionNode
{
public:
    Kick(const string &name, const NodeConfig &config, Brain *_brain) : StatefulActionNode(name, config), brain(_brain) {}

    static PortsList providedPorts()
    {
        return {
            InputPort<int>("max_msec_kick", 3000, "The maximum duration (in milliseconds) for executing a kick action"),
            InputPort<double>("kick_range", 0.3, "Range threshold to reduce velocity for precision"),
            InputPort<double>("vx_limit", 1.2, "vx limit"),
            InputPort<double>("vy_limit", 0.4, "vy limit"),
            InputPort<double>("fixed_velocity_magnitude", 0.8, "Fixed velocity magnitude when walking towards the ball"),
            InputPort<double>("y_offset", -0.15, "y offset"),
        };
    }

    NodeStatus onStart() override;

    NodeStatus onRunning() override;

    // callback to execute if the action was aborted by another node
    void onHalted() override;

private:
    Brain *brain;
    rclcpp::Time _startTime; 
    double _initialBallRange = 0.0;
};


class StandStill : public StatefulActionNode
{
public:
    StandStill(const string &name, const NodeConfig &config, Brain *_brain) : StatefulActionNode(name, config), brain(_brain) {}

    static PortsList providedPorts()
    {
        return {
            InputPort<int>("msecs", 1000, "站立多少毫秒"),
        };
    }

    NodeStatus onStart() override;

    NodeStatus onRunning() override;

    // callback to execute if the action was aborted by another node
    void onHalted() override;

private:
    Brain *brain;
    rclcpp::Time _startTime; 
};


class CamScanField : public SyncActionNode
{
public:
    CamScanField(const std::string &name, const NodeConfig &config, Brain *_brain) : SyncActionNode(name, config), brain(_brain) {}

    static BT::PortsList providedPorts()
    {
        return {
            InputPort<double>("low_pitch", 0.6, "向下看时的最大 pitch"),
            InputPort<double>("high_pitch", 0.45, "向上看时的最小 pitch"),
            InputPort<double>("left_yaw", 0.8, "向左看时的最大 yaw"),
            InputPort<double>("right_yaw", -0.8, "向右看时的最小 yaw"),
            InputPort<int>("msec_cycle", 4000, "多少毫秒转一圈"),
        };
    }

    NodeStatus tick() override;

private:
    Brain *brain;
};


class MoveToPoseOnField : public SyncActionNode
{
public:
    MoveToPoseOnField(const std::string &name, const NodeConfig &config, Brain *_brain) : SyncActionNode(name, config), brain(_brain) {}

    static BT::PortsList providedPorts()
    {
        return {
            InputPort<double>("x", 0, "目标 x 坐标, Field 坐标系"),
            InputPort<double>("y", 0, "目标 y 坐标, Field 坐标系"),
            InputPort<double>("theta", 0, "目标最终朝向, Field 坐标系"),
            InputPort<double>("long_range_threshold", 1.5, "目标点的距离超过这个值时, 优先走过去, 而不是细调位置和方向"),
            InputPort<double>("turn_threshold", 0.4, "长距离时, 目标点的方向超这个数值时, 先转向目标点"),
            InputPort<double>("vx_limit", 0.8, "x 限速"),
            InputPort<double>("vy_limit", 0.5, "y 限速"),
            InputPort<double>("vtheta_limit", 0.2, "theta 限速"),
            InputPort<double>("x_tolerance", 0.5, "x 容差"),
            InputPort<double>("y_tolerance", 0.5, "y 容差"),
            InputPort<double>("theta_tolerance", 0.5, "theta 容差"),
            InputPort<bool>("avoid_obstacle", false, "是否避障")
        };
    }

    BT::NodeStatus tick() override;

private:
    Brain *brain;
};

/**
 * @brief Moves robot to hard-coded ready position based on player ID and role
 * 
 * Ready positions are now hard-coded for each robot based on their player_id:
 * 
 * Strikers:
 * - Player 1: Center striker (-circleRadius-1.0, 0.0)
 * - Player 2: Right striker (-circleRadius-1.0, 2.0) 
 * - Player 3: Left striker (-circleRadius-1.0, -2.0)
 * - Player 4: Right back striker (-circleRadius-2.5, 1.5)
 * - Player 5: Left back striker (-circleRadius-2.5, -1.5)
 * 
 * Goal Keepers:
 * - Player 1: Center goalie (-length/2+goalAreaLength+0.5, 0.0)
 * - Player 2: Right goalie (-length/2+goalAreaLength+0.5, 1.5)
 * - Player 3: Left goalie (-length/2+goalAreaLength+0.5, -1.5)
 * - Player 4: Right back goalie (-length/2+goalAreaLength+1.0, 2.0)
 * - Player 5: Left back goalie (-length/2+goalAreaLength+1.0, -2.0)
 * 
 * Falls back to original dynamic logic for unknown player IDs
 */
class GoToReadyPosition : public SyncActionNode
{
public:
    GoToReadyPosition(const std::string &name, const NodeConfig &config, Brain *_brain) : SyncActionNode(name, config), brain(_brain) {}

    static BT::PortsList providedPorts()
    {
        return {
            InputPort<double>("dist_tolerance", 0.8, "x tolerance"),
            InputPort<double>("theta_tolerance", 0.5, "theta tolerance"),
            InputPort<double>("vx_limit", 0.8, "vx limit"),
            InputPort<double>("vy_limit", 0.5, "vy limit"),
        };
    }

    BT::NodeStatus tick() override;

private:
    Brain *brain;
};


class GoToGoalBlockingPosition : public SyncActionNode
{
public:
    GoToGoalBlockingPosition(const std::string &name, const NodeConfig &config, Brain *_brain) : SyncActionNode(name, config), brain(_brain) {}

    static BT::PortsList providedPorts()
    {
        return {
            InputPort<double>("dist_tolerance", 0.8, "dist tolerance, within which considered arrived."),
            InputPort<double>("theta_tolerance", 0.8, "theta tolerance, winin which considered arrived."),
            InputPort<double>("vx_limit", 0.1, "x speed limit"),
            InputPort<double>("vy_limit", 0.1, "y speed limit"),
            InputPort<double>("dist_to_goalline", 2.5, "机器人站在门前多少距离"),
        };
    }

    BT::NodeStatus tick() override;

private:
    Brain *brain;
};


class Assist : public SyncActionNode
{
public:
    Assist(const std::string &name, const NodeConfig &config, Brain *_brain) : SyncActionNode(name, config), brain(_brain) {}

    static BT::PortsList providedPorts()
    {
        return {
            InputPort<double>("dist_tolerance", 0.8, "dist tolerance, within which considered arrived."),
            InputPort<double>("theta_tolerance", 0.8, "theta tolerance, winin which considered arrived."),
            InputPort<double>("vx_limit", 0.1, "x speed limit"),
            InputPort<double>("vy_limit", 0.1, "y speed limit"),
            InputPort<double>("dist_to_goalline", 2.5, "机器人站在门前多少距离"),
        };
    }

    BT::NodeStatus tick() override;

private:
    Brain *brain;
};


/**
 * @brief 设置机器人的速度
 *
 * @param x,y,theta double, 机器人在 x，y 方向上的速度（m/s）和逆时针转动的角速度（rad/s), 默认值为 0. 全为 0 时，即相当于给出站立不动指令
 *
 */
class SetVelocity : public SyncActionNode
{
public:
    SetVelocity(const string &name, const NodeConfig &config, Brain *_brain) : SyncActionNode(name, config), brain(_brain) {}

    NodeStatus tick() override;
    static PortsList providedPorts()
    {
        return {
            InputPort<double>("x", 0, "Default x is 0"),
            InputPort<double>("y", 0, "Default y is 0"),
            InputPort<double>("theta", 0, "Default  theta is 0"),
        };
    }

private:
    Brain *brain;
};

// 原地踏步
class StepOnSpot : public SyncActionNode
{
public:
    StepOnSpot(const string &name, const NodeConfig &config, Brain *_brain) : SyncActionNode(name, config), brain(_brain) {}

    NodeStatus tick() override;
    static PortsList providedPorts()
    {
        return {};
    }

private:
    Brain *brain;
};

class WaveHand : public SyncActionNode
{
public:
    WaveHand(const std::string &name, const NodeConfig &config, Brain *_brain) : SyncActionNode(name, config), brain(_brain)
    {
    }

    NodeStatus tick() override;

    static BT::PortsList providedPorts()
    {
        return {
            InputPort<string>("action", "start", "start | stop"),
        };
    }

private:
    Brain *brain;
};

class MoveHead : public SyncActionNode
{
public:
    MoveHead(const std::string &name, const NodeConfig &config, Brain *_brain) : SyncActionNode(name, config), brain(_brain)
    {
    }

    NodeStatus tick() override;

    static BT::PortsList providedPorts()
    {
        return {
            InputPort<double>("pitch", 0, "target head pitch"),
            InputPort<double>("yaw", 0, "target head yaw"),
        };
    }

private:
    Brain *brain;
};


class CheckAndStandUp : public SyncActionNode
{
public:
CheckAndStandUp(const string &name, const NodeConfig &config, Brain *_brain) : SyncActionNode(name, config), brain(_brain) {}

    static PortsList providedPorts() {
        return {};
    }

    NodeStatus tick() override;

private:
    Brain *brain;
};


class GoToFreekickPosition : public StatefulActionNode
{
public:
    GoToFreekickPosition(const string &name, const NodeConfig &config, Brain *_brain) : StatefulActionNode(name, config), brain(_brain) {}

    static PortsList providedPorts()
    {
        return {
            InputPort<string>("side", "attack", "attack | defense"),
            InputPort<double>("attack_dist", 0.7, "attack side target dist to ball"),
            InputPort<double>("defense_dist", 1.9, "defense side target dist to ball"),
            InputPort<double>("vx_limit", 1.2, "vx limit"),
            InputPort<double>("vy_limit", 0.5, "vy limit"),

        };
    }

    NodeStatus onStart() override;

    NodeStatus onRunning() override;

    void onHalted() override;

private:
    Brain *brain;
    bool _isInFinalAdjust = false; 
};

// 回到场地内
class GoBackInField : public SyncActionNode
{
public:
    GoBackInField(const string &name, const NodeConfig &config, Brain *_brain) : SyncActionNode(name, config), brain(_brain) {}

    static PortsList providedPorts()
    {
        return {
            InputPort<double>("valve", 0.5, "回到场内距离边界多远可以停止"),
        };
    }

    NodeStatus tick() override;

private:
    Brain *brain;
};


class SimpleChase : public SyncActionNode
{
public:
    SimpleChase(const string &name, const NodeConfig &config, Brain *_brain) : SyncActionNode(name, config), brain(_brain) {}

    static PortsList providedPorts()
    {
        return {
            InputPort<double>("stop_dist", 1.0, "在距离球多远的距离, 就不再走向球了"),
            InputPort<double>("stop_angle", 0.1, "球的角度在多少时, 就不再转向球了"),
            InputPort<double>("vy_limit", 0.2, "限制 Y 方向速度, 以防止走路不稳定. 要起作用需要小于机器本身的限速 0.4"),
            InputPort<double>("vx_limit", 0.6, "限制 X 方向速度, 以防止走路不稳定. 要起作用需要小于机器本身的限速 1.2"),
        };
    }

    NodeStatus tick() override;

private:
    Brain *brain;
};


class CalibrateOdom : public SyncActionNode
{
public:
    CalibrateOdom(const string &name, const NodeConfig &config, Brain *_brain) : SyncActionNode(name, config), brain(_brain) {}

    static PortsList providedPorts()
    {
        return {
            InputPort<double>("x", 0, "x"),
            InputPort<double>("y", 0, "y"),
            InputPort<double>("theta", 0, "theta"),
        };
    }

    NodeStatus tick() override;

private:
    Brain *brain;
};


// 向 cout 打印文字
class PrintMsg : public SyncActionNode
{
public:
    PrintMsg(const std::string &name, const NodeConfig &config, Brain *_brain)
        : SyncActionNode(name, config)
    {
    }

    NodeStatus tick() override;

    static PortsList providedPorts()
    {
        return {InputPort<std::string>("msg")};
    }

private:
    Brain *brain;
};

// 播放一些预定义的声音
class PlaySound : public SyncActionNode
{
public:
    PlaySound(const std::string &name, const NodeConfig &config, Brain *_brain) : SyncActionNode(name, config), brain(_brain)
    {
    }

    NodeStatus tick() override;

    static BT::PortsList providedPorts()
    {
        return {
            InputPort<string>("sound", "cheerful", "播放声音的名称"),
            InputPort<bool>("allow_repeat", false, "是否允许重复播放同一个声音"),
        };
    }

private:
    Brain *brain;
};

// 使用本地 tts (espeak) 朗读文本
class Speak : public SyncActionNode
{
public:
    Speak(const std::string &name, const NodeConfig &config, Brain *_brain) : SyncActionNode(name, config), brain(_brain)
    {
    }

    NodeStatus tick() override;

    static BT::PortsList providedPorts()
    {
        return {
            InputPort<string>("text", "", "朗读的文本内容, 必须是英文"),
        };
    }

private:
    Brain *brain;
};
