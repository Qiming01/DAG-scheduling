#ifndef UTILS_H
#define UTILS_H

#include<vector>
#include<iostream>
#include<sstream>
#include<cmath>
#include<set>
#include<utility>
#include<algorithm>
#include<iomanip>

// 定义可复现模式标志
#define REPRODUCIBLE 1

// 致命错误处理宏
#define FatalError(s) {                                                \
    std::stringstream _where, _message;                                \
    _where << __FILE__ << ':' << __LINE__;                             \
    _message << std::string(s) + "\n" << __FILE__ << ':' << __LINE__;\
    std::cerr << _message.str() << "\nAborting...\n";                  \
    exit(EXIT_FAILURE);                                                \
}

// 控制台颜色输出定义
#define COL_END "\033[0m"          // 重置颜色
#define COL_CYANB "\033[1;36m"     // 加粗青色

// 调试输出控制标志
#define TIME_VERBOSE 1             // 是否显示时间统计
#define METHOD_VERBOSE 0           // 是否显示方法调用信息

// 计时开始宏
#define TSTART timespec start, end;                               \
                    clock_gettime(CLOCK_MONOTONIC, &start);            

// 计时结束宏(带颜色和显示控制)
#define TSTOP_C(col, show)  clock_gettime(CLOCK_MONOTONIC, &end);       \
    double t_ns = ((double)(end.tv_sec - start.tv_sec) * 1.0e9 +       \
                  (double)(end.tv_nsec - start.tv_nsec))/1.0e6;        \
    if(show) std::cout<<col<<"Time:"<<std::setw(16)<<t_ns<<" ms\n"<<COL_END; 

// 默认计时结束宏(使用青色)
#define TSTOP TSTOP_C(COL_CYANB, TIME_VERBOSE)

// 简单计时器类
class SimpleTimer{
    timespec tStart;  // 开始时间
    timespec tEnd;    // 结束时间

public:
    // 时间单位枚举
    enum TimeUnit_t { SECOND, MILLISECOND, MICROSECOND, NANOSECOND};

    // 开始计时
    void tic(){
        clock_gettime(CLOCK_MONOTONIC, &tStart);
    }

    // 结束计时并返回耗时
    double toc(TimeUnit_t time_unit=TimeUnit_t::MICROSECOND){
        clock_gettime(CLOCK_MONOTONIC, &tEnd);
        double t_ns = ((double)(tEnd.tv_sec - tStart.tv_sec) * 1.0e9 + (double)(tEnd.tv_nsec - tStart.tv_nsec));
        // 根据时间单位转换
        switch (time_unit){
        case TimeUnit_t::SECOND:
            t_ns/=1.0e9;  
            break;
        case TimeUnit_t::MILLISECOND:
            t_ns/=1.0e6;  
            break;
        case TimeUnit_t::MICROSECOND:
            t_ns/=1.0e3;  
            break;
        case TimeUnit_t::NANOSECOND:
            break;
        }
        return t_ns;
    }
};

/*
 * 集合差运算(原地修改)
 * 给定集合a和b，计算a \ b并将结果保存在a中
 */
template<typename T>
void set_difference_inplace(std::set<T>& a, const std::set<T>& b)
{
    std::set<T> c;
    std::set_difference(a.begin(), a.end(),b.begin(), b.end(), std::inserter(c, c.begin()) );
    a = c;
}

// 打印vector内容
template<typename T>
void printVector(const std::vector<T>& v, const std::string& name = ""){
    std::cout<<name<<": ";
    for(const auto &val:v)
        std::cout<<val<<" ";
    std::cout<<std::endl;
}

// 打印set内容
template<typename T>
void printSet(const std::set<T>& v, const std::string& name = ""){
    std::cout<<name<<": ";
    for(const auto &val:v)
        std::cout<<val<<" ";
    std::cout<<std::endl;
}

// 打印pair vector内容
template<typename T1, typename T2>
void printPairVector(const std::vector<std::pair<T1, T2>>& v, const std::string& name = ""){
    std::cout<<name<<": ";
    for(const auto &val:v)
        std::cout<<val.first<<":"<<val.second<<"  ";
    std::cout<<std::endl;
}

// 浮点数相等比较
template<typename T>
bool areEqual(const T a, const T b){
    return std::fabs(a - b) < std::numeric_limits<T>::epsilon();
}

// 函数声明
int intRandMaxMin(const int v_min, const int v_max);
float floatRandMaxMin(const float v_min, const float v_max);
void removePathAndExtension(const std::string &full_string, std::string &name);

// DOT文件行类型枚举
enum DOTLine_t {DOT_BEGIN, DOT_END, DOT_NODE, DOT_EDGE, DAG_INFO, VOID_LINE};

// DOT文件解析信息结构体
struct dot_info{
    DOTLine_t lineType;  // 行类型
    int p           = -1;  // 处理器核心分配
    int s           = -1;  // 引擎类型分配
    int id          = -1;  // 节点ID
    int id_from     = -1;  // 起始节点ID(边)
    int id_to       = -1;  // 目标节点ID(边)
    float wcet      = 0;   // 最坏情况执行时间
    float period    = 0;   // 周期
    float deadline  = 0;   // 截止时间
};

// 在逗号处分割字符串
std::vector<std::pair<std::string, std::string>> separateOnComma(const std::string& line);
// 解析DOT文件行
dot_info parseDOTLine(const std::string& line);

#endif /*UTILS_H*/