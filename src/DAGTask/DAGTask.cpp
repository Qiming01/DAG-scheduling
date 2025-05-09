#include "dagSched/DAGTask.h"

namespace dagSched{

// 克隆顶点集合
// to_clone_V: 要克隆的顶点集合
void DAGTask::cloneVertices(const std::vector<SubTask*>& to_clone_V){
    V.clear();
    // 克隆每个顶点
    for(int i=0; i<to_clone_V.size();++i){
        SubTask * v = new SubTask;
        *v = *to_clone_V[i];  // 复制顶点内容
        V.push_back(v);
    }

    // 重建顶点间的连接关系
    for(int i=0; i<to_clone_V.size();++i){
        V[i]->pred.clear();
        V[i]->succ.clear();

        // 重建后继关系
        for(int j=0; j<to_clone_V[i]->succ.size();++j)
            V[i]->succ.push_back(V[to_clone_V[i]->succ[j]->id]);

        // 重建前驱关系
        for(int j=0; j<to_clone_V[i]->pred.size();++j)
            V[i]->pred.push_back(V[to_clone_V[i]->pred[j]->id]);
    }
}

// 销毁所有顶点
void DAGTask::destroyVerices(){
    for(int i=0; i<V.size();++i)
        delete V[i];  // 释放每个顶点内存
}

// 判断顶点v是否是w的后继
// v: 待判断顶点
// w: 起始顶点
// is_succ: 输出参数，表示是否为后继
void DAGTask::isSuccessor(const SubTask* v, const SubTask *w, bool &is_succ) const{
    for(size_t i=0; i<w->succ.size(); ++i){
        if(w->succ[i] == v){
            is_succ = true;
            return;
        }
        else{
            // 递归检查后继的后继
            isSuccessor(v, w->succ[i], is_succ);
        }
    }
    return;
}

// 获取顶点的所有祖先
// i: 顶点索引
// 返回: 祖先顶点集合
std::vector<SubTask*> DAGTask::getSubTaskAncestors(const int i) const{
    std::vector<SubTask*> ancst;
    bool is_succ = false;
    for(int j=0; j<V.size(); ++j){
        is_succ = false;
        isSuccessor(V[i], V[j], is_succ);
        if( is_succ )
            ancst.push_back(V[j]);
    }
    return ancst;
}

// 获取顶点的所有后代
// i: 顶点索引
// 返回: 后代顶点集合
std::vector<SubTask*> DAGTask::getSubTaskDescendants(const int i) const{
    std::vector<SubTask*> desc;
    bool is_succ = false;
    for(int j=0; j<V.size(); ++j){
        is_succ = false;
        isSuccessor(V[j], V[i], is_succ);
        if( is_succ )
            desc.push_back(V[j]);
    }
    return desc;
}

// 执行传递归约，移除冗余边
void DAGTask::transitiveReduction(){
    for(int i=0; i<V.size(); ++i){
        std::vector<SubTask*> succ_desc;
        // 收集所有后继的后代
        for(int j=0; j<V[i]->succ.size();++j){
            std::vector<SubTask*> succ_desc_j = getSubTaskDescendants(V[i]->succ[j]->id);
            succ_desc.insert(succ_desc.end(), succ_desc_j.begin(), succ_desc_j.end());
        }
        
        // 标记需要移除的边
        std::vector<SubTask*> to_remove;
        for(int j=0; j<V[i]->succ.size();++j){
            if ( std::find(succ_desc.begin(), succ_desc.end(), V[i]->succ[j]) != succ_desc.end() ){
                to_remove.push_back(V[i]->succ[j]);
            }
        }

        // 移除冗余边
        for(auto& r:to_remove){
            V[i]->succ.erase(std::remove(V[i]->succ.begin(), V[i]->succ.end(), r), V[i]->succ.end());
            r->pred.erase(std::remove(r->pred.begin(), r->pred.end(), V[i]), r->pred.end());
        }
    }
}

// 检查所有前驱是否已添加
// pred: 前驱顶点集合
// ids: 已添加顶点ID集合
// 返回: 是否所有前驱都已添加
bool DAGTask::allPrecAdded(std::vector<SubTask*> pred, std::vector<int> ids){
    bool prec_present;
    for(int i=0; i<pred.size();++i){
        prec_present = false;
        for(int j=0; j<ids.size();++j){
            if(pred[i]->id == ids[j]){
                prec_present = true;
                break;
            }
        }
        if(!prec_present)
            return false;
    }
    return true;
}

// 执行拓扑排序
void DAGTask::topologicalSort (){
    std::vector<SubTask> V_copy;
    ordIDs.clear();

    // 创建顶点副本
    for(const auto &v: V)
        V_copy.push_back(*v);

    bool nodes_to_add = true;
    while(nodes_to_add){
        nodes_to_add = false;
        for(auto &v: V_copy){
            if((v.pred.empty() || allPrecAdded(v.pred, ordIDs))){
                if(v.id != -1 ){
                    ordIDs.push_back(v.id);
                    v.id = -1;
                }                    
            }
            else
                nodes_to_add = true;
        }
    }

    // 验证排序结果
    if(!checkIndexAndIdsAreEqual())
        FatalError("Ids and Indexes do not correspond, can't use computed topological order!");
    if(ordIDs.size() != V.size())
        FatalError("Ids and V sizes differ!");
}

// 检查顶点ID与索引是否一致
// 返回: 是否一致
bool DAGTask::checkIndexAndIdsAreEqual(){
    for(size_t i=0; i<V.size();++i)
        if(V[i]->id != i)
            return false;
    return true;
}

// 计算DAG的总工作量(volume)
void DAGTask::computeVolume(){
    vol = 0;
    for(size_t i=0; i<V.size();++i)
        vol += V[i]->c;  // 累加所有顶点的WCET
}

// 计算最坏情况工作量(WCW)
void DAGTask::computeWorstCaseWorkload(){
    // 基于Melani等人的算法，计算DAG和条件DAG的工作量
    if(!ordIDs.size())
        topologicalSort();

    std::vector<std::set<int>> paths (V.size());
    paths[ordIDs[ordIDs.size()-1]].insert(ordIDs[ordIDs.size()-1]);
    int idx;
    for(int i = ordIDs.size()-2; i >= 0; --i ){
        idx = ordIDs[i];
        paths[idx].insert(idx);

        if(V[idx]->succ.size()){
            if(V[idx]->mode != C_SOURCE_T){
                // 普通节点: 合并所有后继路径
                for(int j=0; j<V[idx]->succ.size(); ++j)
                    paths[idx].insert(paths[V[idx]->succ[j]->id].begin(), paths[V[idx]->succ[j]->id].end());
            }
            else{
                // 条件源节点: 选择最大工作量的路径
                std::vector<int> sum (V[idx]->succ.size(), 0);
                int max_id = 0;
                float max_sum = 0;
                for(int j=0; j<V[idx]->succ.size(); ++j){
                    for(auto k: paths[V[idx]->succ[j]->id])
                        sum[j] += V[k]->c;
                    if(sum[j] > max_sum){
                        max_sum = sum[j];
                        max_id = j;
                    }
                }
                paths[idx].insert(paths[V[idx]->succ[max_id]->id].begin(), paths[V[idx]->succ[max_id]->id].end());
            }
        }
    }

    // 计算总工作量
    wcw = 0;
    for(auto i:paths[ordIDs[0]]){
        wcw += V[i]->c;
    }
}

// 计算类型化工作量(按处理器类型分组)
void DAGTask::computeTypedVolume(){
    for(size_t i=0; i<V.size();++i){
        if ( typedVol.find(V[i]->gamma) == typedVol.end() ) 
            typedVol[V[i]->gamma] = V[i]->c;
        else
            typedVol[V[i]->gamma] += V[i]->c;
    }
}

// 计算分区工作量(按处理器核心分组)
void DAGTask::computepVolume(){
    pVol.clear();
    for(size_t i=0; i<V.size();++i){
        if ( pVol.find(V[i]->core) == pVol.end() ) 
            pVol[V[i]->core] = V[i]->c;
        else
            pVol[V[i]->core] += V[i]->c;
    }
}

// 计算累计工作量
void DAGTask::computeAccWorkload(){
    int max_acc_prec;
    for(size_t i=0; i<ordIDs.size();++i){
        max_acc_prec = 0;
        // 找出最大前驱累计工作量
        for(size_t j=0; j<V[ordIDs[i]]->pred.size();++j){
            if(V[ordIDs[i]]->pred[j]->accWork > max_acc_prec)
                max_acc_prec = V[ordIDs[i]]->pred[j]->accWork;
        }

        // 当前顶点累计工作量 = 自身WCET + 最大前驱累计工作量
        V[ordIDs[i]]->accWork = V[ordIDs[i]]->c + max_acc_prec;
    }
}

// 计算DAG的关键路径长度
void DAGTask::computeLength(){
    if(!ordIDs.size())
        topologicalSort();
    L = 0;    
    computeAccWorkload();
    // 找出最大累计工作量即为关键路径长度
    for(const auto&v :V)
        if(v->accWork > L)
            L = v->accWork;
}

// 计算DAG的利用率
void DAGTask::computeUtilization(){
    if(vol == 0)
        computeWorstCaseWorkload();
    u =  wcw /  t;  // 利用率 = 最坏情况工作量 / 周期
}

// 计算DAG的密度
void DAGTask::computeDensity(){
    if(L == 0)
        computeLength();
    delta =  L /  d;  // 密度 = 关键路径长度 / 截止时间
}

// 计算所有顶点的本地偏移量(最早开始时间)
void DAGTask::computeLocalOffsets(){
    if(!ordIDs.size())
        topologicalSort();

    for(int i=0; i<ordIDs.size();++i){
        V[ordIDs[i]]->localOffset();
    }
}

// 计算所有顶点的本地截止时间(最晚完成时间)
void DAGTask::computeLocalDeadlines(){
    if(!ordIDs.size())
        topologicalSort();

    for(int i=ordIDs.size()-1; i>=0;--i){
        V[ordIDs[i]]->localDeadline(d);
    }
}

// 计算所有顶点的最早完成时间
void DAGTask::computeEFTs(){
    DAGTask::computeLocalOffsets();

    for(auto&v:V)
        v->EasliestFinishingTime();
}

// 计算所有顶点的最晚开始时间
void DAGTask::computeLSTs(){
    DAGTask::computeLocalDeadlines();

    for(auto&v:V)
        v->LatestStartingTime();
}

// 递归计算从指定路径开始的所有路径
// path: 当前路径
// all_paths: 所有路径集合
// 返回: 更新后的所有路径集合
std::vector<std::vector<int>> DAGTask::computeAllPathsSingleSource(std::vector<int>& path, std::vector<std::vector<int>>& all_paths) const{
    int last_node = path.back();
    if(V[last_node]->succ.size() > 0) {
        // 递归处理每个后继
        for(int i=0; i<V[last_node]->succ.size(); ++i){
            std::vector<int> new_path = path;
            new_path.push_back(V[last_node]->succ[i]->id);
            all_paths = computeAllPathsSingleSource(new_path, all_paths);
        }
    }
    else
        all_paths.push_back(path);  // 无后继则保存当前路径

    return all_paths;
}

// 计算DAG中的所有路径
// 返回: 所有路径集合
std::vector<std::vector<int>> DAGTask::computeAllPaths() const{
    std::vector<std::vector<int>> all_paths;
    // 从每个无前驱的顶点开始
    for(int i=0; i<V.size();++i){
        if(V[i]->pred.size() == 0){
            std::vector<int> cur_path;
            cur_path.push_back(i);
            all_paths = computeAllPathsSingleSource(cur_path, all_paths);
        }
    }
    return all_paths;
}

// DAG比较函数: 按截止时间升序
bool compareDAGsDeadlineInc(const DAGTask& a, const DAGTask& b){
    return a.getDeadline() < b.getDeadline();
}

// DAG比较函数: 按截止时间降序
bool compareDAGsDeadlineDec(const DAGTask& a, const DAGTask& b){
    return a.getDeadline() > b.getDeadline();
}

// DAG比较函数: 按周期升序
bool compareDAGsPeriodInc(const DAGTask& a, const DAGTask& b){
    return a.getPeriod() < b.getPeriod();
}

// DAG比较函数: 按周期降序
bool compareDAGsPeriodDec(const DAGTask& a, const DAGTask& b){
    return a.getPeriod() > b.getPeriod();
}

// DAG比较函数: 按利用率升序
bool compareDAGsUtilInc(const DAGTask& a, const DAGTask& b){
    return a.getUtilization() < b.getUtilization();
}

// DAG比较函数: 按利用率降序
bool compareDAGsUtilDec(const DAGTask& a, const DAGTask& b){
    return a.getUtilization() > b.getUtilization();
}

}