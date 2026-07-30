# RBDL - 刚体动力学库（Rigid Body Dynamics Library）

版权所有 (c) 2011-2020 Martin Felis <martin@fysx.org>

## 简介

RBDL 是一个高效的 C++ 库，包含一些核心的刚体动力学算法，例如用于正动力学的铰接体算法（ABA，Articulated Body Algorithm）、用于逆动力学的递归牛顿-欧拉算法（RNEA，Recursive Newton-Euler Algorithm），以及用于高效计算关节空间惯性矩阵的组合刚体算法（CRBA，Composite Rigid Body Algorithm）。此外，它还包含雅可比矩阵、正逆运动学、外部约束（如接触与碰撞）处理以及闭环模型的相关代码。

该代码最初由 Martin Felis <martin@fysx.org> 在海德堡大学（Heidelberg University）的[跨学科科学计算中心（IWR）](http://www.iwr.uni-heidelberg.de)以及[计算机工程研究所](https://www.ziti.uni-heidelberg.de/ziti/en/)下属的[机器人与生物力学优化研究组（ORB）](http://orb.iwr.uni-heidelberg.de)开发。代码严格遵循 Roy Featherstone 所著《Rigid Body Dynamics Algorithm》一书中的符号约定。

本仓库包含在 ORB 研究组成员维护下的 RBDL 版本。

## 文档

文档内容包含在代码中，可使用 [doxygen](http://www.doxygen.org) 工具提取。

要生成文档，只需运行：

```
    doxygen Doxyfile
```

这将在子目录 `./doc/html` 中生成文档，主页位于 `./doc/html/index.html`。

## 获取 RBDL

官方 rbdl-orb git 仓库可通过以下地址克隆：

```
    https://github.com/ORB-HD/rbdl-orb
```

（git 客户端见 [https://git-scm.com/downloads/guis/](https://git-scm.com/downloads/guis/)。）

为确保正确下载所有子模块，请递归克隆仓库！

```
git clone --recurive https://github.com/ORB-HD/rbdl-orb
```

## 从旧版 RBDL 升级

为方便起见，提供了一个脚本用于将仓库升级到最新版本的 RBDL。

```
./upgrade.sh
```

该脚本会拉取 master 分支的最新提交，并检出所有子仓库的正确版本。手动升级需要执行以下操作：

```
git pull origin master
git submodule update --init
```

## 构建与安装

### Linux：RBDL

1. 安装前先更新 apt 系统。打开终端并输入：

```
  sudo apt update
  sudo apt upgrade
```

2. 安装 git：

```
  sudo apt install -y git-core
```

3. 安装 cmake：

```
  sudo apt install -y cmake
```

4. 安装 Eigen3

RBDL 使用 Eigen3 进行高效计算（[http://eigen.tuxfamily.org](http://eigen.tuxfamily.org)）。

```
  sudo apt install -y libeigen3-dev
```

5. 安装 C++ 编译器

编译器的选择对性能影响很大。建议评估几种不同的编译器（如 Clang）以获得最佳性能。

```
  sudo apt install -y build-essential
```

6. 安装 cmake-curses（*可选*）

如果你打算使用众多插件和其他构建选项，建议使用 cmake-curses，它能让构建配置过程更快且更不易出错。

```
  sudo apt install -y cmake-curses-gui
```

7. 安装 Catch2（*可选*）

如果想运行 RBDL 的测试代码，请安装 Catch2。目前大多数 Linux 发行版的软件仓库中还没有 catch2，因此推荐从源码构建。

```
  $ git clone --branch v2.x https://github.com/catchorg/Catch2.git
  $ cd Catch2
  $ cmake -Bbuild -H. -DBUILD_TESTING=OFF
  $ sudo cmake --build build/ --target install 
```

8. 使用 CMake 构建 RBDL（[http://www.cmake.org](http://www.cmake.org)）。要在单独的目录中以 Release 模式编译该库，请使用：

```
  mkdir /rbdl-build
  cd rbdl-build/
  cmake -D CMAKE_BUILD_TYPE=Release ../rbdl
  make
```

如果你安装了 cmake-curses-gui，可以通过运行 cmake-curses 查看所有可用的构建选项：

```
  mkdir /rbdl-build
  cd rbdl-build/
  ccmake ../rbdl 
```

此时你会看到 RBDL 完整的构建选项列表。我们建议至少构建并运行一次 RBDL 的测试代码，构建时使用：

```
  RBDL_BUILD_TESTS                 ON
  RUN_AUTOMATIC_TESTS              ON
```
ls /usr/local/include/rbdl/rbdl.h
ls /usr/local/lib/librbdl* /usr/local/lib/librbdl_urdfreader*

### Linux：RBDL 文档

1. 安装 doxygen：

```  
    sudo apt install -y doxygen
```

2. 构建 doxygen：

  - 在 RBDL 源码目录打开终端并输入：

  ```
  doxygen Doxyfile
  ```

3. 在浏览器中打开文件 `doc/html/index.html`。

### Linux：RBDL 示例

1. 安装 Boost（*可选*）

运行 RBDL 自带的许多示例仿真需要 Boost。

```
  sudo apt install -y libboost-all-dev
```    

### Linux：RBDL 插件依赖

1. luamodel 插件：

  - 如果你想将用 Lua 编写的模型文件加载到 RBDL。没有该插件时，你需要以编程方式构建模型，或使用 URDF 插件读取模型。为此：

  - 安装 Lua51：

```
    sudo apt install -y lua5.1
    sudo apt install -y liblua5.1-0-dev
```

  - 使用以下选项构建 RBDL：

```
    RBDL_BUILD_ADDON_LUAMODEL        ON
```

2. urdf 插件

  - 如果你想将用 URDF 编写的模型文件加载到 RBDL。该插件使用作为子模块包含的 URDF_Parser 库。你必须已递归克隆了该仓库！如果遗漏了这一步，之后可以在源码目录的终端中初始化子模块：

```
  git submodule init
  git submodule update
```

  - 使用以下选项构建 RBDL：

```
  RBDL_BUILD_ADDON_URDFREADER        ON
```

3. muscle 插件

  - 如果你想在 RBDL 中加入肌肉模型（例如 Millard 等人提出的模型），请使用以下选项构建 RBDL：

```
    RBDL_BUILD_ADDON_GEOMETRY ON
    RBDL_BUILD_ADDON_MUSCLE   ON
```

  - geometry 插件是依赖项，cmake 会自动包含它。

  - 参考文献：Millard M, Emonds AL, Harant M, Mombaur K. A reduced muscle model and planar musculoskeletal model fit for the simulation of whole-body movements. Journal of biomechanics. 2019 Apr 10.

4. muscle 插件：肌肉拟合选项

  - 如果你想使用 Millard 等人详述的肌肉拟合算法。

  - 安装 Ipopt。较简便的方式之一是按照 [Ipopt 在线文档](https://www.coin-or.org/Ipopt/documentation/node12.html#SECTION00042300000000000000) 中的说明进行操作（Ipopt 文件夹中的 README 也包含构建代码说明）。

  - 在 RBDL 的 cmake 配置中将以下标志设为 "On"：

```
          RBDL_BUILD_ADDON_GEOMETRY        ON                                           
          RBDL_BUILD_ADDON_LUAMODEL        ON                                           
          RBDL_BUILD_ADDON_MUSCLE          ON                                          
          RBDL_BUILD_ADDON_MUSCLE_FITTING  ON  
```

  - 将 `CUSTOM_IPOPT_PATH` 设置为 Ipopt 的主目录。

  - 构建 RBDL。

  - 更新你的 `.bashrc` 文件，将 Ipopt 的 lib 文件夹加入 `LD_LIBRARY_PATH`：

```
        export IPOPT_HOME=/home/mjhmilla/dev/Ipopt-3.12.8
        export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$IPOPT_HOME/lib
```

  - 截至 2019 年 3 月，所有肌肉拟合代码均已使用 Ipopt-3.12.8 测试通过。

  - 参考文献：Millard M, Emonds AL, Harant M, Mombaur K. A reduced muscle model and planar musculoskeletal model fit for the simulation of whole-body movements. Journal of biomechanics. 2019 Apr 10.

### Windows

虽然 RBDL 可以安装在 Windows 上，但目前没有 ORB 成员使用 Windows，因此我们无法提供详细说明。

## Python 绑定

RBDL 还可以构建一个实验性的 Python 封装器，支持 Python 3 和 Python 2。为此需启用 `RBDL_BUILD_PYTHON_WRAPPER` cmake 选项，这将为 Python 3 构建封装器；如果你想改用 Python 2，还需同时启用 `RBDL_USE_PYTHON_2` cmake 选项。结果会在构建目录中生成一个额外的 python 目录，可在其中使用 `setup.py` 进行安装。使用 `make install` 时会自动完成这一步。

### Linux：Python 封装器依赖

1. 安装 Python3、NumPy、SciPy 和 Matplotlib（*可选*）

RBDL 的大部分功能都可以通过 Python 访问。如果你有兴趣通过 Python 使用 RBDL，请参考以下说明：

  - 如果你使用的是 Ubuntu 18.04 或更高版本，Python3 已预装。

  - 要检查是否安装了 python3，在命令终端中输入：

```
  python3 -V
```

  - 如果系统中已全局安装 python3，可以用以下命令安装其余库：

```
  sudo apt install -y cython3 python3-numpy python3-scipy python3-matplotlib
```

  - 如果你使用的不是 Ubuntu 18.04，且当前没有 python3，请在网上查找在你的系统上安装这些库的说明。

2. 使用以下选项构建并安装 RBDL：

```
  RBDL_BUILD_PYTHON_WRAPPER : ON
```

（注意：你可能需要 sudo 权限才能将 rbdl.egg_info 文件安装到 `usr/local/lib/python` 目录。）

3. 将 RBDL 加入 Python 路径

更新你的 `.bashrc` 文件，使 Python 能够找到 RBDL 的 Python 版本。为此，需要将 `rbdl-build/python` 的路径添加到 `PYTHONPATH` 中，可在 `.bashrc` 文件中加入以下一行：

```
  export PYTHONPATH=$PYTHONPATH:<path-to-the-RBDL-build-directory>/python
``` 

## 了解更多资源

了解 RBDL 中任何内容的途径主要有以下四种：

1. examples 文件夹

  - 包含一组配有详细文档的深入示例：如果你是 RBDL 新手，建议从这里开始。

  - 还有一组极简示例。

  - 这些示例对基础内容覆盖得相当好，但当前许多高级项目（四元数关节、自定义关节、自定义约束、肌肉拟合）尚无示例。

2. Doxygen 文档

  - 近期开发的方法和组件的 Doxygen 文档非常详细（例如 muscle 插件中的 Millard2016TorqueMuscle 类）。

  - 较成熟方法的 Doxygen 文档则相对简略。

3. 测试代码

  - 每个命令和建模组件的极简示例都可以在测试代码（如 `rbdl/tests`、`addons/geometry/tests`、`addons/muscle/tests` 等）中找到。

  - 可以使用能够搜索整个目录（如 Sublime Text）的文本编辑器，按关键字轻松找到特定命令。

4. 文献资料

  - 除 Featherstone 的著作和 Felis 的论文外，RBDL 中还包含许多令人振奋的方法和建模工具。

  - 相关文献引用在对应方法的 Doxygen 中均有说明。

## 引用

关于理论与实现细节的综述已发表于 [https://doi.org/10.1007/s10514-016-9574-0](Felis, M.L. Auton Robot (2017) 41: 495)。在你的学术研究中引用 RBDL 时，可使用以下 BibTeX 条目：

    @Article{Felis2016,
      author="Felis, Martin L.",
      title="RBDL: an efficient rigid-body dynamics library using recursive algorithms",
      journal="Autonomous Robots",
      year="2016",
      pages="1--17",
      issn="1573-7527",
      doi="10.1007/s10514-016-9574-0",
      url="http://dx.doi.org/10.1007/s10514-016-9574-0"
    }

## 许可证

本库以非常宽松的 zlib 自由软件许可证发布，应允许你在任何需要的地方使用该软件。

以下是完整的许可证文本（zlib 许可证）：

    RBDL - Rigid Body Dynamics Library
    Copyright (c) 2011-2020 Martin Felis <martin@fysx.org>
    
    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.
    
    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:
    
       1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software
       in a product, an acknowledgment in the product documentation would be
       appreciated but is not required.
    
       2. Altered source versions must be plainly marked as such, and must not
       be misrepresented as being the original software.
    
       3. This notice may not be removed or altered from any source
       distribution.

## 致谢

本库的工作最初由[海德堡数学与计算科学研究生院（HGS）](http://hgs.iwr.uni-heidelberg.de/hgs.mathcomp/)，以及欧洲 FP7 项目 [ECHORD](http://echord.eu)（资助号 231143）和 [Koroibot](http://koroibot.eu)（资助号 611909）资助。

geometry 和 muscle 插件的工作由 Matthew Millard 完成。衷心感谢德国科学基金会（Deutsche Forschungs Gemeinschaft）资助号 MI 2109/1-1，以及欧盟 H2020 项目 Spexor（GA 687662）提供的财政支持。
