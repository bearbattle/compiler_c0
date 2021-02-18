# BUAA Compiler-C0

北京航空航天大学 编译原理课程设计

BUAA Compile Technology

by 1806 Bear

## 总体情况

本人在2020年秋季学期选修了这门课程，成绩为79分（其中实验部分81分）。

Repo 中是本人在实验部分使用的代码，在未来*可能*会有维护，请合理使用 Issue, Wiki(维护中) 等功能。

编译器的目标代码是**MIPS**，整体使用了C++实现。使用的是最简单暴力的**无寄存器分配**的**直接读写内存**的策略，实现起来非常简单。

## 使用说明

在使用时，请注意切换到相应的代码分支。

切换代码分支的按钮长这样，位于绿色的下载按钮同一行的最右边：

<div style="display: inline-block;padding: 5px 16px;font-size: 14px;vertical-align: middle; border: 1px solid;
    border-radius: 6px;"><svg class="octicon octicon-git-branch text-gray" height="16" viewBox="0 0 16 16" version="1.1" width="16" aria-hidden="true"><path fill-rule="evenodd" d="M11.75 2.5a.75.75 0 100 1.5.75.75 0 000-1.5zm-2.25.75a2.25 2.25 0 113 2.122V6A2.5 2.5 0 0110 8.5H6a1 1 0 00-1 1v1.128a2.251 2.251 0 11-1.5 0V5.372a2.25 2.25 0 111.5 0v1.836A2.492 2.492 0 016 7h4a1 1 0 001-1v-.628A2.25 2.25 0 019.5 3.25zM4.25 12a.75.75 0 100 1.5.75.75 0 000-1.5zM3.5 3.25a.75.75 0 111.5 0 .75.75 0 01-1.5 0z"></path></svg>main</div>

每次作业我都建立了相关的分支，并附有文档，可以作为参考。

| 分支名 | 主要内容            | 备注                                   |
| ------ | ------------------- | -------------------------------------- |
| hw1    | 测试用例            | 之后可能会对测试用例更新补充           |
| hw2    | 词法分析            |                                        |
| hw3    | 语法分析I           | 封装了词法分析器，实现了基本的符号表   |
| hw4    | 错误处理&语法分析II | 完善了符号表                           |
| hw5    | 代码生成I           | 实现常、变量说明；读、写、赋值语句     |
| hw6    | 代码生成II          | 覆盖所有语法成分；也包含了竞速排序I&II |

## 鸣谢

感谢 [@t123yh](https://github.com/t123yh), [@dglr](https://github.com/dglr), [@xiaohan209](https://github.com/xiaohan209), [@AiHaibara1201](https://github.com/AiHaibara1201) 以及其他一众在整个课程期间帮助过我的同学。

推荐关注：

Telegram: [北航 18 计算机学院资料共享](https://t.me/buaacs2018) 里面是一些由大佬分享的学习资料。