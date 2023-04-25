# Spectre 攻击验证

> 刘铠铭 2020030014 计14

## 实验原理

为了改善指令流水线的流程，CPU 会执行分支预测的优化技术。然而由于在预测失败时 CPU 不会恢复缓存状态，因此我们可以利用分支预测技术读取敏感信息，并且通过缓存侧信道泄露出来。

## 实验流程

### 准备阶段

1. 确定边界外的 x 值，即 malicious_x，使得 spy[x] 可以将所需 byte 读入缓存中；
2. 使 spy_size 和 cache_set 均不在 cache 中，使 x 在 cache 中；
3. 通过执行 5 次分支预测成功的情况（即 x < 16），使分支预测器总是会预测为 taken。

### 攻击阶段

1. 设置 x = malicious_x；
2. 执行 victim_function(x)，该过程中分支预测器会读取 secret byte 加入预先设定的缓存驱逐集 cache_set 中。

### Flush & Reload 阶段

1. 遍历访问 0 - 255 个字节，通过计算读取时间来判断该字节的内容是否在 cache 中，若在则令 result[i]++；
2. 重复上述内容 1000 次，选择最大的 result 值，其即为 secret byte。

## 实验结果

![](./result.png)

待窃取信息为预先插入的 'You should not be able to read this.'，攻击程序可以通过 Spectre Attack 得到秘密信息，并输出每个字符的正确率。

## 参考资料

- 《网络空间安全原理与实践》
-  [Spectre Attack Example](https://github.com/Eugnis/spectre-attack)
-  [Meltdown and Spectre](https://meltdownattack.com/)