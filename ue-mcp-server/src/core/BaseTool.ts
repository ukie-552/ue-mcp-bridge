import { z } from 'zod';
import type { 
  MCPToolDefinition, 
  MCPToolResult, 
  OperationResult,
  OperationContext 
} from '../types/index.js';

export abstract class BaseTool<T = unknown> {
  abstract readonly name: string;
  abstract readonly description: string;
  abstract readonly inputSchema: z.ZodType<unknown>;

  /**
   * 获取工具定义 - 使用简化的JSON Schema格式
   * @returns 符合MCP协议规范的工具定义
   */
  getDefinition(): MCPToolDefinition {
    return {
      name: this.name,
      description: this.description,
      inputSchema: {
        type: 'object',
        properties: {},
      },
    };
  }

  abstract execute(
    params: unknown, 
    context: OperationContext
  ): Promise<OperationResult<T>>;

  formatResult(result: OperationResult<T>): MCPToolResult {
    if (result.success) {
      return {
        content: [
          {
            type: 'text',
            text: JSON.stringify(result.data ?? { success: true }, null, 2),
          },
        ],
      };
    }

    return {
      content: [
        {
          type: 'text',
          text: JSON.stringify({
            error: result.error,
            warnings: result.warnings,
          }, null, 2),
        },
      ],
      isError: true,
    };
  }

  protected validateInput(params: unknown): unknown {
    return this.inputSchema.parse(params);
  }

  protected createSuccessResult<R = T>(data?: R): OperationResult<R> {
    const result: OperationResult<R> = {
      success: true,
    };
    if (data !== undefined) {
      result.data = data;
    }
    return result;
  }

  protected createErrorResult<R = T>(
    code: string, 
    message: string, 
    details?: unknown
  ): OperationResult<R> {
    return {
      success: false,
      error: {
        code,
        message,
        details,
      },
    };
  }

  protected addWarnings(
    result: OperationResult<T>, 
    warnings: string[]
  ): OperationResult<T> {
    return {
      ...result,
      warnings: [...(result.warnings ?? []), ...warnings],
    };
  }
}
