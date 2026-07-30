# urdfreader - 从 URDF（统一机器人描述格式）文件加载模型

版权所有 (c) 2020-2021 Felix Richter <judge@felixrichter.tech>

## RBDL - URDF 读取插件

*本插件相比原版 rbdl 已经过大量更新！*

此插件用于将 URDF 模型导入 RBDL。它使用 [URDF_Parser 库](https://github.com/orb-hd/URDF_Parser) 来解析模型。

也可以使用 ROS 自带的 urdfdom。为此，只需在编译过程中启用 ```RBDL_USE_ROS_URDF_LIBRARY``` cmake 选项即可！

## 更新日志

* **04.12.20** - 用 URDF_Parser 库替换了 urdfdom，该库支持错误处理，且在加载模型时不会导致崩溃
* **18.11.19** - 添加对 ROS 自带 urdfdom 的支持
* **更早版本** - 由原始 rbdl 随附的版本
