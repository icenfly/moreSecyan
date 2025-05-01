import matplotlib.pyplot as plt
import numpy as np
import os

# 数据大小（MB）
data_sizes = [1, 3, 10, 33, 100]

# 从实验结果中提取的运行时间（ms）
# 基本功能
basic_times = [714, 2675, 7741, 23859, 75275]

# 结果保护（Result Protection）
rp_times = [678, 1932, 6719, 22340, 71894]

# 双重执行（Dual Execution）
de_times = [1483, 4364, 14002, 47002, 148867]

# 设置中文字体
plt.rcParams['font.sans-serif'] = ['Arial Unicode MS', 'SimHei']  # 用于显示中文
plt.rcParams['axes.unicode_minus'] = False  # 用于正常显示负号
plt.rcParams['font.size'] = 14  # 设置全局字体大小

# 创建两个版本的图：普通刻度和对数刻度

# 图1：基本功能在不同数据大小上的运行时间 - 普通刻度
plt.figure(figsize=(10, 6))
plt.plot(data_sizes, basic_times, 'o-', linewidth=2, markersize=10, label='基本功能')
plt.xlabel('数据大小 (MB)', fontsize=16)
plt.ylabel('运行时间 (ms)', fontsize=16)
plt.grid(True, linestyle='--', alpha=0.7)
plt.xticks(data_sizes, fontsize=14)
plt.yticks(fontsize=14)
plt.legend(fontsize=16)
# 原标题: plt.title('基本功能运行时间', fontsize=18)
plt.tight_layout()
plt.savefig('experiment1_basic_function.png', dpi=300)
plt.close()

# 图1：基本功能在不同数据大小上的运行时间 - 对数刻度
plt.figure(figsize=(10, 6))
plt.loglog(data_sizes, basic_times, 'o-', linewidth=2, markersize=10, label='基本功能')
plt.xlabel('数据大小 (MB)', fontsize=16)
plt.ylabel('运行时间 (ms)', fontsize=16)
plt.grid(True, linestyle='--', alpha=0.7)
plt.xticks(data_sizes, fontsize=14)
plt.yticks(fontsize=14)
plt.legend(fontsize=16)
# 原标题: plt.title('基本功能运行时间 (对数刻度)', fontsize=18)
plt.tight_layout()
plt.savefig('experiment1_basic_function_log.png', dpi=300)
plt.close()

# 图2：基本功能与结果保护功能对比 - 普通刻度
plt.figure(figsize=(10, 6))
plt.plot(data_sizes, basic_times, 'o-', linewidth=2, markersize=10, label='基本功能')
plt.plot(data_sizes, rp_times, 's-', linewidth=2, markersize=10, label='结果保护功能')
plt.xlabel('数据大小 (MB)', fontsize=16)
plt.ylabel('运行时间 (ms)', fontsize=16)
plt.grid(True, linestyle='--', alpha=0.7)
plt.xticks(data_sizes, fontsize=14)
plt.yticks(fontsize=14)
plt.legend(fontsize=16)
# 原标题: plt.title('基本功能与结果保护功能对比', fontsize=18)
plt.tight_layout()
plt.savefig('experiment2_result_protection.png', dpi=300)
plt.close()

# 图2：基本功能与结果保护功能对比 - 对数刻度
plt.figure(figsize=(10, 6))
plt.loglog(data_sizes, basic_times, 'o-', linewidth=2, markersize=10, label='基本功能')
plt.loglog(data_sizes, rp_times, 's-', linewidth=2, markersize=10, label='结果保护功能')
plt.xlabel('数据大小 (MB)', fontsize=16)
plt.ylabel('运行时间 (ms)', fontsize=16)
plt.grid(True, linestyle='--', alpha=0.7)
plt.xticks(data_sizes, fontsize=14)
plt.yticks(fontsize=14)
plt.legend(fontsize=16)
# 原标题: plt.title('基本功能与结果保护功能对比 (对数刻度)', fontsize=18)
plt.tight_layout()
plt.savefig('experiment2_result_protection_log.png', dpi=300)
plt.close()

# 图3：基本功能与双重执行功能对比 - 普通刻度
plt.figure(figsize=(10, 6))
plt.plot(data_sizes, basic_times, 'o-', linewidth=2, markersize=10, label='基本功能')
plt.plot(data_sizes, de_times, '^-', linewidth=2, markersize=10, label='双重执行功能')
plt.xlabel('数据大小 (MB)', fontsize=16)
plt.ylabel('运行时间 (ms)', fontsize=16)
plt.grid(True, linestyle='--', alpha=0.7)
plt.xticks(data_sizes, fontsize=14)
plt.yticks(fontsize=14)
plt.legend(fontsize=16)
# 原标题: plt.title('基本功能与双重执行功能对比', fontsize=18)
plt.tight_layout()
plt.savefig('experiment3_dual_execution.png', dpi=300)
plt.close()

# 图3：基本功能与双重执行功能对比 - 对数刻度
plt.figure(figsize=(10, 6))
plt.loglog(data_sizes, basic_times, 'o-', linewidth=2, markersize=10, label='基本功能')
plt.loglog(data_sizes, de_times, '^-', linewidth=2, markersize=10, label='双重执行功能')
plt.xlabel('数据大小 (MB)', fontsize=16)
plt.ylabel('运行时间 (ms)', fontsize=16)
plt.grid(True, linestyle='--', alpha=0.7)
plt.xticks(data_sizes, fontsize=14)
plt.yticks(fontsize=14)
plt.legend(fontsize=16)
# 原标题: plt.title('基本功能与双重执行功能对比 (对数刻度)', fontsize=18)
plt.tight_layout()
plt.savefig('experiment3_dual_execution_log.png', dpi=300)
plt.close()

# 额外创建一个综合对比图 - 普通刻度
plt.figure(figsize=(12, 7))
plt.plot(data_sizes, basic_times, 'o-', linewidth=2, markersize=10, label='基本功能')
plt.plot(data_sizes, rp_times, 's-', linewidth=2, markersize=10, label='结果保护功能')
plt.plot(data_sizes, de_times, '^-', linewidth=2, markersize=10, label='双重执行功能')
plt.xlabel('数据大小 (MB)', fontsize=16)
plt.ylabel('运行时间 (ms)', fontsize=16)
plt.grid(True, linestyle='--', alpha=0.7)
plt.xticks(data_sizes, fontsize=14)
plt.yticks(fontsize=14)
plt.legend(fontsize=16)
# 原标题: plt.title('三种功能运行时间对比', fontsize=18)
plt.tight_layout()
plt.savefig('experiment_all_comparison.png', dpi=300)
plt.close()

# 额外创建一个综合对比图 - 对数刻度
plt.figure(figsize=(12, 7))
plt.loglog(data_sizes, basic_times, 'o-', linewidth=2, markersize=10, label='基本功能')
plt.loglog(data_sizes, rp_times, 's-', linewidth=2, markersize=10, label='结果保护功能')
plt.loglog(data_sizes, de_times, '^-', linewidth=2, markersize=10, label='双重执行功能')
plt.xlabel('数据大小 (MB)', fontsize=16)
plt.ylabel('运行时间 (ms)', fontsize=16)
plt.grid(True, linestyle='--', alpha=0.7)
plt.xticks(data_sizes, fontsize=14)
plt.yticks(fontsize=14)
plt.legend(fontsize=16)
# 原标题: plt.title('三种功能运行时间对比 (对数刻度)', fontsize=18)
plt.tight_layout()
plt.savefig('experiment_all_comparison_log.png', dpi=300)
plt.close()

print("已生成改进的折线图：")
print("1. experiment1_basic_function.png - 基本功能在不同数据大小上的运行时间")
print("2. experiment1_basic_function_log.png - 基本功能在不同数据大小上的运行时间 (对数刻度)")
print("3. experiment2_result_protection.png - 基本功能与基于差分隐私的数据保护功能对比")
print("4. experiment2_result_protection_log.png - 基本功能与基于差分隐私的数据保护功能对比 (对数刻度)")
print("5. experiment3_dual_execution.png - 基本功能与基于双向执行的安全性增强功能对比")
print("6. experiment3_dual_execution_log.png - 基本功能与基于双向执行的安全性增强功能对比 (对数刻度)")
print("7. experiment_all_comparison.png - 三种功能在不同数据大小上的运行时间对比")
print("8. experiment_all_comparison_log.png - 三种功能在不同数据大小上的运行时间对比 (对数刻度)") 