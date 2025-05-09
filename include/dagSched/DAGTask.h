#ifndef DAGTASK_H
#define DAGTASK_H

#include <vector>
#include <map>
#include <cmath>
#include <cstdlib>
#include <random>
#include <algorithm>
#include <iostream>
#include <fstream>

#include <yaml-cpp/yaml.h>
#include "dagSched/SubTask.h"
#include "dagSched/utils.h"
#include "dagSched/GeneratorParams.h"

namespace dagSched{
   
// 任务创建状态枚举
enum creationStates {CONDITIONAL_T=0, PARALLEL_T=1, TERMINAL_T=2};

// DAG任务类，表示一个有向无环图任务
class DAGTask{

    std::vector<SubTask*> V;  // 子任务顶点集合

    // 任务参数
    float t = 0;          // 周期(period)
    float d = 0;          // 截止时间(deadline)
    float L = 0;          // 最长链长度(longest chain)
    float vol  = 0;       // 总体积(volume)
    float wcw = 0;        // 最坏情况工作负载(worst case workload)
    float delta = 0;      // 密度(density)
    float u = 0;          // 利用率(utilization)

    std::vector<int> ordIDs;        // 拓扑排序后的ID序列
    std::map<int, float> typedVol;  // 按核心类型分类的体积 [核心类型, 体积]
    std::map<int, float> pVol;      // 按分区分类的体积 [核心ID, 体积]

    public:

    float R = 0;          // 响应时间(response time)

    DAGTask(){};
    DAGTask(const float T, const float D): t(T), d(D) {};
    ~DAGTask(){};

    // 输入输出操作
    void readTaskFromYamlNode(YAML::Node tasks, const int i);  // 从YAML节点读取任务
    void readTaskFromDOT(const std::string &filename);        // 从DOT文件读取任务
    void saveAsDot(const std::string &filename);               // 保存为DOT格式
    friend std::ostream& operator<<(std::ostream& os, const DAGTask& t); // 输出运算符重载

    // DAG操作
    void cloneVertices(const std::vector<SubTask*>& to_clone_V); // 克隆顶点
    void destroyVerices(); // 销毁顶点
    void isSuccessor(const SubTask* v, const SubTask *w, bool &is_succ) const; // 判断后继关系
    bool allPrecAdded(std::vector<SubTask*> pred, std::vector<int> ids); // 检查所有前驱是否已添加
    void topologicalSort(); // 拓扑排序
    bool checkIndexAndIdsAreEqual(); // 检查索引和ID是否匹配
    void computeAccWorkload(); // 计算累计工作负载
    void computeLength(); // 计算最长链长度
    void computeVolume(); // 计算总体积
    void computeTypedVolume(); // 计算类型化体积
    void computepVolume(); // 计算分区体积
    void computeWorstCaseWorkload(); // 计算最坏情况工作负载
    void computeUtilization(); // 计算利用率
    void computeDensity(); // 计算密度
    std::vector<std::vector<int>> computeAllPathsSingleSource(std::vector<int>& path, std::vector<std::vector<int>>& all_paths) const; // 计算所有路径(单源)
    std::vector<std::vector<int>> computeAllPaths() const; // 计算所有路径

    // 时间分析相关方法
    void localDeadline(SubTask *task, const int i); // 计算本地截止时间
    void localOffset(SubTask *task, const int i); // 计算本地偏移量
    void computeLocalOffsets(); // 计算所有本地偏移量
    void computeLocalDeadlines(); // 计算所有本地截止时间
    void computeEFTs(); // 计算最早完成时间
    void computeLSTs(); // 计算最晚开始时间

    // 获取顶点关系
    std::vector<SubTask*> getSubTaskAncestors(const int i) const; // 获取祖先顶点
    std::vector<SubTask*> getSubTaskDescendants(const int i) const; // 获取后代顶点
    void transitiveReduction(); // 传递约简

    // 获取方法
    float getLength() const {return L;}; // 获取最长链长度
    float getVolume() const {return vol;}; // 获取总体积
    std::map<int, float> getTypedVolume() const {return typedVol;}; // 获取类型化体积
    std::map<int, float> getpVolume() const {return pVol;}; // 获取分区体积
    float getWorstCaseWorkload() const {return wcw;}; // 获取最坏情况工作负载
    float getWCW() const {return wcw;}; // 获取最坏情况工作负载(别名)
    float getPeriod() const {return t;}; // 获取周期
    float getDeadline() const {return d;}; // 获取截止时间
    float getUtilization() const {return u;}; // 获取利用率
    float getDensity() const {return delta;}; // 获取密度
    std::vector<int> getTopologicalOrder() const {return ordIDs;}; // 获取拓扑排序
    std::vector<SubTask*> getVertices() const {return V;}; // 获取顶点集合

    // 设置方法
    void setVertices(std::vector<SubTask*> given_V){ V.clear(); V = given_V; } // 设置顶点集合
    void setDeadline(const float deadline) { d = deadline; } // 设置截止时间
    void setPeriod(const float period) { t = period; } // 设置周期

    // Melani生成方法
    void assignWCET(const int minC, const int maxC); // 分配最坏执行时间
    void expandTaskSeriesParallel(SubTask* source,SubTask* sink,const int depth,const int numBranches, const bool ifCond, GeneratorParams& gp); // 扩展串并行任务
    void makeItDag(float prob); // 转换为DAG
    void assignSchedParametersUUniFast(const float U); // 使用UUniFast分配调度参数
    void assignSchedParameters(const float beta); // 分配调度参数
    void assignFixedSchedParameters(const float period, const float deadline); // 分配固定调度参数
};

// DAG比较函数
bool compareDAGsDeadlineInc(const DAGTask& a, const DAGTask& b); // 按截止时间递增比较
bool compareDAGsDeadlineDec(const DAGTask& a, const DAGTask& b); // 按截止时间递减比较
bool compareDAGsPeriodInc(const DAGTask& a, const DAGTask& b); // 按周期递增比较
bool compareDAGsPeriodDec(const DAGTask& a, const DAGTask& b); // 按周期递减比较
bool compareDAGsUtilInc(const DAGTask& a, const DAGTask& b); // 按利用率递增比较
bool compareDAGsUtilDec(const DAGTask& a, const DAGTask& b); // 按利用率递减比较

} 

#endif /* DAGTASK_H */