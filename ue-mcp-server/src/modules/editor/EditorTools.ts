import { z } from 'zod';
import { BaseTool } from '../../core/BaseTool.js';
import type { OperationContext, OperationResult } from '../../types/index.js';
import { ueBridge } from '../../bridge/index.js';
import fs from 'fs/promises';
import path from 'path';

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

const TakeScreenshotSchema = z.object({
  output_path: z.string().optional().describe('Path to save the screenshot (default: Project/Screenshots/)'),
  width: z.number().optional().default(1920).describe('Screenshot width'),
  height: z.number().optional().default(1080).describe('Screenshot height'),
  show_ui: z.boolean().optional().default(false).describe('Show editor UI in screenshot'),
});

export class TakeScreenshotTool extends BaseTool {
  readonly name = 'take_screenshot';
  readonly description = 'Take a fullscreen screenshot of the UE editor viewport';
  readonly inputSchema = TakeScreenshotSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof TakeScreenshotSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'take_screenshot',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('SCREENSHOT_FAILED', result.error ?? 'Failed to take screenshot');
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

const ALLOWED_SCREENSHOT_EXTENSIONS = ['.png', '.jpg', '.jpeg', '.bmp', '.exr', '.hdr'];
const SCREENSHOT_DIR_NAME = 'Screenshots';

function validateScreenshotPath(filePath: string): { valid: boolean; error?: string; resolvedPath?: string } {
  const absolutePath = path.resolve(filePath);
  const normalizedPath = path.normalize(absolutePath);
  
  const ext = path.extname(normalizedPath).toLowerCase();
  if (!ALLOWED_SCREENSHOT_EXTENSIONS.includes(ext)) {
    return { 
      valid: false, 
      error: `Invalid file extension. Allowed extensions: ${ALLOWED_SCREENSHOT_EXTENSIONS.join(', ')}` 
    };
  }
  
  const pathSegments = normalizedPath.split(path.sep);
  const screenshotsIndex = pathSegments.lastIndexOf(SCREENSHOT_DIR_NAME);
  
  if (screenshotsIndex === -1) {
    return { 
      valid: false, 
      error: `File must be in a '${SCREENSHOT_DIR_NAME}' directory` 
    };
  }
  
  const screenshotDir = pathSegments.slice(0, screenshotsIndex + 1).join(path.sep);
  const relativePath = path.relative(screenshotDir, normalizedPath);
  
  if (relativePath.startsWith('..') || path.isAbsolute(relativePath)) {
    return { 
      valid: false, 
      error: 'Path traversal detected. File must be within the Screenshots directory' 
    };
  }
  
  return { valid: true, resolvedPath: normalizedPath };
}

const DeleteScreenshotSchema = z.object({
  file_path: z.string().describe('Full path to the screenshot file to delete (must be in a Screenshots directory)'),
});

export class DeleteScreenshotTool extends BaseTool {
  readonly name = 'delete_screenshot';
  readonly description = 'Delete a screenshot file to save space';
  readonly inputSchema = DeleteScreenshotSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof DeleteScreenshotSchema>;
      
      const validation = validateScreenshotPath(validated.file_path);
      if (!validation.valid) {
        return this.createErrorResult('INVALID_PATH', validation.error ?? 'Invalid screenshot path');
      }
      
      const absolutePath = validation.resolvedPath!;
      
      try {
        await fs.access(absolutePath);
      } catch {
        return this.createErrorResult('FILE_NOT_FOUND', `Screenshot file not found: ${absolutePath}`);
      }

      await fs.unlink(absolutePath);

      return this.createSuccessResult({
        deleted: true,
        file_path: absolutePath,
      });
    } catch (error) {
      return this.createErrorResult(
        'DELETE_FAILED',
        error instanceof Error ? error.message : 'Failed to delete screenshot'
      );
    }
  }
}

const GetEngineLogsSchema = z.object({
  max_count: z.number().optional().default(100).describe('Maximum number of logs to retrieve (1-1000)'),
});

export class GetEngineLogsTool extends BaseTool {
  readonly name = 'get_engine_logs';
  readonly description = 'Get recent output logs from the Unreal Engine editor';
  readonly inputSchema = GetEngineLogsSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetEngineLogsSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'get_engine_logs',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_LOGS_FAILED', result.error ?? 'Failed to get engine logs');
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
  new TakeScreenshotTool(),
  new DeleteScreenshotTool(),
  new GetEngineLogsTool(),
];
