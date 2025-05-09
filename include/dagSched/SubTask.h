#ifndef SUBTASK_H
#define SUBTASK_H

#include <vector>
#include "dagSched/utils.h"

namespace dagSched{

// 子任务模式枚举
// NORMAL_T: 普通任务节点
// C_INTERN_T: 条件分支内部节点
// C_SOURCE_T: 条件分支源节点
// C_SINK_T: 条件分支汇节点
enum subTaskMode {NORMAL_T, C_INTERN_T, C_SOURCE_T, C_SINK_T};

// 子任务类，表示DAG中的一个任务节点
class SubTask{

    public:
    // 基本属性
    int id          = 0;    // 子任务唯一标识符
    int depth       = 0;    // 在DAG中的深度
    int width       = 0;    // 在DAG中的宽度
    int gamma       = 0;    // 核心类型(处理器类型)
    int core        = 0;    // 分配的核心ID
    int prio        = 0;    // 子任务优先级

    // 时间相关属性
    float c         = 0;    // 最坏情况执行时间(WCET)
    float accWork   = 0;    // 累计工作量
    float r         = 0;    // 子任务响应时间
    float localO    = -1;   // 本地偏移量(最早开始时间)
    float localD    = -1;   // 本地截止时间(最晚完成时间)
    float EFT       = -1;   // 最早完成时间
    float LST       = -1;   // 最晚开始时间

    subTaskMode mode = NORMAL_T;    // 节点类型

    // 图结构关系
    std::vector<SubTask*> succ;     // 后继节点列表
    std::vector<SubTask*> pred;     // 前驱节点列表

    // 获取条件前驱节点ID列表
    std::vector<int> getCondPred();

    // 计算本地偏移量(最早开始时间)
    void localOffset();
    // 计算最早完成时间
    void EasliestFinishingTime();
    // 计算本地截止时间
    void localDeadline(const float task_deadline);
    // 计算最晚开始时间
    void LatestStartingTime();
};

}

#endif /* SUBTASK_H */