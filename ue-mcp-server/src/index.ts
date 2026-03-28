import { Server } from '@modelcontextprotocol/sdk/server/index.js';
import { StdioServerTransport } from '@modelcontextprotocol/sdk/server/stdio.js';
import {
  CallToolRequestSchema,
  ListToolsRequestSchema,
  ListResourcesRequestSchema,
  ReadResourceRequestSchema,
} from '@modelcontextprotocol/sdk/types.js';
import type { CallToolResult } from '@modelcontextprotocol/sdk/types.js';

import { allTools } from './modules/index.js';
import { SafetyController, logger } from './core/index.js';
import { ueBridge } from './bridge/index.js';
import type { OperationContext } from './types/index.js';

const safetyController = new SafetyController();

const server = new Server(
  {
    name: 'ue-editor-mcp-server',
    version: '1.0.0',
  },
  {
    capabilities: {
      tools: {},
      resources: {},
    },
  }
);

server.setRequestHandler(ListToolsRequestSchema, () => {
  return {
    tools: allTools.map(tool => tool.getDefinition()),
  };
});

server.setRequestHandler(CallToolRequestSchema, async (request): Promise<CallToolResult> => {
  const { name, arguments: args } = request.params;
  
  const context: OperationContext = {
    operationId: `op_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`,
    timestamp: new Date(),
    sessionId: 'default',
  };

  logger.info(`Tool called: ${name}`, context, { arguments: args });

  const safetyCheck = safetyController.checkOperation(name, args ?? {}, context);
  
  if (!safetyCheck.allowed) {
    return {
      content: [
        {
          type: 'text',
          text: JSON.stringify({
            error: 'Operation not allowed',
            warnings: safetyCheck.warnings,
          }, null, 2),
        },
      ],
      isError: true,
    };
  }

  if (safetyCheck.requiresConfirmation) {
    logger.warning(`Operation "${name}" requires confirmation`, context, {
      riskLevel: safetyCheck.riskLevel,
      warnings: safetyCheck.warnings,
    });
  }

  const tool = allTools.find(t => t.name === name);
  
  if (!tool) {
    return {
      content: [
        {
          type: 'text',
          text: JSON.stringify({
            error: `Unknown tool: ${name}`,
          }, null, 2),
        },
      ],
      isError: true,
    };
  }

  try {
    const result = await tool.execute(args ?? {}, context);
    const mcpResult = tool.formatResult(result) as CallToolResult;
    
    logger.info(`Tool execution completed: ${name}`, context, {
      success: result.success,
    });

    return mcpResult;
  } catch (error) {
    logger.error(`Tool execution failed: ${name}`, context, error);
    
    return {
      content: [
        {
          type: 'text',
          text: JSON.stringify({
            error: error instanceof Error ? error.message : 'Unknown error',
          }, null, 2),
        },
      ],
      isError: true,
    };
  }
});

server.setRequestHandler(ListResourcesRequestSchema, () => {
  return {
    resources: [
      {
        uri: 'ue://editor/status',
        name: 'UE Editor Status',
        description: 'Current status of the Unreal Engine Editor connection',
        mimeType: 'application/json',
      },
      {
        uri: 'ue://scene/snapshot',
        name: 'Scene Snapshot',
        description: 'Current scene state including all actors',
        mimeType: 'application/json',
      },
      {
        uri: 'ue://project/info',
        name: 'Project Information',
        description: 'Information about the current UE project',
        mimeType: 'application/json',
      },
    ],
  };
});

server.setRequestHandler(ReadResourceRequestSchema, async (request) => {
  const { uri } = request.params;

  switch (uri) {
    case 'ue://editor/status':
      return {
        contents: [
          {
            uri,
            mimeType: 'application/json',
            text: JSON.stringify({
              connected: ueBridge.isConnected(),
              status: ueBridge.isConnected() ? 'connected' : 'waiting_for_connection',
              message: ueBridge.isConnected() 
                ? 'UE Editor is connected and ready'
                : 'UE Editor is not connected. Please start Unreal Engine with MCP Automation Bridge plugin enabled.',
              connectionDetails: ueBridge.getConnectionStatus(),
              timestamp: new Date().toISOString(),
            }, null, 2),
          },
        ],
      };

    case 'ue://scene/snapshot': {
      if (!ueBridge.isConnected()) {
        return {
          contents: [
            {
              uri,
              mimeType: 'application/json',
              text: JSON.stringify({
                error: 'UE Editor not connected',
                message: 'Please start Unreal Engine to get scene snapshot',
                timestamp: new Date().toISOString(),
              }, null, 2),
            },
          ],
        };
      }

      const actorsResult = await ueBridge.executeCommand({
        command: 'get_scene_actors',
        params: {},
        expectsResult: true,
      });

      return {
        contents: [
          {
            uri,
            mimeType: 'application/json',
            text: JSON.stringify({
              actors: actorsResult.success ? actorsResult.result : [],
              timestamp: new Date().toISOString(),
            }, null, 2),
          },
        ],
      };
    }

    case 'ue://project/info': {
      if (!ueBridge.isConnected()) {
        return {
          contents: [
            {
              uri,
              mimeType: 'application/json',
              text: JSON.stringify({
                error: 'UE Editor not connected',
                message: 'Please start Unreal Engine to get project info',
                timestamp: new Date().toISOString(),
              }, null, 2),
            },
          ],
        };
      }

      const projectResult = await ueBridge.executeCommand({
        command: 'get_project_info',
        params: {},
        expectsResult: true,
      });

      return {
        contents: [
          {
            uri,
            mimeType: 'application/json',
            text: JSON.stringify(
              projectResult.success ? projectResult.result : { error: 'Failed to get project info' },
              null, 2
            ),
          },
        ],
      };
    }

    default:
      throw new Error(`Unknown resource: ${uri}`);
  }
});

/**
 * 主函数 - 启动MCP服务器
 * 支持优雅降级：即使UE编辑器未启动，服务器也会正常运行
 */
async function main(): Promise<void> {
  logger.info('Starting UE Editor MCP Server...');

  try {
    await ueBridge.connect();
    logger.info('UE Bridge initialized - server ready');
  } catch (error) {
    logger.warning('UE Bridge initialization failed - running in degraded mode', undefined, {
      error: error instanceof Error ? error.message : 'Unknown error',
      note: 'Server will continue running and attempt to reconnect when UE Editor is available'
    });
  }

  const transport = new StdioServerTransport();
  await server.connect(transport);

  logger.info('UE Editor MCP Server is running');
  logger.info('Server status:', undefined, {
    ueConnected: ueBridge.isConnected(),
    mode: ueBridge.isConnected() ? 'connected' : 'waiting for UE Editor'
  });
}

main().catch((error) => {
  logger.error('Fatal error in main', undefined, error);
  process.exit(1);
});
