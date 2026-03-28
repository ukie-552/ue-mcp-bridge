import { z } from 'zod';
import { BaseTool } from '../../core/BaseTool.js';
import type { OperationContext, OperationResult } from '../../types/index.js';
import { ueBridge } from '../../bridge/index.js';

const ConfigureInputSchema = z.object({
  axis_mappings: z.array(z.object({
    name: z.string(),
    scale: z.number(),
    key: z.string(),
  })).optional().describe('Axis mappings to configure'),
  action_mappings: z.array(z.object({
    name: z.string(),
    key: z.string(),
    shift: z.boolean().optional(),
    ctrl: z.boolean().optional(),
    alt: z.boolean().optional(),
    cmd: z.boolean().optional(),
  })).optional().describe('Action mappings to configure'),
});

export class ConfigureInputTool extends BaseTool {
  readonly name = 'configure_input';
  readonly description = 'Configure input mappings for the project';
  readonly inputSchema = ConfigureInputSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof ConfigureInputSchema>;

      const result = await ueBridge.executeCommand({
        command: 'configure_input',
        params: validated,
        expectsResult: false,
      });

      if (!result.success) {
        return this.createErrorResult('CONFIGURE_INPUT_FAILED', result.error ?? 'Failed to configure input');
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

const CreateGameModeSchema = z.object({
  game_mode_name: z.string().describe('Name for the game mode'),
  base_class: z.string().optional().default('/Script/Engine.GameModeBase').describe('Base class for the game mode'),
  pawn_class: z.string().optional().describe('Pawn class to use'),
  player_controller_class: z.string().optional().describe('Player controller class'),
  hud_class: z.string().optional().describe('HUD class'),
  save_path: z.string().optional().describe('Path to save the game mode'),
});

export class CreateGameModeTool extends BaseTool {
  readonly name = 'create_game_mode';
  readonly description = 'Create a new game mode blueprint';
  readonly inputSchema = CreateGameModeSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<{ game_mode_path: string }>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof CreateGameModeSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'create_game_mode',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('CREATE_GAME_MODE_FAILED', result.error ?? 'Failed to create game mode');
      }

      return this.createSuccessResult(result.result as { game_mode_path: string });
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const SetProjectSettingsSchema = z.object({
  settings: z.record(z.unknown()).describe('Project settings to apply'),
  category: z.enum(['General', 'Rendering', 'Physics', 'Audio', 'Input', 'Game', 'Network']).describe('Settings category'),
});

export class SetProjectSettingsTool extends BaseTool {
  readonly name = 'set_project_settings';
  readonly description = 'Modify project settings';
  readonly inputSchema = SetProjectSettingsSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof SetProjectSettingsSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'set_project_settings',
        params: validated,
        expectsResult: false,
      });

      if (!result.success) {
        return this.createErrorResult('SET_SETTINGS_FAILED', result.error ?? 'Failed to set project settings');
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

const CreateFolderSchema = z.object({
  folder_path: z.string().describe('Path for the new folder'),
  parent_path: z.string().optional().describe('Parent folder path'),
});

export class CreateFolderTool extends BaseTool {
  readonly name = 'create_folder';
  readonly description = 'Create a new folder in the content browser';
  readonly inputSchema = CreateFolderSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof CreateFolderSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'create_folder',
        params: validated,
        expectsResult: false,
      });

      if (!result.success) {
        return this.createErrorResult('CREATE_FOLDER_FAILED', result.error ?? 'Failed to create folder');
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

const GetProjectInfoSchema = z.object({});

export class GetProjectInfoTool extends BaseTool {
  readonly name = 'get_project_info';
  readonly description = 'Get information about the current project and current level';
  readonly inputSchema = GetProjectInfoSchema;

  async execute(_params: unknown, _context: OperationContext): Promise<OperationResult<{
    project_name: string;
    project_path: string;
    engine_version: string;
    default_game_mode: string;
    current_level: {
      level_name: string;
      level_path: string;
    };
  }>> {
    try {
      const projectResult = await ueBridge.executeCommand({
        command: 'get_project_info',
        params: {},
        expectsResult: true,
      });

      const levelResult = await ueBridge.executeCommand({
        command: 'get_current_level',
        params: {},
        expectsResult: true,
      });

      if (!projectResult.success) {
        return this.createErrorResult('GET_INFO_FAILED', projectResult.error ?? 'Failed to get project info');
      }

      const projectInfo = projectResult.result as {
        project_name: string;
        project_path: string;
        engine_version: string;
        default_game_mode: string;
      };

      const levelInfo = levelResult.success ? levelResult.result as { level_name: string; level_path: string } : { level_name: '', level_path: '' };

      return this.createSuccessResult({
        ...projectInfo,
        current_level: levelInfo,
      });
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}
