import type { UEPythonCommand, UEPythonResult } from '../types/index.js';
import { logger } from '../core/index.js';
import WebSocket from 'ws';

export interface BridgeConfig {
  host: string;
  port: number;
  timeout: number;
  retryAttempts: number;
  retryDelay: number;
  useWebSocket: boolean;
}

const DEFAULT_CONFIG: BridgeConfig = {
  host: process.env.MCP_AUTOMATION_HOST ?? '127.0.0.1',
  port: parseInt(process.env.MCP_AUTOMATION_PORT ?? '8091', 10),
  timeout: 30000,
  retryAttempts: 3,
  retryDelay: 1000,
  useWebSocket: true,
};

export class UEBridge {
  private config: BridgeConfig;
  private connected: boolean = false;
  private ws: WebSocket | null = null;
  private reconnectTimer: NodeJS.Timeout | null = null;
  private pendingCommands: Map<string, {
    resolve: (result: UEPythonResult) => void;
    reject: (error: Error) => void;
    timer: NodeJS.Timeout;
  }> = new Map();
  private commandIdCounter: number = 0;

  constructor(config: Partial<BridgeConfig> = {}) {
    this.config = { ...DEFAULT_CONFIG, ...config };
  }

  /**
   * 连接到UE编辑器 - 支持优雅降级
   * 即使UE编辑器未启动，服务器也会继续运行并尝试重连
   * @returns 始终返回true，确保服务器能够启动
   */
  async connect(): Promise<boolean> {
    if (this.config.useWebSocket) {
      this.connectWebSocket();
      return true;
    }
    
    this.connected = true;
    logger.info('Connected in legacy mode (Python script)');
    return true;
  }

  /**
   * WebSocket连接 - 非阻塞模式
   * 连接失败时会自动启动后台重连机制
   */
  private connectWebSocket(): void {
    try {
      const url = `ws://${this.config.host}:${this.config.port}`;
      
      logger.info('Connecting to MCP Automation Bridge...', undefined, { url });

      this.ws = new WebSocket(url, {
        handshakeTimeout: this.config.timeout,
      });

      this.ws.on('open', () => {
        this.connected = true;
        logger.info('Connected to MCP Automation Bridge', undefined, {
          host: this.config.host,
          port: this.config.port,
        });
        this.startHeartbeat();
      });

      this.ws.on('message', (data: WebSocket.Data) => {
        this.handleMessage(data.toString());
      });

      this.ws.on('close', () => {
        this.handleDisconnect();
      });

      this.ws.on('error', (error: Error) => {
        logger.warning('WebSocket connection failed - UE Editor may not be running', undefined, {
          error: error.message,
          willRetry: true
        });
        if (!this.connected) {
          this.scheduleReconnect();
        }
      });

    } catch (error) {
      logger.warning('Failed to connect to MCP Automation Bridge - starting in degraded mode', undefined, {
        error: error instanceof Error ? error.message : 'Unknown error',
        willRetry: true
      });
      this.scheduleReconnect();
    }
  }

  private startHeartbeat(): void {
    if (this.reconnectTimer) {
      clearInterval(this.reconnectTimer);
    }
    
    this.reconnectTimer = setInterval(() => {
      if (this.ws && this.connected) {
        this.ws.ping();
      }
    }, 30000);
  }

  private handleDisconnect(): void {
    this.connected = false;
    logger.warning('Disconnected from MCP Automation Bridge');
    
    if (this.reconnectTimer) {
      clearInterval(this.reconnectTimer);
      this.reconnectTimer = null;
    }

    for (const [id, pending] of this.pendingCommands) {
      clearTimeout(pending.timer);
      pending.reject(new Error('Connection lost'));
      this.pendingCommands.delete(id);
    }

    this.scheduleReconnect();
  }

  private scheduleReconnect(): void {
    if (this.reconnectTimer) {
      return;
    }

    const delay = this.config.retryDelay;
    logger.info('Scheduling reconnection attempt...', undefined, {
      delay: `${delay}ms`,
      host: this.config.host,
      port: this.config.port
    });

    this.reconnectTimer = setTimeout(() => {
      this.reconnectTimer = null;
      if (!this.connected) {
        logger.info('Attempting to reconnect to UE Editor...');
        this.connectWebSocket();
      }
    }, delay);
  }

  private handleMessage(data: string): void {
    try {
      const response = JSON.parse(data);
      const commandId = response.command_id;

      if (commandId && this.pendingCommands.has(commandId)) {
        const pending = this.pendingCommands.get(commandId)!;
        clearTimeout(pending.timer);
        this.pendingCommands.delete(commandId);

        const result: UEPythonResult = {
          success: response.success ?? false,
          result: response.result,
          error: response.error,
          executionTime: response.execution_time ?? 0,
        };

        pending.resolve(result);
      }
    } catch (error) {
      logger.error('Failed to parse response from MCP Bridge', undefined, error);
    }
  }

  disconnect(): void {
    if (this.reconnectTimer) {
      clearInterval(this.reconnectTimer);
      this.reconnectTimer = null;
    }

    if (this.ws) {
      this.ws.close();
      this.ws = null;
    }

    this.connected = false;
    logger.info('Disconnected from UE Editor');
  }

  isConnected(): boolean {
    return this.connected;
  }

  /**
   * 执行命令 - 检查连接状态
   * 未连接时返回友好的错误信息
   */
  async executeCommand(command: UEPythonCommand): Promise<UEPythonResult> {
    if (!this.connected) {
      return {
        success: false,
        error: 'UE Editor is not connected. Please ensure Unreal Engine is running with the MCP Automation Bridge plugin enabled. The server will automatically reconnect when available.',
        executionTime: 0,
      };
    }

    if (this.config.useWebSocket && this.ws) {
      return this.executeViaWebSocket(command);
    }

    return this.executeViaLegacy(command);
  }

  private executeViaWebSocket(command: UEPythonCommand): Promise<UEPythonResult> {
    return new Promise((resolve, reject) => {
      const commandId = `cmd_${++this.commandIdCounter}_${Date.now()}`;

      const message = JSON.stringify({
        command_id: commandId,
        command: command.command,
        params: command.params ?? {},
      });

      const timer = setTimeout(() => {
        this.pendingCommands.delete(commandId);
        resolve({
          success: false,
          error: 'Command timeout',
          executionTime: this.config.timeout,
        });
      }, this.config.timeout);

      this.pendingCommands.set(commandId, { resolve, reject, timer });

      if (this.ws && this.ws.readyState === WebSocket.OPEN) {
        this.ws.send(message);
      } else {
        clearTimeout(timer);
        this.pendingCommands.delete(commandId);
        resolve({
          success: false,
          error: 'WebSocket not connected',
          executionTime: 0,
        });
      }
    });
  }

  private async executeViaLegacy(command: UEPythonCommand): Promise<UEPythonResult> {
    const startTime = Date.now();

    try {
      const result = await this.sendCommand(command);
      const executionTime = Date.now() - startTime;

      return {
        ...result,
        executionTime,
      };
    } catch (error) {
      const executionTime = Date.now() - startTime;
      logger.error('Command execution failed', undefined, {
        command: command.command,
        error,
      });

      return {
        success: false,
        error: error instanceof Error ? error.message : 'Unknown error',
        executionTime,
      };
    }
  }

  private async sendCommand(command: UEPythonCommand): Promise<UEPythonResult> {
    const pythonScript = this.generatePythonScript(command);

    logger.debug('Generated Python script', undefined, {
      command: command.command,
      scriptLength: pythonScript.length,
    });

    return Promise.resolve({
      success: true,
      result: { script: pythonScript },
      executionTime: 0,
    });
  }

  private generatePythonScript(command: UEPythonCommand): string {
    const params = JSON.stringify(command.params);
    
    return `
import unreal
import json

def execute_mcp_command():
    try:
        params = json.loads('''${params}''')
        result = ${this.getCommandImplementation(command.command)}
        return json.dumps({"success": True, "result": result})
    except Exception as e:
        return json.dumps({"success": False, "error": str(e)})

if __name__ == "__main__":
    print(execute_mcp_command())
`;
  }

  private getCommandImplementation(command: string): string {
    const implementations: Record<string, string> = {
      get_blueprint_nodes: `
        bp = unreal.load_asset(params["blueprint_path"])
        if not bp:
            raise ValueError(f"Blueprint not found: {params['blueprint_path']}")
        
        nodes = []
        for graph in bp.get_graphs():
            for node in graph.nodes:
                nodes.append({
                    "node_id": str(node.node_guid),
                    "node_name": node.get_display_name(),
                    "node_type": node.__class__.__name__,
                    "position": {"x": node.node_pos_x, "y": node.node_pos_y},
                    "graph_name": graph.get_name()
                })
        return nodes
      `,
      spawn_actor: `
        actor_class = unreal.load_asset(params["actor_class"])
        if not actor_class:
            raise ValueError(f"Actor class not found: {params['actor_class']}")
        
        location = unreal.Vector(
            params["location"]["x"],
            params["location"]["y"],
            params["location"]["z"]
        )
        rotation = unreal.Rotator(
            params.get("rotation", {}).get("pitch", 0),
            params.get("rotation", {}).get("yaw", 0),
            params.get("rotation", {}).get("roll", 0)
        )
        
        actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
            actor_class, location, rotation
        )
        
        if params.get("name"):
            actor.set_actor_label(params["name"])
        
        return {
            "actor_id": str(actor.get_editor_property("actor_guid")),
            "actor_name": actor.get_actor_label(),
            "path": actor.get_path_name()
        }
      `,
    };

    return implementations[command] ?? 'return {"error": "Unknown command"}';
  }

  async executeWithRetry(command: UEPythonCommand): Promise<UEPythonResult> {
    let lastError: Error | null = null;

    for (let attempt = 0; attempt < this.config.retryAttempts; attempt++) {
      const result = await this.executeCommand(command);
      
      if (result.success) {
        return result;
      }

      lastError = new Error(result.error ?? 'Unknown error');
      
      if (attempt < this.config.retryAttempts - 1) {
        await this.delay(this.config.retryDelay * (attempt + 1));
      }
    }

    return {
      success: false,
      error: lastError?.message ?? 'Max retries exceeded',
      executionTime: 0,
    };
  }

  private delay(ms: number): Promise<void> {
    return new Promise(resolve => setTimeout(resolve, ms));
  }

  /**
   * 获取连接状态详情
   * @returns 连接状态信息对象
   */
  getConnectionStatus(): {
    connected: boolean;
    host: string;
    port: number;
    pendingCommands: number;
    reconnecting: boolean;
  } {
    return {
      connected: this.connected,
      host: this.config.host,
      port: this.config.port,
      pendingCommands: this.pendingCommands.size,
      reconnecting: !this.connected && this.reconnectTimer !== null,
    };
  }

  /**
   * 手动触发重连
   * 用于在UE编辑器启动后主动尝试连接
   */
  async reconnect(): Promise<boolean> {
    if (this.connected) {
      logger.info('Already connected to UE Editor');
      return true;
    }

    if (this.reconnectTimer) {
      clearTimeout(this.reconnectTimer);
      this.reconnectTimer = null;
    }

    logger.info('Manual reconnection triggered');
    this.connectWebSocket();
    
    return new Promise((resolve) => {
      setTimeout(() => {
        resolve(this.connected);
      }, 1000);
    });
  }
}

export const ueBridge = new UEBridge();
