import { z } from 'zod';
import { BaseTool } from '../../core/BaseTool.js';
import type { OperationContext, OperationResult } from '../../types/index.js';
import { ueBridge } from '../../bridge/index.js';

const CreateLevelSchema = z.object({
  level_name: z.string().describe('Name for the new level'),
  template: z.enum(['Empty', 'Basic', 'TimeOfDay']).optional().default('Empty').describe('Level template'),
  save_path: z.string().optional().describe('Path to save the level'),
});

export class CreateLevelTool extends BaseTool<{ level_path: string }> {
  readonly name = 'create_level';
  readonly description = 'Create a new level in the project';
  readonly inputSchema = CreateLevelSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<{ level_path: string }>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof CreateLevelSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'create_level',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('CREATE_LEVEL_FAILED', result.error ?? 'Failed to create level');
      }

      return this.createSuccessResult(result.result as { level_path: string });
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const LoadLevelSchema = z.object({
  level_path: z.string().describe('Path to the level to load'),
});

export class LoadLevelTool extends BaseTool {
  readonly name = 'load_level';
  readonly description = 'Load an existing level';
  readonly inputSchema = LoadLevelSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof LoadLevelSchema>;

      const result = await ueBridge.executeCommand({
        command: 'load_level',
        params: validated,
        expectsResult: false,
      });

      if (!result.success) {
        return this.createErrorResult('LOAD_LEVEL_FAILED', result.error ?? 'Failed to load level');
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

const SaveCurrentLevelSchema = z.object({
  save_as: z.string().optional().describe('Save with a new name'),
});

export class SaveCurrentLevelTool extends BaseTool {
  readonly name = 'save_current_level';
  readonly description = 'Save the current level';
  readonly inputSchema = SaveCurrentLevelSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof SaveCurrentLevelSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'save_current_level',
        params: validated,
        expectsResult: false,
      });

      if (!result.success) {
        return this.createErrorResult('SAVE_LEVEL_FAILED', result.error ?? 'Failed to save level');
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

const AddLevelStreamingSchema = z.object({
  level_path: z.string().describe('Path to the level to add as streaming'),
  streaming_method: z.enum(['AlwaysLoaded', 'Blueprint']).optional().default('Blueprint').describe('Streaming method'),
  offset: z.object({
    x: z.number(),
    y: z.number(),
    z: z.number(),
  }).optional().describe('World offset for the streaming level'),
});

export class AddLevelStreamingTool extends BaseTool {
  readonly name = 'add_level_streaming';
  readonly description = 'Add a streaming level to the current world';
  readonly inputSchema = AddLevelStreamingSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof AddLevelStreamingSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'add_level_streaming',
        params: validated,
        expectsResult: false,
      });

      if (!result.success) {
        return this.createErrorResult('ADD_STREAMING_FAILED', result.error ?? 'Failed to add streaming level');
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

const SetWorldSettingsSchema = z.object({
  settings: z.record(z.unknown()).describe('World settings to apply'),
});

export class SetWorldSettingsTool extends BaseTool {
  readonly name = 'set_world_settings';
  readonly description = 'Configure world settings for the current level';
  readonly inputSchema = SetWorldSettingsSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof SetWorldSettingsSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'set_world_settings',
        params: validated,
        expectsResult: false,
      });

      if (!result.success) {
        return this.createErrorResult('SET_SETTINGS_FAILED', result.error ?? 'Failed to set world settings');
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
