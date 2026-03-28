import { z } from 'zod';
import { BaseTool } from '../../core/BaseTool.js';
import type { OperationContext, OperationResult } from '../../types/index.js';
import { ueBridge } from '../../bridge/index.js';

const CreateMaterialSchema = z.object({
  material_path: z.string().describe('Path where the material will be created (e.g., /Game/Materials)'),
  material_name: z.string().describe('Name of the material'),
  blend_mode: z.enum(['Opaque', 'Translucent', 'Masked', 'Additive']).optional().default('Opaque').describe('Blend mode'),
  two_sided: z.boolean().optional().default(false).describe('Whether the material is two-sided'),
});

export class CreateMaterialTool extends BaseTool {
  readonly name = 'create_material';
  readonly description = 'Create a new material asset';
  readonly inputSchema = CreateMaterialSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof CreateMaterialSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'create_material',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('CREATE_FAILED', result.error ?? 'Failed to create material');
      }

      return this.createSuccessResult(result.result);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const CreateMaterialInstanceSchema = z.object({
  instance_path: z.string().describe('Path where the material instance will be created'),
  instance_name: z.string().describe('Name of the material instance'),
  parent_material: z.string().optional().describe('Path to parent material'),
});

export class CreateMaterialInstanceTool extends BaseTool {
  readonly name = 'create_material_instance';
  readonly description = 'Create a new material instance';
  readonly inputSchema = CreateMaterialInstanceSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof CreateMaterialInstanceSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'create_material_instance',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('CREATE_FAILED', result.error ?? 'Failed to create material instance');
      }

      return this.createSuccessResult(result.result);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const CreateMaterialNodeSchema = z.object({
  material_path: z.string().describe('Path to the material'),
  node_type: z.string().describe('Type of node to create (e.g., Constant, Multiply, TextureSample, ScalarParameter)'),
  position: z.object({
    x: z.number(),
    y: z.number(),
  }).optional().describe('Position in the material graph'),
  value: z.union([
    z.number(),
    z.object({ x: z.number(), y: z.number() }),
    z.object({ r: z.number(), g: z.number(), b: z.number(), a: z.number().optional() }),
  ]).optional().describe('Value for constant nodes'),
  parameter_name: z.string().optional().describe('Parameter name for parameter nodes'),
  default_value: z.union([z.number(), z.object({ r: z.number(), g: z.number(), b: z.number(), a: z.number().optional() })]).optional().describe('Default value for parameters'),
  texture: z.string().optional().describe('Texture path for texture sample nodes'),
  coordinate_index: z.number().optional().describe('UV coordinate index'),
  u_tiling: z.number().optional().describe('U tiling for texture coordinates'),
  v_tiling: z.number().optional().describe('V tiling for texture coordinates'),
  speed_x: z.number().optional().describe('X speed for panner'),
  speed_y: z.number().optional().describe('Y speed for panner'),
});

export class CreateMaterialNodeTool extends BaseTool {
  readonly name = 'create_material_node';
  readonly description = 'Create a node in the material graph';
  readonly inputSchema = CreateMaterialNodeSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof CreateMaterialNodeSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'create_material_node',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('CREATE_NODE_FAILED', result.error ?? 'Failed to create material node');
      }

      return this.createSuccessResult(result.result);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const ConnectMaterialPinsSchema = z.object({
  material_path: z.string().describe('Path to the material'),
  source_node: z.string().describe('Name of the source node'),
  source_output: z.string().optional().describe('Name of the output pin on source node'),
  target_node: z.string().describe('Name of the target node (or material output like BaseColor, Normal)'),
  target_input: z.string().optional().describe('Name of the input pin on target node'),
});

export class ConnectMaterialPinsTool extends BaseTool {
  readonly name = 'connect_material_pins';
  readonly description = 'Connect two pins in the material graph';
  readonly inputSchema = ConnectMaterialPinsSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof ConnectMaterialPinsSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'connect_material_pins',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('CONNECT_FAILED', result.error ?? 'Failed to connect material pins');
      }

      return this.createSuccessResult(result.result);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const DisconnectMaterialPinsSchema = z.object({
  material_path: z.string().describe('Path to the material'),
  target_node: z.string().describe('Name of the target node'),
  target_input: z.string().optional().describe('Name of the input pin to disconnect (disconnects all if not specified)'),
});

export class DisconnectMaterialPinsTool extends BaseTool {
  readonly name = 'disconnect_material_pins';
  readonly description = 'Disconnect pins in the material graph';
  readonly inputSchema = DisconnectMaterialPinsSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof DisconnectMaterialPinsSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'disconnect_material_pins',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('DISCONNECT_FAILED', result.error ?? 'Failed to disconnect material pins');
      }

      return this.createSuccessResult(result.result);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const SetMaterialNodePropertySchema = z.object({
  material_path: z.string().describe('Path to the material'),
  node_name: z.string().describe('Name of the node'),
  property_name: z.string().describe('Name of the property to set'),
  property_value: z.record(z.unknown()).describe('Value object for the property'),
});

export class SetMaterialNodePropertyTool extends BaseTool {
  readonly name = 'set_material_node_property';
  readonly description = 'Set a property on a material node';
  readonly inputSchema = SetMaterialNodePropertySchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof SetMaterialNodePropertySchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'set_material_node_property',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('SET_PROPERTY_FAILED', result.error ?? 'Failed to set material node property');
      }

      return this.createSuccessResult(result.result);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const GetMaterialGraphSchema = z.object({
  material_path: z.string().describe('Path to the material'),
});

export class GetMaterialGraphTool extends BaseTool {
  readonly name = 'get_material_graph';
  readonly description = 'Get the material graph structure including all nodes and connections';
  readonly inputSchema = GetMaterialGraphSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetMaterialGraphSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'get_material_graph',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_GRAPH_FAILED', result.error ?? 'Failed to get material graph');
      }

      return this.createSuccessResult(result.result);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const DeleteMaterialNodeSchema = z.object({
  material_path: z.string().describe('Path to the material'),
  node_name: z.string().describe('Name of the node to delete'),
});

export class DeleteMaterialNodeTool extends BaseTool {
  readonly name = 'delete_material_node';
  readonly description = 'Delete a node from the material graph';
  readonly inputSchema = DeleteMaterialNodeSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof DeleteMaterialNodeSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'delete_material_node',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('DELETE_FAILED', result.error ?? 'Failed to delete material node');
      }

      return this.createSuccessResult(result.result);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

export const materialTools: BaseTool<unknown>[] = [
  new CreateMaterialTool(),
  new CreateMaterialInstanceTool(),
  new CreateMaterialNodeTool(),
  new ConnectMaterialPinsTool(),
  new DisconnectMaterialPinsTool(),
  new SetMaterialNodePropertyTool(),
  new GetMaterialGraphTool(),
  new DeleteMaterialNodeTool(),
];
