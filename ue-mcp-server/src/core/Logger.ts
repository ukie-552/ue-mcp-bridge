import type { LogEntry, LogLevel, OperationContext } from '../types/index.js';

export class Logger {
  private readonly entries: LogEntry[] = [];
  private readonly maxEntries: number = 10000;
  private minLevel: LogLevel = 'info';

  setMinLevel(level: LogLevel): void {
    this.minLevel = level;
  }

  private readonly levelPriority: Record<LogLevel, number> = {
    debug: 0,
    info: 1,
    warning: 2,
    error: 3,
  };

  private shouldLog(level: LogLevel): boolean {
    return this.levelPriority[level] >= this.levelPriority[this.minLevel];
  }

  private log(
    level: LogLevel, 
    message: string, 
    context?: OperationContext, 
    data?: unknown
  ): void {
    if (!this.shouldLog(level)) {
      return;
    }

    const entry: LogEntry = {
      level,
      message,
      timestamp: new Date(),
      context,
      data,
    };

    this.entries.push(entry);

    if (this.entries.length > this.maxEntries) {
      this.entries.shift();
    }

    this.outputToConsole(entry);
  }

  /**
   * 输出到控制台 - 所有日志都输出到stderr以避免干扰MCP协议
   * MCP协议使用stdout进行JSON通信，因此日志必须输出到stderr
   */
  private outputToConsole(entry: LogEntry): void {
    const timestamp = entry.timestamp.toISOString();
    const prefix = `[${timestamp}] [${entry.level.toUpperCase()}]`;
    
    const output = entry.context
      ? `${prefix} [${entry.context.operationId}] ${entry.message}`
      : `${prefix} ${entry.message}`;

    const dataStr = entry.data ? ` ${JSON.stringify(entry.data)}` : '';
    const fullOutput = `${output}${dataStr}`;

    console.error(fullOutput);
  }

  debug(message: string, context?: OperationContext, data?: unknown): void {
    this.log('debug', message, context, data);
  }

  info(message: string, context?: OperationContext, data?: unknown): void {
    this.log('info', message, context, data);
  }

  warning(message: string, context?: OperationContext, data?: unknown): void {
    this.log('warning', message, context, data);
  }

  error(message: string, context?: OperationContext, data?: unknown): void {
    this.log('error', message, context, data);
  }

  getEntries(level?: LogLevel): LogEntry[] {
    if (level) {
      return this.entries.filter(e => e.level === level);
    }
    return [...this.entries];
  }

  clear(): void {
    this.entries.length = 0;
  }
}

export const logger = new Logger();
