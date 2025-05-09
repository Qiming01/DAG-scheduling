#include "dagSched/DAGTask.h"

namespace dagSched{

// 重载输出运算符，用于打印DAG任务信息
std::ostream& operator<<(std::ostream& os, const DAGTask& t)
{
    // 打印任务基本信息
    os<<"----------------------------------------------------\n";
    os<< "deadline :" << t.d <<std::endl;  // 截止时间
    os<< "period :" << t.t <<std::endl;    // 周期
    os<< "length :" << t.L <<std::endl;    // 最长链长度
    os<< "volume :" << t.vol <<std::endl;  // 总体积
    os<< "wcw :" << t.wcw <<std::endl;     // 最坏情况工作负载
    os<< "utilization :" << t.u <<std::endl; // 利用率
    os<< "density :" << t.delta <<std::endl; // 密度
    
    // 打印所有顶点信息
    os<< "vertices :"<<std::endl;
    for(auto v: t.V){
        os<< "\t v_" << v->id << " - c: "<<v->c <<" \tsucc: ";
        // 打印后继顶点ID
        for(auto s:v->succ)
            os<< s->id << " ";
        os<<" \tprec: ";
        // 打印前驱顶点ID
        for(auto p:v->pred)
            os<< p->id << " ";

        // 打印顶点其他属性
        os<<" \tcore: " << v->core;        // 分配的核心
        os<<" \tlocal O: " << v->localO;   // 本地偏移量
        os<<" \tlocal D: " << v->localD;   // 本地截止时间
        os<<" \n";
    }
    
    // 打印拓扑排序结果(如果有)
    if(t.ordIDs.size()){
        std::cout<<"Topological order: ";
        for(auto id:t.ordIDs)
            std::cout<<id<< " ";
        std::cout<<std::endl;
    }

    os<<"----------------------------------------------------\n";
    return os;
}

// 从YAML节点读取任务信息
void DAGTask::readTaskFromYamlNode(YAML::Node tasks, const int i){
    // 读取周期和截止时间
    t = tasks[i]["t"].as<int>();
    d = tasks[i]["d"].as<int>();

    // 读取顶点信息
    YAML::Node vert = tasks[i]["vertices"];
    std::map<int, int> id_pos;  // 映射原始ID到位置索引

    for(int j=0; j<vert.size(); j++){
        SubTask *v = new SubTask;
        v->id = j;  // 使用连续编号作为新ID
        v->c = vert[j]["c"].as<int>();  // 读取WCET

        // 建立原始ID到新ID的映射
        id_pos[vert[j]["id"].as<int>()] = j;

        // 读取可选属性
        if(vert[j]["s"])
            v->gamma = vert[j]["s"].as<int>();  // 核心类型
        if(vert[j]["p"])
            v->core = vert[j]["p"].as<int>();   // 分配的核心

        V.push_back(v);  // 添加到顶点集合
    }

    // 读取边信息并建立连接关系
    YAML::Node edges = tasks[i]["edges"];
    int form_id, to_id;
    for(int j=0; j<edges.size(); j++){
        // 使用映射表转换原始ID
        form_id = id_pos[edges[j]["from"].as<int>()];
        to_id = id_pos[edges[j]["to"].as<int>()];

        // 建立前驱后继关系
        V[form_id]->succ.push_back(V[to_id]);
        V[to_id]->pred.push_back(V[form_id]);
    }
}

// 从DOT文件读取任务信息
void DAGTask::readTaskFromDOT(const std::string &filename){
    std::ifstream dot_dag(filename);
    std::string line;
    int node_count = 0;
    int form_id, to_id;

    std::map<int, int> id_pos;  // 映射原始ID到位置索引
    while (std::getline(dot_dag, line)){
        dot_info di = parseDOTLine(line);  // 解析DOT行

        // 根据行类型处理
        if (di.lineType == DOTLine_t::DAG_INFO){
            // 读取DAG基本信息
            t = di.period;
            d = di.deadline;
        }
        else if (di.lineType == DOTLine_t::DOT_NODE){
            // 处理顶点信息
            SubTask *v = new SubTask;
            v->id = node_count;
            v->c = di.wcet;
            id_pos[di.id] = node_count;  // 建立ID映射

            // 读取可选属性
            if(di.s != -1)
                v->gamma = di.s;  // 核心类型
            if(di.p != -1)
                v->core = di.p;   // 分配的核心

            V.push_back(v);  // 添加到顶点集合
            node_count++;
        }
        else if (di.lineType == DOTLine_t::DOT_EDGE){
            // 处理边信息
            form_id = id_pos[di.id_from];
            to_id = id_pos[di.id_to];

            // 建立前驱后继关系
            V[form_id]->succ.push_back(V[to_id]);
            V[to_id]->pred.push_back(V[form_id]);
        }
    }
    dot_dag.close();
}

// 将任务保存为DOT格式
void DAGTask::saveAsDot(const std::string &filename){
    std::ofstream of(filename);

    of<<"digraph Task {\n";

    // 输出DAG基本信息
    of<<"i [shape=box, label=\"D="<<d<<" T="<<t<<"\"]; \n";
    
    // 输出所有顶点
    for (const auto &v: V){
        of<<v->id<<" [label=\""<<v->c<<"("<<v->id<<", p:"<<v->core<<")"<<"\"";
        // 根据顶点类型设置形状
        if(v->mode == C_SOURCE_T) of<<",shape=diamond";  // 条件源节点
        else if(v->mode == C_SINK_T) of<<",shape=box";   // 条件汇节点
        of<<"];\n";
    }

    // 输出所有边
    for (const auto &v: V){
        for(auto s: v->succ)
            of<<v->id<<" -> "<<s->id<<";\n";
    }
    of<<"}";

    of.close();
}
}