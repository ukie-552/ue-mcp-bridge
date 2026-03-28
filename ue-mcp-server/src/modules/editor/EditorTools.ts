import { z } from 'zod';
import { BaseTool } from '../../core/BaseTool.js';
import type { OperationContext, OperationResult } from '../../types/index.js';
import { ueBridge } from '../../bridge/index.js';

const GetAvailableComponentTypesSchema = z.object({
  category: z.string().optional().describe('Filter by category (Rendering, Lights, Camera, Physics, Audio, Movement, AI, Effects, Utility)'),
});

export class GetAvailableComponentTypesTool extends BaseTool {
  readonly name = 'get_available_component_types';
  readonly description = 'Get all available component types in Unreal Engine';
  readonly inputSchema = GetAvailableComponentTypesSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetAvailableComponentTypesSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'get_available_component_types',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_COMPONENT_TYPES_FAILED', result.error ?? 'Failed to get component types');
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

const RemoveComponentSchema = z.object({
  actor_id: z.string().describe('The ID of the actor'),
  component_name: z.string().describe('The name of the component to remove'),
});

export class RemoveComponentTool extends BaseTool {
  readonly name = 'remove_component';
  readonly description = 'Remove a component from an actor';
  readonly inputSchema = RemoveComponentSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof RemoveComponentSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'remove_component',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('REMOVE_COMPONENT_FAILED', result.error ?? 'Failed to remove component');
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

const SetComponentPropertySchema = z.object({
  actor_id: z.string().describe('The ID of the actor'),
  component_name: z.string().describe('The name of the component'),
  property_name: z.string().describe('The name of the property to set'),
  value: z.any().describe('The value to set (number, string, bool, or object for structs)'),
});

export class SetComponentPropertyTool extends BaseTool {
  readonly name = 'set_component_property';
  readonly description = 'Set a property value on a component';
  readonly inputSchema = SetComponentPropertySchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof SetComponentPropertySchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'set_component_property',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('SET_PROPERTY_FAILED', result.error ?? 'Failed to set component property');
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

const SelectActorsSchema = z.object({
  actor_ids: z.array(z.string()).optional().describe('Array of actor IDs to select'),
  actor_id: z.string().optional().describe('Single actor ID to select'),
  add_to_selection: z.boolean().optional().describe('Add to current selection instead of replacing'),
  deselect_all_first: z.boolean().optional().describe('Deselect all actors first'),
});

export class SelectActorsTool extends BaseTool {
  readonly name = 'select_actors';
  readonly description = 'Select actors in the editor viewport';
  readonly inputSchema = SelectActorsSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof SelectActorsSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'select_actors',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('SELECT_ACTORS_FAILED', result.error ?? 'Failed to select actors');
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

const GetActorComponentsSchema = z.object({
  actor_id: z.string().describe('The ID of the actor'),
  include_properties: z.boolean().optional().describe('Include editable properties for each component'),
});

export class GetActorComponentsTool extends BaseTool {
  readonly name = 'get_actor_components';
  readonly description = 'Get all components attached to an actor';
  readonly inputSchema = GetActorComponentsSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetActorComponentsSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'get_actor_components',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_COMPONENTS_FAILED', result.error ?? 'Failed to get actor components');
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

const EditorUndoRedoSchema = z.object({
  action: z.enum(['undo', 'redo']).describe('The action to perform'),
  count: z.number().optional().describe('Number of undo/redo operations to perform'),
});

export class EditorUndoRedoTool extends BaseTool {
  readonly name = 'editor_undo_redo';
  readonly description = 'Perform undo or redo operations in the editor';
  readonly inputSchema = EditorUndoRedoSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof EditorUndoRedoSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'editor_undo_redo',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('UNDO_REDO_FAILED', result.error ?? 'Failed to perform undo/redo');
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

const GetDetailsPanelPropertiesSchema = z.object({
  object_type: z.enum(['actor', 'component', 'class']).describe('The type of object to inspect'),
  object_id: z.string().describe('The ID or path of the object'),
  actor_id: z.string().optional().describe('Required if object_type is "component"'),
  category: z.string().optional().describe('Filter by property category'),
});

export class GetDetailsPanelPropertiesTool extends BaseTool {
  readonly name = 'get_details_panel_properties';
  readonly description = 'Get all editable properties from the Details panel for an object';
  readonly inputSchema = GetDetailsPanelPropertiesSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetDetailsPanelPropertiesSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'get_details_panel_properties',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_PROPERTIES_FAILED', result.error ?? 'Failed to get details panel properties');
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

export const editorTools: BaseTool<unknown>[] = [
  new GetAvailableComponentTypesTool(),
  new RemoveComponentTool(),
  new SetComponentPropertyTool(),
  new SelectActorsTool(),
  new GetActorComponentsTool(),
  new EditorUndoRedoTool(),
  new GetDetailsPanelPropertiesTool(),
];
