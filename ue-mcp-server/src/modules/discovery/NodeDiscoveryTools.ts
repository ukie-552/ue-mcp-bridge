import { z } from 'zod';
import { BaseTool } from '../../core/BaseTool.js';
import type { OperationContext, OperationResult } from '../../types/index.js';
import { ueBridge } from '../../bridge/index.js';

const GetAvailableWidgetTypesSchema = z.object({
  category: z.string().optional().describe('Filter by category (Basic, Input, Panels, Lists, Advanced)'),
});

export class GetAvailableWidgetTypesTool extends BaseTool {
  readonly name = 'get_available_widget_types';
  readonly description = 'Get all available UMG widget types in Unreal Engine';
  readonly inputSchema = GetAvailableWidgetTypesSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetAvailableWidgetTypesSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'get_available_widget_types',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_WIDGET_TYPES_FAILED', result.error ?? 'Failed to get widget types');
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

const GetAvailableBlueprintNodesSchema = z.object({
  category: z.string().optional().describe('Filter by category (Flow Control, Variables, Math, Logic, Actor, Communication, Input, Utility)'),
  search: z.string().optional().describe('Search term to filter nodes'),
});

export class GetAvailableBlueprintNodesTool extends BaseTool {
  readonly name = 'get_available_blueprint_nodes';
  readonly description = 'Get all available blueprint node types in Unreal Engine';
  readonly inputSchema = GetAvailableBlueprintNodesSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetAvailableBlueprintNodesSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'get_available_blueprint_nodes',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_NODES_FAILED', result.error ?? 'Failed to get blueprint nodes');
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

const GetAvailableMaterialNodesSchema = z.object({
  category: z.string().optional().describe('Filter by category (Constants, Parameters, Texture, Math, Vector Operations, UV, World, Effects, Particles, Vertex, Time)'),
  search: z.string().optional().describe('Search term to filter nodes'),
});

export class GetAvailableMaterialNodesTool extends BaseTool {
  readonly name = 'get_available_material_nodes';
  readonly description = 'Get all available material node types in Unreal Engine';
  readonly inputSchema = GetAvailableMaterialNodesSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetAvailableMaterialNodesSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'get_available_material_nodes',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_NODES_FAILED', result.error ?? 'Failed to get material nodes');
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

const GetAvailableAnimationNodesSchema = z.object({
  category: z.string().optional().describe('Filter by category (State Machine, Animation Assets, Blending, IK, Bone Manipulation, etc.)'),
  search: z.string().optional().describe('Search term to filter nodes'),
});

export class GetAvailableAnimationNodesTool extends BaseTool {
  readonly name = 'get_available_animation_nodes';
  readonly description = 'Get all available animation node types in Unreal Engine';
  readonly inputSchema = GetAvailableAnimationNodesSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetAvailableAnimationNodesSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'get_available_animation_nodes',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_NODES_FAILED', result.error ?? 'Failed to get animation nodes');
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

export const nodeDiscoveryTools: BaseTool<unknown>[] = [
  new GetAvailableWidgetTypesTool(),
  new GetAvailableBlueprintNodesTool(),
  new GetAvailableMaterialNodesTool(),
  new GetAvailableAnimationNodesTool(),
];
