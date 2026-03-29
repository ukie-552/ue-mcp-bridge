import { z } from 'zod';
import { BaseTool } from '../../core/BaseTool.js';
import type { OperationContext, OperationResult, NodeInfo, ConnectionInfo, PinInfo, PinType } from '../../types/index.js';
import { ueBridge } from '../../bridge/index.js';

const RawPinDataSchema = z.object({
  pin_name: z.string(),
  pin_type: z.string(),
  is_array: z.boolean(),
  direction: z.enum(['input', 'output']),
});

const PositionSchema = z.object({
  x: z.number(),
  y: z.number(),
});

const SizeSchema = z.object({
  x: z.number(),
  y: z.number(),
});

const RawNodeDataSchema = z.object({
  node_id: z.string(),
  node_name: z.string(),
  node_type: z.string(),
  position: PositionSchema,
  size: SizeSchema.optional(),
  comment: z.string().optional(),
  graph_name: z.string().optional(),
  pins: z.array(RawPinDataSchema).optional(),
});

const BlueprintNodesResponseSchema = z.object({
  nodes: z.array(RawNodeDataSchema),
});

type RawPinData = z.infer<typeof RawPinDataSchema>;
type RawNodeData = z.infer<typeof RawNodeDataSchema>;
type BlueprintNodesResponse = z.infer<typeof BlueprintNodesResponseSchema>;

function isBlueprintNodesResponse(data: unknown): data is BlueprintNodesResponse {
  const result = BlueprintNodesResponseSchema.safeParse(data);
  return result.success;
}

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

const GetBlueprintGraphTreeSchema = z.object({
  blueprint_path: z.string().describe('Full path to the blueprint asset'),
  graph_name: z.string().optional().describe('Name of the graph to analyze (default: all graphs)'),
  include_pins: z.boolean().optional().default(true).describe('Include detailed pin information'),
  include_connections: z.boolean().optional().default(true).describe('Include connection relationships'),
});

export interface BlueprintGraphTree {
  blueprintPath: string;
  graphs: Array<{
    graphName: string;
    nodes: Array<NodeInfo & {
      pins?: PinInfo[];
      inputConnections?: Array<{
        sourceNodeId: string;
        sourcePinName: string;
        targetPinName: string;
      }>;
      outputConnections?: Array<{
        targetNodeId: string;
        sourcePinName: string;
        targetPinName: string;
      }>;
    }>;
    connections: ConnectionInfo[];
  }>;
  summary: {
    totalNodes: number;
    totalConnections: number;
    graphsAnalyzed: number;
  };
}

export class GetBlueprintGraphTreeTool extends BaseTool {
  readonly name = 'get_blueprint_graph_tree';
  readonly description = 'Get complete blueprint graph tree including all nodes, pins, and connection relationships';
  readonly inputSchema = GetBlueprintGraphTreeSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<BlueprintGraphTree>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetBlueprintGraphTreeSchema>;
      
      const nodesResult = await ueBridge.executeCommand({
        command: 'get_blueprint_nodes',
        params: {
          blueprint_path: validated.blueprint_path,
          graph_name: validated.graph_name,
          include_connections: validated.include_connections,
        },
        expectsResult: true,
      });

      if (!nodesResult.success) {
        return this.createErrorResult('GET_NODES_FAILED', nodesResult.error ?? 'Failed to get blueprint nodes');
      }

      if (!isBlueprintNodesResponse(nodesResult.result)) {
        return this.createErrorResult('INVALID_RESPONSE', 'Invalid response format from blueprint nodes query');
      }
      
      const nodes = nodesResult.result.nodes;
      
      const graphTree: BlueprintGraphTree = {
        blueprintPath: validated.blueprint_path,
        graphs: [],
        summary: {
          totalNodes: 0,
          totalConnections: 0,
          graphsAnalyzed: 0,
        },
      };

      const graphMap = new Map<string, any>();
      
      for (const node of nodes) {
        const graphName = node.graph_name || 'unknown';
        
        if (!graphMap.has(graphName)) {
          graphMap.set(graphName, {
            graphName,
            nodes: [],
            connections: [],
          });
        }

        const graphData = graphMap.get(graphName)!;
        
        const nodeInfo: any = {
          nodeId: node.node_id,
          nodeName: node.node_name,
          nodeType: node.node_type,
          position: node.position,
          size: node.size,
          comment: node.comment,
          graphName: graphName,
          blueprintPath: validated.blueprint_path,
        };

        if (validated.include_pins && node.pins) {
          nodeInfo.pins = node.pins.map((pin: any) => ({
            pinId: `${node.node_id}_${pin.pin_name}`,
            pinName: pin.pin_name,
            pinType: {
              category: pin.pin_type,
              isArray: pin.is_array,
              isReference: false,
            },
            isInput: pin.direction === 'input',
            connectedPins: [],
            owningNodeId: node.node_id,
          }));
        }

        graphData.nodes.push(nodeInfo);
      }

      if (validated.include_connections) {
        for (const [graphName, graphData] of graphMap) {
          const connectionsResult = await ueBridge.executeCommand({
            command: 'get_node_connections',
            params: {
              blueprint_path: validated.blueprint_path,
              graph_name: graphName,
            },
            expectsResult: true,
          });

          if (connectionsResult.success) {
            const connections = connectionsResult.result as ConnectionInfo[];
            graphData.connections = connections;
            
            for (const conn of connections) {
              const sourceNode = graphData.nodes.find((n: any) => n.nodeId === conn.sourceNodeId);
              const targetNode = graphData.nodes.find((n: any) => n.nodeId === conn.targetNodeId);
              
              if (sourceNode) {
                if (!sourceNode.outputConnections) sourceNode.outputConnections = [];
                sourceNode.outputConnections.push({
                  targetNodeId: conn.targetNodeId,
                  sourcePinName: conn.sourcePinId.split('_').pop() || '',
                  targetPinName: conn.targetPinId.split('_').pop() || '',
                });
              }
              
              if (targetNode) {
                if (!targetNode.inputConnections) targetNode.inputConnections = [];
                targetNode.inputConnections.push({
                  sourceNodeId: conn.sourceNodeId,
                  sourcePinName: conn.sourcePinId.split('_').pop() || '',
                  targetPinName: conn.targetPinId.split('_').pop() || '',
                });
              }
            }
            
            graphTree.summary.totalConnections += connections.length;
          }
        }
      }

      for (const graphData of graphMap.values()) {
        graphTree.graphs.push(graphData);
        graphTree.summary.totalNodes += graphData.nodes.length;
      }
      graphTree.summary.graphsAnalyzed = graphTree.graphs.length;

      return this.createSuccessResult(graphTree);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const GetActiveBlueprintStateSchema = z.object({
  blueprint_path: z.string().optional().describe('Optional: Path to a specific blueprint (if not provided, uses the currently active blueprint)'),
});

export interface ActiveBlueprintState {
  blueprintPath: string;
  blueprintName: string;
  currentGraph: string;
  isActiveBlueprint: boolean;
  graphs: Array<{
    graphName: string;
    isCurrent: boolean;
    nodes: Array<{
      nodeId: string;
      nodeName: string;
      nodeType: string;
      position: { x: number; y: number };
      pins?: Array<{
        pinName: string;
        pinType: string;
        direction: 'input' | 'output';
        isArray: boolean;
        hasConnection: boolean;
      }>;
      inputConnections?: Array<{
        sourceNodeId: string;
        sourcePinName: string;
        targetPinName: string;
      }>;
      outputConnections?: Array<{
        targetNodeId: string;
        sourcePinName: string;
        targetPinName: string;
      }>;
    }>;
  }>;
  selectedNodes: string[];
}

export class GetActiveBlueprintStateTool extends BaseTool {
  readonly name = 'get_active_blueprint_state';
  readonly description = 'Get the current state of the active blueprint editor, or a specific blueprint by path. Includes all nodes and their connection relationships.';
  readonly inputSchema = GetActiveBlueprintStateSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<ActiveBlueprintState>> {
    try {
      const result = await ueBridge.executeCommand({
        command: 'get_active_blueprint_state',
        params: {},
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_ACTIVE_BLUEPRINT_FAILED', result.error ?? 'Failed to get active blueprint state');
      }

      return this.createSuccessResult(result.result as ActiveBlueprintState);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}
