import { z } from 'zod';
import { BaseTool } from '../../core/BaseTool.js';
import type { OperationContext, OperationResult, NodeInfo } from '../../types/index.js';
import { ueBridge } from '../../bridge/index.js';

const GetBlueprintNodesSchema = z.object({
  blueprint_path: z.string().describe('Full path to the blueprint asset'),
  graph_name: z.string().optional().describe('Name of the graph to get nodes from'),
  include_connections: z.boolean().optional().default(false).describe('Include connection information'),
});

export class GetBlueprintNodesTool extends BaseTool {
  readonly name = 'get_blueprint_nodes';
  readonly description = 'Get all nodes from a blueprint graph with their positions and types';
  readonly inputSchema = GetBlueprintNodesSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<NodeInfo[]>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetBlueprintNodesSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'get_blueprint_nodes',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_NODES_FAILED', result.error ?? 'Failed to get blueprint nodes');
      }

      const nodes = result.result as NodeInfo[];
      return this.createSuccessResult(nodes);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const CreateBlueprintNodeSchema = z.object({
  blueprint_path: z.string().describe('Full path to the blueprint asset'),
  node_type: z.string().describe('Type of node to create (e.g., "Event", "CallFunction", "Variable")'),
  graph_name: z.string().optional().describe('Target graph name'),
  position: z.object({
    x: z.number(),
    y: z.number(),
  }).optional().describe('Position on the graph canvas'),
  node_data: z.record(z.unknown()).optional().describe('Additional node configuration'),
  reference_node_id: z.string().optional().describe('ID of reference node for relative positioning'),
  direction: z.enum(['right', 'left', 'up', 'down']).optional().describe('Direction from reference node'),
  spacing: z.number().optional().default(256).describe('Spacing between nodes when using reference'),
});

export class CreateBlueprintNodeTool extends BaseTool {
  readonly name = 'create_blueprint_node';
  readonly description = 'Create a new node in a blueprint graph at the specified position or relative to a reference node';
  readonly inputSchema = CreateBlueprintNodeSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<NodeInfo>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof CreateBlueprintNodeSchema>;
      
      if (validated.reference_node_id && !validated.position) {
        const nodesResult = await ueBridge.executeCommand({
          command: 'get_blueprint_nodes',
          params: { blueprint_path: validated.blueprint_path },
          expectsResult: true,
        });

        if (nodesResult.success) {
          const nodes = nodesResult.result as NodeInfo[];
          const refNode = nodes.find(n => n.nodeId === validated.reference_node_id);
          
          if (refNode) {
            const spacing = validated.spacing ?? 256;
            let x = refNode.position.x;
            let y = refNode.position.y;
            
            switch (validated.direction ?? 'right') {
              case 'right':
                x += (refNode.size?.x ?? 200) + spacing;
                break;
              case 'left':
                x -= spacing + 200;
                break;
              case 'down':
                y += (refNode.size?.y ?? 100) + spacing;
                break;
              case 'up':
                y -= spacing + 100;
                break;
            }
            
            validated.position = {
              x: Math.round(x / 16) * 16,
              y: Math.round(y / 16) * 16,
            };
          }
        }
      }

      const result = await ueBridge.executeCommand({
        command: 'create_blueprint_node',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('CREATE_NODE_FAILED', result.error ?? 'Failed to create blueprint node');
      }

      const nodeInfo = result.result as NodeInfo;
      return this.createSuccessResult(nodeInfo);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const MoveBlueprintNodeSchema = z.object({
  blueprint_path: z.string().describe('Full path to the blueprint asset'),
  node_id: z.string().describe('ID of the node to move'),
  position: z.object({
    x: z.number(),
    y: z.number(),
  }).describe('New position on the graph canvas'),
  align_to_grid: z.boolean().optional().default(true).describe('Align position to grid'),
});

export class MoveBlueprintNodeTool extends BaseTool {
  readonly name = 'move_blueprint_node';
  readonly description = 'Move a blueprint node to a new position on the graph canvas';
  readonly inputSchema = MoveBlueprintNodeSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<NodeInfo>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof MoveBlueprintNodeSchema>;
      
      if (validated.align_to_grid) {
        validated.position.x = Math.round(validated.position.x / 16) * 16;
        validated.position.y = Math.round(validated.position.y / 16) * 16;
      }

      const result = await ueBridge.executeCommand({
        command: 'move_blueprint_node',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('MOVE_NODE_FAILED', result.error ?? 'Failed to move blueprint node');
      }

      const nodeInfo = result.result as NodeInfo;
      return this.createSuccessResult(nodeInfo);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const DeleteBlueprintNodeSchema = z.object({
  blueprint_path: z.string().describe('Full path to the blueprint asset'),
  node_id: z.string().describe('ID of the node to delete'),
  delete_connections: z.boolean().optional().default(true).describe('Also delete connected wires'),
});

export class DeleteBlueprintNodeTool extends BaseTool {
  readonly name = 'delete_blueprint_node';
  readonly description = 'Delete a node from a blueprint graph';
  readonly inputSchema = DeleteBlueprintNodeSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof DeleteBlueprintNodeSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'delete_blueprint_node',
        params: validated,
        expectsResult: false,
      });

      if (!result.success) {
        return this.createErrorResult('DELETE_NODE_FAILED', result.error ?? 'Failed to delete blueprint node');
      }

      return this.createSuccessResult();
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}
