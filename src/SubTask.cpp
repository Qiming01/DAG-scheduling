#include "dagSched/SubTask.h"

namespace dagSched{

// 获取条件前驱节点ID列表
std::vector<int> SubTask::getCondPred(){
    std::vector<int> cond_pred;
    for(const auto& p:pred){
        // 只收集模式为C_SOURCE_T的前驱节点
        if(p->mode == C_SOURCE_T)
            cond_pred.push_back(p->id);
    }
    return cond_pred;
}

// 计算本地偏移量(最早开始时间)
void SubTask::localOffset(){
    if(pred.size() == 0)
        // 没有前驱节点，最早开始时间为0
        localO = 0;
    else{
        localO = 0;
        float temp_local_o = 0;
        // 遍历所有前驱节点，计算最大(前驱节点的最早开始时间 + 其执行时间)
        for(int i=0; i<pred.size();++i){
            temp_local_o = pred[i]->localO + pred[i]->c;
            if(temp_local_o > localO) localO = temp_local_o;
        }
    }
}

// 计算最早完成时间
void SubTask::EasliestFinishingTime(){
    if(localO == -1)
        FatalError("需要先计算本地偏移量");
    // 最早完成时间 = 最早开始时间 + 执行时间
    EFT = localO + c;
}

// 计算本地截止时间
void SubTask::localDeadline(const float task_deadline){
    if(succ.size() == 0)
        // 没有后继节点，使用任务全局截止时间
        localD = task_deadline;
    else{
        localD = 99999;  // 初始化为大数
        float temp_local_d = 0;
        // 遍历所有后继节点，计算最小(后继节点的本地截止时间 - 其执行时间)
        for(int i=0; i<succ.size();++i){
            temp_local_d = succ[i]->localD - succ[i]->c;
            if(temp_local_d < localD) localD = temp_local_d;
        }
    }
}

// 计算最晚开始时间
void SubTask::LatestStartingTime(){
    if(localD == -1)
        FatalError("需要先计算本地截止时间");
    // 最晚开始时间 = 本地截止时间 - 执行时间
    LST = localD - c;
}
   
}