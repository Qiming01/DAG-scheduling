
#ifndef GENERATORPARAMS_H
#define GENERATORPARAMS_H

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <random>
#include <yaml-cpp/yaml.h>

#include "dagSched/utils.h"

namespace dagSched{

// 生成类型枚举
// VARYING_N: 变化任务数量
// VARYING_U: 变化利用率
// VARYING_M: 变化处理器数量
enum GenerationType_t {VARYING_N, VARYING_U, VARYING_M};

// 截止时间类型枚举
// CONSTRAINED: 受限截止时间
// IMPLICIT: 隐式截止时间
// ARBITRARY: 任意截止时间
enum DeadlinesType_t {CONSTRAINED, IMPLICIT, ARBITRARY};

// 调度类型枚举
// GLOBAL: 全局调度
// PARTITIONED: 分区调度
// FEDERATED: 联邦调度
// SOTA: 最先进调度
enum SchedulingType_t {GLOBAL, PARTITIONED, FEDERATED, SOTA};

// 算法类型枚举
// FTP: 固定优先级调度
// EDF: 最早截止时间优先调度
enum AlgorithmType_t {FTP, EDF};

// 工作负载类型枚举
// SINGLE_DAG: 单个DAG
// TASKSET: 任务集
enum workloadType_t {SINGLE_DAG, TASKSET};

// DAG类型枚举
// DAG: 普通有向无环图
// CDAG: 条件有向无环图
// TDAG: 类型化有向无环图
enum DAGType_t {DAG, CDAG, TDAG};

// DAG生成参数类
// 用于配置生成DAG的各种参数
class GeneratorParams{

    public:

    // Melani等人提出的DAG生成算法参数
    int maxCondBranches     = 2;    // 最大允许的条件分支数
    int maxParBranches      = 6;    // 最大允许的并行分支数
    int recDepth            = 2;    // 任务图生成的最大递归深度

    // 概率参数
    float pCond             = 0;    // 生成条件分支的概率
    float pPar              = 0.2;  // 生成并行分支的概率
    float pTerm             = 0.8;  // 生成终止顶点的概率
    
    // WCET参数
    float Cmin              = 1;    // 子任务最小WCET
    float Cmax              = 100;  // 子任务最大WCET
    
    // 其他参数
    float addProb           = 0.1;  // 可能时在两个节点间添加边的概率
    float probSCond         = 0.5;  // 源节点是条件节点的概率
    float Utot              = 1;    // 总利用率
    float beta              = 0.1;  // 参数beta

    int saveRate            = 25;   // 保存频率

    // 利用率范围参数
    float Umin              = 0;    // 最小利用率
    float Umax              = 8;    // 最大利用率
    float stepU             = 0.25; // 利用率步长
    int m                   = 8;    // 处理器数量

    // 任务数量参数
    int nMin                = 11;   // 最小任务数
    int nMax                = 20;   // 最大任务数
    int nTasks              = 0;    // 任务数量
    int stepN               = 1;    // 任务数步长

    // 处理器数量参数
    int mMin                = 2;    // 最小处理器数
    int mMax                = 30;   // 最大处理器数
    int stepM               = 1;    // 处理器数步长

    // 任务集参数
    int tasksetPerVarFactor = 1;    // 每个变量的任务集因子
    int nTasksets           = 1;    // 任务集数量

    // 处理器类型参数
    int diffProcTypes       = 1;    // 不同处理器类型数量
    int minProcPerType      = 1;    // 每种类型最小处理器数
    int maxProcPerType      = 1;    // 每种类型最大处理器数

    std::vector<int> typedProc;     // 类型化处理器列表

    // 枚举参数
    GenerationType_t gType  = GenerationType_t::VARYING_N;  // 生成类型
    DeadlinesType_t dtype   = DeadlinesType_t::CONSTRAINED; // 截止时间类型
    SchedulingType_t sType  = SchedulingType_t::GLOBAL;     // 调度类型
    AlgorithmType_t aType   = AlgorithmType_t::FTP;         // 算法类型
    workloadType_t wType    = workloadType_t::TASKSET;      // 工作负载类型
    DAGType_t DAGType       = DAGType_t::DAG;               // DAG类型

    // 随机分布相关
    std::discrete_distribution<int> dist;  // 分支添加分布
    std::vector<double> weights;          // 权重向量
    std::mt19937 gen;                      // 随机数生成器

    // 配置参数方法
    void configureParams(GenerationType_t gt){
        gType = gt;

        // 设置权重向量
        weights.push_back(pCond);
        weights.push_back(pPar);
        weights.push_back(pTerm);
        dist.param(std::discrete_distribution<int> ::param_type(std::begin(weights), std::end(weights)));

        // 根据生成类型设置任务集数量
        switch (gt){
        case VARYING_N:
            nTasksets = tasksetPerVarFactor * (nMax - nMin + 1);
            break;
        case VARYING_U:
            nTasksets = tasksetPerVarFactor * ((Umax - Umin) / stepU);
            break;
        case VARYING_M:
            mMin = std::floor(Utot);
            nTasksets = tasksetPerVarFactor * (mMax - mMin + 1);
            break;
        }

        // 根据DAG类型调整参数
        if(DAGType != DAGType_t::CDAG){
            pCond = 0;
            probSCond = 0;
        }

        // 类型化DAG的特殊处理
        if(DAGType != DAGType_t::TDAG){
            typedProc.resize(diffProcTypes);
            for(int p=0; p<typedProc.size(); ++p)
                typedProc[p] = rand () % maxProcPerType + minProcPerType;
        }

        // 单个DAG的特殊处理
        if(wType == workloadType_t::SINGLE_DAG)
            nTasks = 1;
        
        // 设置随机种子
        if(REPRODUCIBLE) gen.seed(1);
        else gen.seed(time(0));
    }

    // 从YAML文件读取参数
    void readFromYaml(const std::string& params_path){
        YAML::Node config = YAML::LoadFile(params_path);
        
        // 读取各种参数
        if(config["maxCondBranches"]) maxCondBranches = config["maxCondBranches"].as<int>();
        if(config["maxParBranches"]) maxParBranches = config["maxParBranches"].as<int>();
        if(config["recDepth"]) recDepth = config["recDepth"].as<int>();
        if(config["pCond"]) pCond = config["pCond"].as<float>();
        if(config["pPar"]) pPar = config["pPar"].as<float>();
        if(config["pTerm"]) pTerm = config["pTerm"].as<float>();
        if(config["Cmin"]) Cmin = config["Cmin"].as<float>();
        if(config["Cmax"]) Cmax = config["Cmax"].as<float>();
        if(config["addProb"]) addProb = config["addProb"].as<float>();
        if(config["probSCond"]) probSCond = config["probSCond"].as<float>();
        if(config["Utot"]) Utot = config["Utot"].as<float>();
        if(config["beta"]) beta = config["beta"].as<float>();
        if(config["saveRate"]) saveRate = config["saveRate"].as<int>();
        if(config["Umin"]) Umin = config["Umin"].as<float>();
        if(config["Umax"]) Umax = config["Umax"].as<float>();
        if(config["stepU"]) stepU = config["stepU"].as<float>();
        if(config["m"]) m = config["m"].as<int>();
        if(config["nMin"]) nMin = config["nMin"].as<int>();
        if(config["nMax"]) nMax = config["nMax"].as<int>();
        if(config["nTasks"]) nTasks = config["nTasks"].as<int>();
        if(config["stepN"]) stepN = config["stepN"].as<int>();
        if(config["mMin"]) mMin = config["mMin"].as<int>();
        if(config["mMax"]) mMax = config["mMax"].as<int>();
        if(config["stepM"]) stepM = config["stepM"].as<int>();
        if(config["tasksetPerVarFactor"]) tasksetPerVarFactor = config["tasksetPerVarFactor"].as<int>();
        if(config["nTasksets"]) nTasksets = config["nTasksets"].as<int>();
        if(config["diffProcTypes"]) diffProcTypes = config["diffProcTypes"].as<int>();
        if(config["minProcPerType"]) minProcPerType = config["minProcPerType"].as<int>();
        if(config["maxProcPerType"]) maxProcPerType = config["maxProcPerType"].as<int>();
        if(config["gType"]) gType = (GenerationType_t)  config["gType"].as<int>(); 
        if(config["dtype"]) dtype = (DeadlinesType_t) config["dtype"].as<int>();
        if(config["sType"]) sType = (SchedulingType_t) config["sType"].as<int>();
        if(config["aType"]) aType = (AlgorithmType_t) config["aType"].as<int>();
        if(config["wType"]) wType = (workloadType_t) config["wType"].as<int>();
        if(config["DAGType"]) DAGType = (DAGType_t) config["DAGType"].as<int>();
    }

    // 打印参数
    void print(){
        std::cout<<"maxCondBranches: "<<maxCondBranches<<std::endl;
        std::cout<<"maxParBranches: "<<maxParBranches<<std::endl;
        std::cout<<"recDepth: "<<recDepth<<std::endl;
        std::cout<<"pCond: "<<pCond<<std::endl;
        std::cout<<"pPar: "<<pPar<<std::endl;
        std::cout<<"pTerm: "<<pTerm<<std::endl;
        std::cout<<"Cmin: "<<Cmin<<std::endl;
        std::cout<<"Cmax: "<<Cmax<<std::endl;
        std::cout<<"addProb: "<<addProb<<std::endl;
        std::cout<<"probSCond: "<<probSCond<<std::endl;
        std::cout<<"Utot: "<<Utot<<std::endl;
        std::cout<<"beta: "<<beta<<std::endl;
        std::cout<<"saveRate: "<<saveRate<<std::endl;
        std::cout<<"Umin: "<<Umin<<std::endl;
        std::cout<<"Umax: "<<Umax<<std::endl;
        std::cout<<"stepU: "<<stepU<<std::endl;
        std::cout<<"m: "<<m<<std::endl;
        std::cout<<"nMin: "<<nMin<<std::endl;
        std::cout<<"nMax: "<<nMax<<std::endl;
        std::cout<<"nTasks: "<<nTasks<<std::endl;
        std::cout<<"stepN: "<<stepN<<std::endl;
        std::cout<<"mMin: "<<mMin<<std::endl;
        std::cout<<"mMax: "<<mMax<<std::endl;
        std::cout<<"stepM: "<<stepM<<std::endl;
        std::cout<<"tasksetPerVarFactor: "<<tasksetPerVarFactor<<std::endl;
        std::cout<<"nTasksets: "<<nTasksets<<std::endl;
        std::cout<<"diffProcTypes: "<<diffProcTypes<<std::endl;
        std::cout<<"minProcPerType: "<<minProcPerType<<std::endl;
        std::cout<<"maxProcPerType: "<<maxProcPerType<<std::endl;
        std::cout<<"gType: "<<gType<<std::endl;
        std::cout<<"dtype: "<<dtype<<std::endl;
        std::cout<<"sType: "<<sType<<std::endl;
        std::cout<<"aType: "<<aType<<std::endl;
        std::cout<<"wType: "<<wType<<std::endl;
        std::cout<<"DAGType: "<<DAGType<<std::endl;
    }
};

}

#endif /* GENERATORPARAMS_H */