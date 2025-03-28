# 实验结果分析

## 实验概述

本实验针对不同大小的数据集（1MB、3MB、10MB、33MB、100MB）进行了三种不同功能的性能测试：
1. 基本功能
2. 基于差分隐私的数据保护功能（结果保护）
3. 基于双向执行的安全性增强功能（双重执行）

## 实验一：基本功能在不同数据大小上的运行时间

### 数据结果
| 数据大小 (MB) | 运行时间 (ms) |
|--------------|--------------|
| 1            | 714          |
| 3            | 2,675        |
| 10           | 7,741        |
| 33           | 23,859       |
| 100          | 75,275       |

### 分析
- 随着数据大小的增加，运行时间呈现近似线性增长趋势
- 从1MB到100MB，运行时间增加了约105倍，而数据大小增加了100倍，表明系统的扩展性良好
- 对数图表显示，运行时间与数据大小之间存在较为稳定的关系，符合预期的计算复杂度

## 实验二：基本功能与基于差分隐私的数据保护功能对比

### 数据结果
| 数据大小 (MB) | 基本功能 (ms) | 结果保护功能 (ms) | 性能差异 (%) |
|--------------|--------------|-----------------|-------------|
| 1            | 714          | 678             | -5.0%       |
| 3            | 2,675        | 1,932           | -27.8%      |
| 10           | 7,741        | 6,719           | -13.2%      |
| 33           | 23,859       | 22,340          | -6.4%       |
| 100          | 75,275       | 71,894          | -4.5%       |

### 分析
- 有趣的是，结果保护功能在所有数据大小上的运行时间都略低于基本功能
- 这可能是因为结果保护功能在处理过程中对数据进行了某种优化或简化
- 差异在3MB数据集上最为明显（约27.8%的性能提升）
- 在较大的数据集（33MB和100MB）上，两种功能的性能差异较小（约4.5%-6.4%）
- 总体而言，添加差分隐私的数据保护功能不会带来性能损失，反而可能略有提升

## 实验三：基本功能与基于双向执行的安全性增强功能对比

### 数据结果
| 数据大小 (MB) | 基本功能 (ms) | 双重执行功能 (ms) | 性能开销 (%) |
|--------------|--------------|-----------------|-------------|
| 1            | 714          | 1,483           | +107.7%     |
| 3            | 2,675        | 4,364           | +63.1%      |
| 10           | 7,741        | 14,002          | +80.9%      |
| 33           | 23,859       | 47,002          | +97.0%      |
| 100          | 75,275       | 148,867         | +97.8%      |

### 分析
- 双重执行功能的运行时间明显高于基本功能，这符合预期，因为它需要执行两次计算
- 性能开销大约在63%-108%之间，平均约为89%
- 理论上，双重执行应该使运行时间增加一倍（100%），实际结果与理论预期相符
- 在较小的数据集（1MB）上，相对开销更大（107.7%），可能是因为双重执行的固定开销在小数据集上占比更高
- 在较大的数据集（33MB和100MB）上，性能开销稳定在约97%-98%左右

## 综合分析

1. **扩展性**：三种功能都表现出良好的扩展性，运行时间随数据大小增长而增长，且增长趋势相对稳定。

2. **安全性与性能权衡**：
   - 结果保护功能（差分隐私）不仅没有带来性能损失，反而可能略有性能提升，这是一个意外的积极发现。
   - 双重执行功能带来了约90%的性能开销，这是为了提高安全性而付出的合理代价。

3. **选择建议**：
   - 如果应用场景对数据隐私保护有要求，可以无顾虑地启用结果保护功能，因为它不会带来性能损失。
   - 如果应用场景对安全性有更高要求，可以考虑启用双重执行功能，但需要接受约90%的性能开销。
   - 在资源受限的环境中，基本功能可能是最佳选择，特别是当数据量较大时。

4. **未来优化方向**：
   - 可以进一步研究为什么结果保护功能会带来性能提升，并尝试将这种优化应用到其他功能中。
   - 可以尝试优化双重执行功能，减少其性能开销，例如通过并行计算或其他优化技术。

## 结论

本实验验证了系统在不同数据大小上的基本功能性能，并对比了添加安全性增强功能后的性能变化。结果表明，系统具有良好的扩展性，差分隐私的数据保护功能不会带来性能损失，而双向执行的安全性增强功能带来的性能开销在可接受范围内。这些发现为系统的实际部署和使用提供了有价值的参考。 