# UE Editor AI Automation System | UE编辑器AI自动化系统

[English](#english) | [中文](#中文)

---

## English

### Overview

A comprehensive Unreal Engine 5 plugin that enables AI assistants to control the editor through the Model Context Protocol (MCP).

### Quick Start

#### Step 1: Install UE Plugin

Copy the `McpAutomationBridge` folder to your UE project:

```
YourProject/Plugins/McpAutomationBridge/
```

#### Step 2: Regenerate Project Files

- **Windows**: Right-click `.uproject` → "Generate Visual Studio project files"
- **Mac**: Right-click `.uproject` → "Generate Xcode project"

#### Step 3: Build and Enable Plugin

1. Open project in Unreal Editor
2. When prompted to rebuild missing modules, click "Yes"
3. Close and reopen the editor
4. Go to **Edit → Plugins**
5. Enable:
   - MCP Automation Bridge
   - Editor Scripting Utilities
   - Niagara (optional)

#### Step 4: Install MCP Server

```bash
cd ue-mcp-server
npm install
npm run build
```

#### Step 5: Configure MCP Client

Refer to `mcp-config.example.json` for configuration examples.

**Claude Desktop Configuration**

Add to `claude_desktop_config.json`:

```json
{
  "mcpServers": {
    "unreal-engine": {
      "command": "node",
      "args": ["/absolute/path/to/ue-mcp-server/dist/index.js"],
      "env": {
        "MCP_AUTOMATION_HOST": "127.0.0.1",
        "MCP_AUTOMATION_PORT": "8091"
      }
    }
  }
}
```

**Config File Locations:**
- **macOS**: `~/Library/Application Support/Claude/claude_desktop_config.json`
- **Windows**: `%APPDATA%\Claude\claude_desktop_config.json`

#### Step 6: Start Using

1. Open your UE project
2. Restart Claude Desktop
3. The plugin will automatically start the WebSocket server on port 8091

### Architecture

```
Claude Desktop (AI)
       ↓ MCP Protocol
ue-mcp-server (TypeScript)
       ↓ WebSocket (Port 8091)
McpAutomationBridge (UE C++ Plugin)
       ↓ UE C++ API
Unreal Engine Editor
```

### Core Features

#### Blueprint System
- Node operations: create, move, delete, copy
- Connection management: data flow and execution flow
- Variable & function creation with access control
- Support: Actor, Widget, Function Library, Interface, Enum, Structure

#### Scene Editing
- Actor management: spawn, move, rotate, scale, delete
- Level management: create, switch, streaming
- Environment: terrain, vegetation, lighting

#### Material System
- Create materials and instances
- Connect material nodes
- Configure blend modes and parameters

#### UI System (UMG)
- Create widgets (Button, Text, Image, etc.)
- Layout controls (Canvas Panel, Box, Grid, etc.)
- Event binding and interactions

#### Animation System
- Create animation blueprints
- Design state machines
- Configure transitions

#### Audio System
- Import and manage audio assets
- Configure attenuation and concurrency
- Dynamic audio systems

#### Physics System
- Collision configuration
- Physical materials
- Physics simulation

#### AI & Game Logic
- Behavior trees
- Perception systems
- AI decision logic

#### Project Management
- Input mappings
- Gameplay tags
- Asset management

#### Debugging & Optimization
- Visual debugging
- Logging system
- Performance optimization

### API Reference

#### Blueprint Operations

**Get Blueprint Nodes**
```json
{
  "command": "get_blueprint_nodes",
  "params": {
    "blueprint_path": "/Game/Blueprints/BP_MyActor",
    "graph_name": "EventGraph"
  }
}
```

**Create Blueprint Node**
```json
{
  "command": "create_blueprint_node",
  "params": {
    "blueprint_path": "/Game/Blueprints/BP_MyActor",
    "node_type": "Event",
    "event_name": "ReceiveBeginPlay",
    "position": { "x": 0, "y": 0 }
  }
}
```

**Connect Blueprint Pins**
```json
{
  "command": "connect_blueprint_pins",
  "params": {
    "blueprint_path": "/Game/Blueprints/BP_MyActor",
    "source_node_id": "node-guid-1",
    "source_pin_name": "then",
    "target_node_id": "node-guid-2",
    "target_pin_name": "execute"
  }
}
```

#### Actor Operations

**Spawn Actor**
```json
{
  "command": "spawn_actor",
  "params": {
    "actor_class": "/Game/Blueprints/BP_MyActor.BP_MyActor_C",
    "location": { "x": 0, "y": 0, "z": 100 },
    "rotation": { "pitch": 0, "yaw": 0, "roll": 0 },
    "name": "MySpawnedActor"
  }
}
```

**Get Scene Actors**
```json
{
  "command": "get_scene_actors",
  "params": {
    "filter": {
      "class_name": "PointLight",
      "tag": "Interactive"
    },
    "include_hidden": false
  }
}
```

### Complete Tool List

#### Blueprint Tools
- `get_blueprint_nodes` - Get blueprint node information
- `create_blueprint_node` - Create blueprint nodes
- `move_blueprint_node` - Move blueprint nodes
- `delete_blueprint_node` - Delete blueprint nodes
- `connect_blueprint_pins` - Connect blueprint pins
- `disconnect_blueprint_pins` - Disconnect blueprint pins
- `get_node_connections` - Get node connection information
- `create_blueprint_variable` - Create blueprint variables
- `create_blueprint_function` - Create blueprint functions

#### Scene Tools
- `spawn_actor` - Spawn actors
- `move_actor` - Move actors
- `rotate_actor` - Rotate actors
- `scale_actor` - Scale actors
- `delete_actor` - Delete actors
- `get_scene_actors` - Get scene actor list

#### Asset Tools
- `find_assets` - Find assets in project
- `create_asset` - Create assets
- `rename_asset` - Rename assets
- `move_asset` - Move assets
- `delete_asset` - Delete assets
- `duplicate_asset` - Duplicate assets
- `get_asset_references` - Get asset references

#### Project Tools
- `create_game_mode` - Create game modes
- `configure_input` - Configure input mappings
- `create_game_tag` - Create gameplay tags

### Use Cases

#### Create Character Blueprint
1. Create character blueprint class (inheriting from Character)
2. Add character movement component
3. Add spring arm and camera components
4. Create movement input logic (WASD)
5. Add mouse view control
6. Reference animation blueprint
7. Configure default properties
8. Place instance in scene

#### Implement Scoring System
1. Create game mode blueprint
2. Design scoring variables and logic
3. Create UI Widget for score display
4. Implement scoring events
5. Configure win/lose conditions
6. Add sound effects
7. Implement level restart

#### Optimize Project Performance
1. Analyze project structure
2. Identify performance bottlenecks
3. Optimize material complexity
4. Merge static meshes
5. Configure LOD
6. Set culling distance
7. Organize asset directory
8. Generate optimization report

### Troubleshooting

#### Plugin Failed to Load
1. Close Unreal Editor
2. Reopen the project
3. Plugin should load correctly

#### Connection Refused
1. Verify plugin is enabled in **Edit → Plugins**
2. Check port 8091 is not blocked by firewall
3. Ensure MCP server is running

#### Build Errors
1. Close your IDE
2. Delete `Intermediate/`, `Binaries/`, `Saved/` folders
3. Regenerate project files
4. Rebuild the project

### Design Principles

#### Atomic Operations
Each MCP tool performs one specific operation:
- Allows flexible operation combination
- Reduces error probability
- Simplifies testing and maintenance
- Provides clear error messages

#### State Visualization
Complete editor state information:
- Currently selected objects
- All actors in scene
- All nodes in blueprint
- Connection relationships
- Asset references

#### Intelligent Guidance
- Operation suggestions
- Parameter auto-completion
- Context-aware recommendations
- Best practice tips

#### Safety Control
- Operation whitelist
- Sensitive operation confirmation
- Complete operation logs
- Automatic backup
- Operation rollback support

### Implementation Roadmap

#### Phase 1: Basic Capabilities
- Blueprint node operations
- Blueprint connection operations
- Variable and function management
- Basic scene operations

#### Phase 2: Advanced Capabilities
- Material system operations
- UI system operations
- Animation system operations
- Physics system configuration

#### Phase 3: Expert Capabilities
- AI system operations
- Project configuration management
- Performance optimization tools
- Advanced debugging features

### Supported Versions

- Unreal Engine 5.0 - 5.7
- Platforms: Win64, Mac, Linux

### License

MIT License

---

## 中文

### 概述

一个完整的虚幻引擎5插件，通过MCP协议使AI助手能够控制编辑器。

### 快速开始

#### 步骤一：安装UE插件

将 `McpAutomationBridge` 文件夹复制到您的UE项目：

```
YourProject/Plugins/McpAutomationBridge/
```

#### 步骤二：重新生成项目文件

- **Windows**：右键 `.uproject` → "Generate Visual Studio project files"
- **Mac**：右键 `.uproject` → "Generate Xcode project"

#### 步骤三：构建并启用插件

1. 在虚幻编辑器中打开项目
2. 提示重建缺失模块时，点击"是"
3. 关闭并重新打开编辑器
4. 前往 **编辑 → 插件**
5. 启用：
   - MCP Automation Bridge
   - Editor Scripting Utilities
   - Niagara（可选）

#### 步骤四：安装MCP服务器

```bash
cd ue-mcp-server
npm install
npm run build
```

#### 步骤五：配置MCP客户端

请参考 `mcp-config.example.json` 查看完整配置示例。

**Claude Desktop 配置**

添加到 `claude_desktop_config.json`：

```json
{
  "mcpServers": {
    "unreal-engine": {
      "command": "node",
      "args": ["/绝对路径/ue-mcp-server/dist/index.js"],
      "env": {
        "MCP_AUTOMATION_HOST": "127.0.0.1",
        "MCP_AUTOMATION_PORT": "8091"
      }
    }
  }
}
```

**配置文件位置：**
- **macOS**: `~/Library/Application Support/Claude/claude_desktop_config.json`
- **Windows**: `%APPDATA%\Claude\claude_desktop_config.json`

#### 步骤六：开始使用

1. 打开您的UE项目
2. 重启Claude Desktop
3. 插件会自动在8091端口启动WebSocket服务器

### 架构

```
Claude Desktop (AI)
       ↓ MCP协议
ue-mcp-server (TypeScript)
       ↓ WebSocket (端口 8091)
McpAutomationBridge (UE C++ 插件)
       ↓ UE C++ API
虚幻引擎编辑器
```

### 核心功能

#### 蓝图系统
- 节点操作：创建、移动、删除、复制
- 连接管理：数据流和执行流
- 变量与函数创建，支持访问控制
- 支持：Actor、Widget、函数库、接口、枚举、结构体

#### 场景编辑
- Actor管理：生成、移动、旋转、缩放、删除
- 关卡管理：创建、切换、流式加载
- 环境构建：地形、植被、光照

#### 材质系统
- 创建材质和实例
- 连接材质节点
- 配置混合模式和参数

#### UI系统（UMG）
- 创建控件（按钮、文本、图像等）
- 布局控件（画布面板、框、网格等）
- 事件绑定和交互

#### 动画系统
- 创建动画蓝图
- 设计状态机
- 配置过渡

#### 音频系统
- 导入和管理音频资产
- 配置衰减和并发
- 动态音频系统

#### 物理系统
- 碰撞配置
- 物理材质
- 物理模拟

#### AI与游戏逻辑
- 行为树
- 感知系统
- AI决策逻辑

#### 项目管理
- 输入映射
- 游戏标签
- 资产管理

#### 调试与优化
- 可视化调试
- 日志系统
- 性能优化

### API参考

#### 蓝图操作

**获取蓝图节点**
```json
{
  "command": "get_blueprint_nodes",
  "params": {
    "blueprint_path": "/Game/Blueprints/BP_MyActor",
    "graph_name": "EventGraph"
  }
}
```

**创建蓝图节点**
```json
{
  "command": "create_blueprint_node",
  "params": {
    "blueprint_path": "/Game/Blueprints/BP_MyActor",
    "node_type": "Event",
    "event_name": "ReceiveBeginPlay",
    "position": { "x": 0, "y": 0 }
  }
}
```

**连接蓝图引脚**
```json
{
  "command": "connect_blueprint_pins",
  "params": {
    "blueprint_path": "/Game/Blueprints/BP_MyActor",
    "source_node_id": "node-guid-1",
    "source_pin_name": "then",
    "target_node_id": "node-guid-2",
    "target_pin_name": "execute"
  }
}
```

#### Actor操作

**生成Actor**
```json
{
  "command": "spawn_actor",
  "params": {
    "actor_class": "/Game/Blueprints/BP_MyActor.BP_MyActor_C",
    "location": { "x": 0, "y": 0, "z": 100 },
    "rotation": { "pitch": 0, "yaw": 0, "roll": 0 },
    "name": "MySpawnedActor"
  }
}
```

**获取场景Actor**
```json
{
  "command": "get_scene_actors",
  "params": {
    "filter": {
      "class_name": "PointLight",
      "tag": "Interactive"
    },
    "include_hidden": false
  }
}
```

### 完整工具列表

#### 蓝图工具
- `get_blueprint_nodes` - 获取蓝图节点信息
- `create_blueprint_node` - 创建蓝图节点
- `move_blueprint_node` - 移动蓝图节点
- `delete_blueprint_node` - 删除蓝图节点
- `connect_blueprint_pins` - 连接蓝图引脚
- `disconnect_blueprint_pins` - 断开蓝图引脚
- `get_node_connections` - 获取节点连接信息
- `create_blueprint_variable` - 创建蓝图变量
- `create_blueprint_function` - 创建蓝图函数

#### 场景工具
- `spawn_actor` - 生成Actor
- `move_actor` - 移动Actor
- `rotate_actor` - 旋转Actor
- `scale_actor` - 缩放Actor
- `delete_actor` - 删除Actor
- `get_scene_actors` - 获取场景Actor列表

#### 资产工具
- `find_assets` - 查找项目资产
- `create_asset` - 创建资产
- `rename_asset` - 重命名资产
- `move_asset` - 移动资产
- `delete_asset` - 删除资产
- `duplicate_asset` - 复制资产
- `get_asset_references` - 获取资产引用

#### 项目工具
- `create_game_mode` - 创建游戏模式
- `configure_input` - 配置输入映射
- `create_game_tag` - 创建游戏标签

### 应用场景

#### 创建角色蓝图
1. 创建角色蓝图类（继承自Character）
2. 添加角色移动组件
3. 添加弹簧臂和摄像机组件
4. 创建移动输入逻辑（WASD）
5. 添加鼠标视角控制
6. 引用动画蓝图
7. 配置默认属性
8. 在场景中放置实例

#### 实现计分系统
1. 创建游戏模式蓝图
2. 设计计分变量和逻辑
3. 创建UI Widget显示分数
4. 实现得分事件
5. 配置胜利/失败条件
6. 添加音效反馈
7. 实现关卡重启

#### 优化项目性能
1. 分析项目结构
2. 识别性能瓶颈
3. 优化材质复杂度
4. 合并静态网格体
5. 配置LOD
6. 设置剔除距离
7. 整理资产目录
8. 生成优化报告

### 故障排除

#### 插件加载失败
1. 关闭虚幻编辑器
2. 重新打开项目
3. 插件应该能正确加载

#### 连接被拒绝
1. 验证插件已在 **编辑 → 插件** 中启用
2. 检查端口8091未被防火墙阻止
3. 确保MCP服务器正在运行

#### 构建错误
1. 关闭您的IDE
2. 删除 `Intermediate/`、`Binaries/`、`Saved/` 文件夹
3. 重新生成项目文件
4. 重新构建项目

### 设计原则

#### 操作原子化
每个MCP工具执行一个特定操作：
- 允许灵活的操作组合
- 降低错误概率
- 简化测试和维护
- 提供清晰的错误信息

#### 状态可视化
完整的编辑器状态信息：
- 当前选中的对象
- 场景中的所有Actor
- 蓝图中的所有节点
- 连接关系
- 资产引用

#### 智能引导
- 操作建议
- 参数自动补全
- 上下文感知推荐
- 最佳实践提示

#### 安全控制
- 操作白名单
- 敏感操作确认
- 完整的操作日志
- 自动备份
- 操作回滚支持

### 实现路径

#### 阶段一：基础能力
- 蓝图节点操作
- 蓝图连接操作
- 变量和函数管理
- 基础场景操作

#### 阶段二：进阶能力
- 材质系统操作
- UI系统操作
- 动画系统操作
- 物理系统配置

#### 阶段三：高级能力
- AI系统操作
- 项目配置管理
- 性能优化工具
- 高级调试功能

### 支持版本

- 虚幻引擎 5.0 - 5.7
- 平台：Win64, Mac, Linux

### 许可证

MIT License
