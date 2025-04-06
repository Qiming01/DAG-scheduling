# 一个用于测试实时 DAG 任务模型可调度性的库

本仓库提供了一个用于处理实时有向无环图（DAG）任务模型及其扩展（如条件 DAG 和类型化 DAG）的库。  
此外，还实现了实时领域文献中的几种方法。  
这是 Micaela Verucchi 博士论文《DAG 任务的全面分析：现代实时嵌入式系统的解决方案》的研究贡献之一。

如果您在研究中使用了此代码，请引用以下论文：

```
@article{verucchi2023survey,
  title={A survey on real-time DAG scheduling, revisiting the Global-Partitioned Infinity War},
  author={Verucchi, Micaela and Olmedo, Ignacio Sa{\~n}udo and Bertogna, Marko},
  journal={Real-Time Systems},
  pages={1--52},
  year={2023},
  publisher={Springer}
}
```

## 依赖项

此仓库需要以下库：graphviz（用于 dot）、libyaml-cpp-dev（用于读取 yaml 配置文件）和 python3-matplotlib（用于绘制图表）。  
在 Linux 上，使用以下方式安装依赖：

```
sudo apt-get install graphviz libyaml-cpp-dev python3-matplotlib libtbb-dev libeigen3-dev python3-dev
```

## 如何编译该仓库

安装完依赖后，使用 cmake 构建项目：

```
git clone https://github.com/mive93/DAG-scheduling 
cd DAG-scheduling
git submodule update --init --recursive 
cmake -S . -B build
cmake --build build
```

## 输入与输出

该库使用 DOT 格式读取和输出 DAG，如图所示：

![dot](img/dot_format.png "DAG with DOT") 

### 输入

该库支持两种任务集输入格式：yaml 或 DOT。

#### YAML 格式

一个任务集通过一个任务数组指定，每个任务具有以下参数：

 * `t`：周期  
 * `d`：截止时间  
 * `vertices`：DAG 的节点数组，定义方式如下：
    * `id`：节点编号  
    * `c`：节点的最坏情况执行时间（WCET）  
    * `p`：该节点分配到的核心（可选）  
    * `s`：gamma，分配到的引擎类型（可选）  

示例文件见：`demo/taskset.yaml`

#### DOT 格式

每个 DAG 通过一个 DOT 文件定义，所有属于任务集的 dot 文件路径应逐行写入一个 txt 文件中。  
示例文件见：`demo/dot_files.txt`

每个 DOT 文件按照如下约定定义：

开始处有一个初始节点，表示 DAG 任务的整体信息，需指定截止时间 `D` 和周期 `T`，例如：  
```i [shape=box, D=603.859, T=1605.45]; ```

随后每个节点以 ID、代表 WCET 的标签和其他可选参数（如 `p` 和 `s`）定义，例如：  
``` 0 [label="57", p=7]; ```

最后，边通过节点 ID 定义，例如：  
``` 0 -> 2; ```

完整示例见：`demo/test0.dot`

### 输出

要将 dot 文件转换为 png 图像，使用以下命令：

```
dot -Tpng test.dot > test.png
```

## Demo

Demo 是一个测试所有已支持方法的程序，可对用户自定义的 DAG 任务集或随机生成的任务集进行测试。  
执行方式如下：

```
./demo <random-flag> <dag-file>
```

其中：

  * `<random-flag>`：设为 0 表示从文件读取任务集，设为 1 表示使用 Melani 等人提出的方法 [1][2] 随机生成任务集  
  * `<dag-file>`：如果 `<random-flag>` 为 0，该参数指定要分析的任务集文件路径

## Eval

Eval 程序将基于给定配置文件评估多个可调度性测试方法。  
执行方式如下：

```
./eval <config-file> <show-plots>
```

其中：

  * `<config-file>`：一个 yml 文件，如 `data` 文件夹中的示例，用于指定任务集生成的所有参数及要评估的方法  
  * `<show-plots>`：设为 1 显示图表，设为 0 隐藏图表。无论设为多少，图表均不会直接显示，而是保存在名为 `res` 的文件夹中  

每次评估结束后将生成两类图表：一类展示可调度率，另一类展示各方法的执行时间。

![plots](img/sched_times.png "Resulting plots") 

## 支持的可调度性测试方法

| 方法来源              | 工作负载     | 截止类型 | 模型        | 调度策略 | 抢占方式 | 算法       |
|----------------------|--------------|----------|-------------|-----------|-----------|------------|
| Baruah2012 [3]        | 单个任务     | C        | DAG         | G         | -         | EDF        |
| Bonifaci2013 [4]      | 任务集       | A        | DAG         | G         | FP        | EDF, DM    |
| Li2013 [5]            | 任务集       | I        | DAG         | G         | FP        | EDF        |
| Qamhieh2013 [6]       | 任务集       | C        | DAG         | G         | FP        | EDF        |
| Baruah2014 [7]        | 任务集       | C        | DAG         | G         | FP        | EDF        |
| Melani2015 [1]        | 任务集       | C        | DAG, C-DAG  | G         | FP        | EDF, FTP   |
| Serrano2016 [8]       | 任务集       | C        | DAG         | G         | LP        | FTP        |
| Fonseca2016 [14]      | 任务集       | C        | DAG         | P         | FP        | FTP        |
| Pathan2017 [9]        | 任务集       | C        | DAG         | G         | FP        | DM         |
| Fonseca2017 [10]      | 任务集       | C        | DAG         | G         | FP        | DM         |
| Casini2018 [15]       | 任务集       | C        | DAG         | P         | LP        | FTP        |
| Han2019 [11]          | 单个任务     | C        | H-DAG       | G         | -         | -          |
| He2019 [12]           | 任务集       | C        | DAG         | G         | FP        | EDF, FTP   |
| Fonseca2019 [13]      | 任务集       | C, A      | DAG         | G         | FP        | EDF, FTP   |
| Nasri2019 [16]        | 任务集       | C        | DAG         | G         | LP        | EDF        |
| Zahaf2020 [17]        | 任务集       | C        | HC-DAG      | P         | FP, FNP   | EDF        |

* [1] Alessandra Melani 等人，“多处理器系统中条件 DAG 任务的响应时间分析”（ECRTS 2015）  
* [2] https://retis.sssup.it/~d.casini/resources/DAG_Generator/cptasks.zip  
* [3] Baruah 等人，“一种针对重复实时任务的通用并行任务模型”（RTSS 2012）  
* [4] Bonifaci 等人，“在零散 DAG 任务模型中的可行性分析”（ECRTS 2013）  
* [5] Li 等人，“杰出论文奖：并行任务的全局 EDF 分析”（ECRTS 2013）  
* [6] Qamhieh 等人，“多处理器系统中 DAG 的全局 EDF 调度”（RTNS 2013）  
* [7] Baruah 等人，“零散 DAG 任务系统的多处理器全局可调度性分析改进”（ECRTS 2014）  
* [8] Maria A Serrano 等人，“有限抢占条件下固定优先级调度的 DAG 任务响应时间分析”（DATE 2016）  
* [9] Risat Pathan 等人，“多核平台上并行实时重复任务的调度”（IEEE TPDS 2017）  
* [10] Fonseca 等人，“用于全局固定优先调度的零散 DAG 任务的改进响应时间分析”（RTNS 2017）  
* [11] Meiling Han 等人，“异构多核上类型化 DAG 并行任务的响应时间界”（IEEE TPDS 2019）  
* [12] Qingqiang He 等人，“多核上 DAG 任务实时调度中的任务内优先级分配”（IEEE TPDS 2019）  
* [13] Fonseca 等人，“在全局固定优先调度下具有任意截止时间的 DAG 任务可调度性分析”（Real-Time Systems 2019）  
* [14] Fonseca 等人，“分区调度下零散 DAG 任务的响应时间分析”（SIES 2016）  
* [15] Daniel Casini 等人，“无抢占并行任务的分区固定优先级调度”（RTSS 2018）  
* [16] Mitra Nasri 等人，“全局调度下有限抢占并行 DAG 任务的响应时间分析”（ECRTS 2019）  
* [17] Houssam-Eddine Zahaf 等人，“异构实时系统的 HPC-DAG 任务模型”（IEEE TC 2020）  