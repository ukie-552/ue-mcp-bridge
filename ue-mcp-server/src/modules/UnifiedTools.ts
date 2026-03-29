import { z } from 'zod';
import { BaseTool } from '../core/BaseTool.js';
import type { OperationContext, OperationResult } from '../types/index.js';
import { ueBridge } from '../bridge/index.js';
import fs from 'fs/promises';
import path from 'path';

const SceneOperationSchema = z.object({
  action: z.enum(['spawn', 'move', 'rotate', 'scale', 'delete', 'duplicate', 'select', 'get_info', 'get_components', 'set_property']).describe('Scene operation to perform'),
  actor_id: z.string().optional().describe('Actor ID (required for most operations except spawn)'),
  actor_class: z.string().optional().describe('Actor class path (required for spawn)'),
  actor_name: z.string().optional().describe('Actor name'),
  location: z.object({ x: z.number(), y: z.number(), z: z.number() }).optional().describe('Location for spawn/move'),
  rotation: z.object({ pitch: z.number(), yaw: z.number(), roll: z.number() }).optional().describe('Rotation for spawn/rotate'),
  scale: z.object({ x: z.number(), y: z.number(), z: z.number() }).optional().describe('Scale for scale operation'),
  property_name: z.string().optional().describe('Property name (for set_property)'),
  property_value: z.any().optional().describe('Property value (for set_property)'),
  include_properties: z.boolean().optional().describe('Include properties (for get_components)'),
});

export class SceneOperationTool extends BaseTool {
  readonly name = 'scene_operation';
  readonly description = 'Perform various scene operations: spawn, move, rotate, scale, delete, duplicate, select actors';
  readonly inputSchema = SceneOperationSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof SceneOperationSchema>;
      
      const commandMap: Record<string, string> = {
        spawn: 'spawn_actor',
        move: 'move_actor',
        rotate: 'rotate_actor',
        scale: 'scale_actor',
        delete: 'delete_actor',
        duplicate: 'duplicate_actor',
        select: 'select_actors',
        get_info: 'get_actor_info',
        get_components: 'get_actor_components',
        set_property: 'set_actor_property',
      };

      const command = commandMap[validated.action];
      if (!command) {
        return this.createErrorResult('INVALID_ACTION', `Unknown action: ${validated.action}`);
      }

      const result = await ueBridge.executeCommand({
        command,
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('SCENE_OPERATION_FAILED', result.error ?? 'Scene operation failed');
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

const AssetOperationSchema = z.object({
  action: z.enum(['create', 'rename', 'move', 'delete', 'duplicate', 'find', 'get_references']).describe('Asset operation to perform'),
  asset_type: z.enum(['Blueprint', 'Material', 'Widget', 'AnimationBlueprint', 'Texture', 'Sound', 'DataTable']).optional().describe('Asset type (for create)'),
  asset_name: z.string().optional().describe('Asset name'),
  asset_path: z.string().optional().describe('Current asset path'),
  new_path: z.string().optional().describe('New path (for move)'),
  new_name: z.string().optional().describe('New name (for rename/duplicate)'),
  save_path: z.string().optional().describe('Save path (for create)'),
  parent_class: z.string().optional().describe('Parent class (for Blueprint)'),
  search_path: z.string().optional().describe('Search path (for find)'),
  filter: z.object({
    class_name: z.string().optional(),
    name_contains: z.string().optional(),
    tags: z.array(z.string()).optional(),
  }).optional().describe('Filter criteria (for find)'),
});

export class AssetOperationTool extends BaseTool {
  readonly name = 'asset_operation';
  readonly description = 'Perform asset operations: create, rename, move, delete, duplicate, find assets';
  readonly inputSchema = AssetOperationSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof AssetOperationSchema>;
      
      const commandMap: Record<string, string> = {
        create: 'create_asset',
        rename: 'rename_asset',
        move: 'move_asset',
        delete: 'delete_asset',
        duplicate: 'duplicate_asset',
        find: 'find_assets',
        get_references: 'get_asset_references',
      };

      const command = commandMap[validated.action];
      if (!command) {
        return this.createErrorResult('INVALID_ACTION', `Unknown action: ${validated.action}`);
      }

      const result = await ueBridge.executeCommand({
        command,
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('ASSET_OPERATION_FAILED', result.error ?? 'Asset operation failed');
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

const BlueprintOperationSchema = z.object({
  action: z.enum(['create', 'get_nodes', 'create_node', 'delete_node', 'move_node', 'connect_pins', 'disconnect_pins', 'get_functions', 'create_variable', 'set_variable', 'get_variables']).describe('Blueprint operation'),
  blueprint_path: z.string().describe('Path to the blueprint'),
  node_type: z.string().optional().describe('Node type (for create_node)'),
  position: z.object({ x: z.number(), y: z.number() }).optional().describe('Node position'),
  node_id: z.string().optional().describe('Node ID (for delete/move)'),
  source_node: z.string().optional().describe('Source node (for connect)'),
  source_output: z.string().optional().describe('Source output pin'),
  target_node: z.string().optional().describe('Target node (for connect)'),
  target_input: z.string().optional().describe('Target input pin'),
  variable_name: z.string().optional().describe('Variable name'),
  variable_type: z.string().optional().describe('Variable type'),
  variable_value: z.any().optional().describe('Variable value'),
});

export class BlueprintOperationTool extends BaseTool {
  readonly name = 'blueprint_operation';
  readonly description = 'Perform blueprint operations: create nodes, connect pins, manage variables';
  readonly inputSchema = BlueprintOperationSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof BlueprintOperationSchema>;
      
      const commandMap: Record<string, string> = {
        create: 'create_blueprint',
        get_nodes: 'get_blueprint_nodes',
        create_node: 'create_blueprint_node',
        delete_node: 'delete_blueprint_node',
        move_node: 'move_blueprint_node',
        connect_pins: 'connect_blueprint_pins',
        disconnect_pins: 'disconnect_blueprint_pins',
        get_functions: 'get_blueprint_functions',
        create_variable: 'create_blueprint_variable',
        set_variable: 'set_variable_value',
        get_variables: 'get_blueprint_variables',
      };

      const command = commandMap[validated.action];
      if (!command) {
        return this.createErrorResult('INVALID_ACTION', `Unknown action: ${validated.action}`);
      }

      const result = await ueBridge.executeCommand({
        command,
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('BLUEPRINT_OPERATION_FAILED', result.error ?? 'Blueprint operation failed');
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

const MaterialOperationSchema = z.object({
  action: z.enum(['create', 'create_instance', 'create_node', 'delete_node', 'connect_pins', 'disconnect_pins', 'get_graph', 'set_node_property']).describe('Material operation'),
  material_path: z.string().describe('Path to the material'),
  material_name: z.string().optional().describe('Material name (for create)'),
  blend_mode: z.enum(['Opaque', 'Translucent', 'Masked', 'Additive']).optional().describe('Blend mode'),
  two_sided: z.boolean().optional().describe('Two-sided material'),
  instance_name: z.string().optional().describe('Instance name'),
  parent_material: z.string().optional().describe('Parent material path'),
  node_type: z.string().optional().describe('Node type'),
  node_name: z.string().optional().describe('Node name'),
  property_name: z.string().optional().describe('Property name'),
  property_value: z.record(z.unknown()).optional().describe('Property value'),
  source_node: z.string().optional().describe('Source node'),
  source_output: z.string().optional().describe('Source output pin'),
  target_node: z.string().optional().describe('Target node'),
  target_input: z.string().optional().describe('Target input pin'),
});

export class MaterialOperationTool extends BaseTool {
  readonly name = 'material_operation';
  readonly description = 'Perform material operations: create materials, nodes, connect pins';
  readonly inputSchema = MaterialOperationSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof MaterialOperationSchema>;
      
      const commandMap: Record<string, string> = {
        create: 'create_material',
        create_instance: 'create_material_instance',
        create_node: 'create_material_node',
        delete_node: 'delete_material_node',
        connect_pins: 'connect_material_pins',
        disconnect_pins: 'disconnect_material_pins',
        get_graph: 'get_material_graph',
        set_node_property: 'set_material_node_property',
      };

      const command = commandMap[validated.action];
      if (!command) {
        return this.createErrorResult('INVALID_ACTION', `Unknown action: ${validated.action}`);
      }

      const result = await ueBridge.executeCommand({
        command,
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('MATERIAL_OPERATION_FAILED', result.error ?? 'Material operation failed');
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

const WidgetOperationSchema = z.object({
  action: z.enum(['create', 'add_control', 'set_property', 'bind_event', 'set_layout', 'get_controls', 'delete_control', 'rename_control']).describe('Widget operation'),
  widget_path: z.string().describe('Path to the widget blueprint'),
  widget_name: z.string().optional().describe('Widget name (for create)'),
  control_type: z.string().optional().describe('Control type (for add_control)'),
  control_name: z.string().optional().describe('Control name'),
  property_name: z.string().optional().describe('Property name'),
  property_value: z.record(z.unknown()).optional().describe('Property value'),
  event_type: z.string().optional().describe('Event type (for bind_event)'),
  old_name: z.string().optional().describe('Old name (for rename)'),
  new_name: z.string().optional().describe('New name (for rename)'),
});

export class WidgetOperationTool extends BaseTool {
  readonly name = 'widget_operation';
  readonly description = 'Perform UMG widget operations: create widgets, add controls, set properties';
  readonly inputSchema = WidgetOperationSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof WidgetOperationSchema>;
      
      const commandMap: Record<string, string> = {
        create: 'create_widget_blueprint',
        add_control: 'add_widget_control',
        set_property: 'set_widget_property',
        bind_event: 'bind_widget_event',
        set_layout: 'set_widget_layout',
        get_controls: 'get_widget_controls',
        delete_control: 'delete_widget_control',
        rename_control: 'rename_widget_control',
      };

      const command = commandMap[validated.action];
      if (!command) {
        return this.createErrorResult('INVALID_ACTION', `Unknown action: ${validated.action}`);
      }

      const result = await ueBridge.executeCommand({
        command,
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('WIDGET_OPERATION_FAILED', result.error ?? 'Widget operation failed');
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

const SequencerOperationSchema = z.object({
  action: z.enum(['create_sequence', 'add_track', 'add_actor', 'add_keyframe', 'create_camera', 'play', 'get_info']).describe('Sequencer operation'),
  sequence_path: z.string().describe('Path to the level sequence'),
  sequence_name: z.string().optional().describe('Sequence name (for create)'),
  duration: z.number().optional().describe('Duration in seconds'),
  track_type: z.enum(['Transform', 'CameraCut', 'Audio', 'Fade', 'Float']).optional().describe('Track type'),
  actor_name: z.string().optional().describe('Actor name'),
  binding_name: z.string().optional().describe('Binding name'),
  channel: z.string().optional().describe('Channel name (for keyframe)'),
  time: z.number().optional().describe('Time in seconds (for keyframe)'),
  value: z.number().optional().describe('Keyframe value'),
  camera_name: z.string().optional().describe('Camera name'),
  start_time: z.number().optional().describe('Start time (for play)'),
  loop: z.boolean().optional().describe('Loop playback'),
});

export class SequencerOperationTool extends BaseTool {
  readonly name = 'sequencer_operation';
  readonly description = 'Perform sequencer operations: create sequences, add tracks, keyframes';
  readonly inputSchema = SequencerOperationSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof SequencerOperationSchema>;
      
      const commandMap: Record<string, string> = {
        create_sequence: 'create_level_sequence',
        add_track: 'add_sequence_track',
        add_actor: 'add_actor_to_sequence',
        add_keyframe: 'add_keyframe',
        create_camera: 'create_cine_camera',
        play: 'play_sequence',
        get_info: 'get_sequence_info',
      };

      const command = commandMap[validated.action];
      if (!command) {
        return this.createErrorResult('INVALID_ACTION', `Unknown action: ${validated.action}`);
      }

      const result = await ueBridge.executeCommand({
        command,
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('SEQUENCER_OPERATION_FAILED', result.error ?? 'Sequencer operation failed');
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

const AnimationOperationSchema = z.object({
  action: z.enum(['create_blueprint', 'create_state_machine', 'add_state', 'add_transition', 'set_animation', 'get_info', 'play']).describe('Animation operation'),
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
  anim_blueprint_name: z.string().optional().describe('Blueprint name (for create)'),
  target_skeleton: z.string().optional().describe('Target skeleton path'),
  state_machine_name: z.string().optional().describe('State machine name'),
  state_name: z.string().optional().describe('State name'),
  from_state: z.string().optional().describe('From state (for transition)'),
  to_state: z.string().optional().describe('To state (for transition)'),
  animation_path: z.string().optional().describe('Animation path'),
});

export class AnimationOperationTool extends BaseTool {
  readonly name = 'animation_operation';
  readonly description = 'Perform animation operations: create anim blueprints, state machines';
  readonly inputSchema = AnimationOperationSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof AnimationOperationSchema>;
      
      const commandMap: Record<string, string> = {
        create_blueprint: 'create_anim_blueprint',
        create_state_machine: 'create_state_machine',
        add_state: 'add_anim_state',
        add_transition: 'add_state_transition',
        set_animation: 'set_state_animation',
        get_info: 'get_state_machine_info',
        play: 'play_animation',
      };

      const command = commandMap[validated.action];
      if (!command) {
        return this.createErrorResult('INVALID_ACTION', `Unknown action: ${validated.action}`);
      }

      const result = await ueBridge.executeCommand({
        command,
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('ANIMATION_OPERATION_FAILED', result.error ?? 'Animation operation failed');
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

const NiagaraOperationSchema = z.object({
  action: z.enum(['create_system', 'add_emitter', 'add_module', 'set_parameter', 'spawn_actor', 'get_info']).describe('Niagara operation'),
  system_path: z.string().describe('Path to the Niagara system'),
  system_name: z.string().optional().describe('System name (for create)'),
  emitter_name: z.string().optional().describe('Emitter name'),
  module_name: z.string().optional().describe('Module name'),
  parameter_name: z.string().optional().describe('Parameter name'),
  parameter_type: z.enum(['Float', 'Vector', 'Color', 'Bool', 'Int']).optional().describe('Parameter type'),
  value: z.any().optional().describe('Parameter value'),
});

export class NiagaraOperationTool extends BaseTool {
  readonly name = 'niagara_operation';
  readonly description = 'Perform Niagara operations: create systems, emitters, set parameters';
  readonly inputSchema = NiagaraOperationSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof NiagaraOperationSchema>;
      
      const commandMap: Record<string, string> = {
        create_system: 'create_niagara_system',
        add_emitter: 'add_niagara_emitter',
        add_module: 'add_niagara_module',
        set_parameter: 'set_niagara_parameter',
        spawn_actor: 'spawn_niagara_actor',
        get_info: 'get_niagara_system_info',
      };

      const command = commandMap[validated.action];
      if (!command) {
        return this.createErrorResult('INVALID_ACTION', `Unknown action: ${validated.action}`);
      }

      const result = await ueBridge.executeCommand({
        command,
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('NIAGARA_OPERATION_FAILED', result.error ?? 'Niagara operation failed');
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

const ProjectOperationSchema = z.object({
  action: z.enum(['configure_input', 'create_game_mode', 'set_settings', 'create_folder', 'get_info', 'get_current_level', 'load_level', 'save_level', 'create_level', 'add_level_streaming', 'set_world_settings']).describe('Project operation'),
  settings: z.record(z.unknown()).optional().describe('Settings object'),
  category: z.enum(['General', 'Rendering', 'Physics', 'Audio', 'Input', 'Game', 'Network']).optional().describe('Settings category'),
  game_mode_name: z.string().optional().describe('Game mode name'),
  folder_path: z.string().optional().describe('Folder path'),
  level_path: z.string().optional().describe('Level path'),
  level_name: z.string().optional().describe('Level name'),
});

export class ProjectOperationTool extends BaseTool {
  readonly name = 'project_operation';
  readonly description = 'Perform project operations: configure settings, create game modes, manage levels';
  readonly inputSchema = ProjectOperationSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof ProjectOperationSchema>;
      
      const commandMap: Record<string, string> = {
        configure_input: 'configure_input',
        create_game_mode: 'create_game_mode',
        set_settings: 'set_project_settings',
        create_folder: 'create_folder',
        get_info: 'get_project_info',
        get_current_level: 'get_current_level',
        load_level: 'load_level',
        save_level: 'save_current_level',
        create_level: 'create_level',
        add_level_streaming: 'add_level_streaming',
        set_world_settings: 'set_world_settings',
      };

      const command = commandMap[validated.action];
      if (!command) {
        return this.createErrorResult('INVALID_ACTION', `Unknown action: ${validated.action}`);
      }

      if (validated.action === 'get_info') {
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

        const projectInfo = projectResult.result as any;
        const levelInfo = levelResult.success ? levelResult.result as any : { level_name: '', level_path: '' };

        return this.createSuccessResult({
          ...projectInfo,
          current_level: levelInfo,
        });
      }

      const result = await ueBridge.executeCommand({
        command,
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('PROJECT_OPERATION_FAILED', result.error ?? 'Project operation failed');
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

const DiscoverySchema = z.object({
  type: z.enum(['widget_types', 'blueprint_nodes', 'material_nodes', 'animation_nodes', 'component_types']).describe('Type of discovery'),
  category: z.string().optional().describe('Filter by category'),
  search: z.string().optional().describe('Search term'),
});

export class DiscoveryTool extends BaseTool {
  readonly name = 'discovery';
  readonly description = 'Discover available types: widgets, nodes, components';
  readonly inputSchema = DiscoverySchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof DiscoverySchema>;
      
      const commandMap: Record<string, string> = {
        widget_types: 'get_available_widget_types',
        blueprint_nodes: 'get_available_blueprint_nodes',
        material_nodes: 'get_available_material_nodes',
        animation_nodes: 'get_available_animation_nodes',
        component_types: 'get_available_component_types',
      };

      const command = commandMap[validated.type];
      if (!command) {
        return this.createErrorResult('INVALID_TYPE', `Unknown discovery type: ${validated.type}`);
      }

      const result = await ueBridge.executeCommand({
        command,
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('DISCOVERY_FAILED', result.error ?? 'Discovery failed');
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

const EditorToolSchema = z.object({
  action: z.enum(['undo', 'redo', 'get_details', 'take_screenshot', 'delete_screenshot', 'get_logs', 'execute_console', 'start_pie', 'stop_pie', 'show_notification']).describe('Editor tool action'),
  count: z.number().optional().describe('Number of undo/redo operations'),
  object_type: z.enum(['actor', 'component', 'class']).optional().describe('Object type (for get_details)'),
  object_id: z.string().optional().describe('Object ID/path (for get_details)'),
  actor_id: z.string().optional().describe('Actor ID (for component details)'),
  output_path: z.string().optional().describe('Screenshot output path'),
  width: z.number().optional().default(1920).describe('Screenshot width'),
  height: z.number().optional().default(1080).describe('Screenshot height'),
  show_ui: z.boolean().optional().default(false).describe('Show UI in screenshot'),
  file_path: z.string().optional().describe('File path (for delete_screenshot)'),
  max_count: z.number().optional().default(100).describe('Max log count (for get_logs)'),
  command: z.string().optional().describe('Console command'),
  message: z.string().optional().describe('Notification message'),
});

export class EditorTool extends BaseTool {
  readonly name = 'editor_tool';
  readonly description = 'Editor utilities: undo/redo, screenshots, logs, console commands';
  readonly inputSchema = EditorToolSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof EditorToolSchema>;
      
      const commandMap: Record<string, string> = {
        undo: 'editor_undo_redo',
        redo: 'editor_undo_redo',
        get_details: 'get_details_panel_properties',
        take_screenshot: 'take_screenshot',
        delete_screenshot: 'delete_screenshot',
        get_logs: 'get_engine_logs',
        execute_console: 'execute_console_command',
        start_pie: 'start_pie',
        stop_pie: 'stop_pie',
        show_notification: 'show_notification',
      };

      let commandParams = { ...validated };
      
      if (validated.action === 'undo') {
        commandParams.action = 'undo';
      } else if (validated.action === 'redo') {
        commandParams.action = 'redo';
      }

      if (validated.action === 'delete_screenshot' && validated.file_path) {
        const ALLOWED_EXTENSIONS = ['.png', '.jpg', '.jpeg', '.bmp', '.exr', '.hdr'];
        const absolutePath = path.resolve(validated.file_path);
        const ext = path.extname(absolutePath).toLowerCase();
        
        if (!ALLOWED_EXTENSIONS.includes(ext)) {
          return this.createErrorResult('INVALID_EXTENSION', `Invalid file extension. Allowed: ${ALLOWED_EXTENSIONS.join(', ')}`);
        }

        try {
          await fs.access(absolutePath);
        } catch {
          return this.createErrorResult('FILE_NOT_FOUND', `Screenshot file not found: ${absolutePath}`);
        }

        await fs.unlink(absolutePath);
        return this.createSuccessResult({ deleted: true, file_path: absolutePath });
      }

      const command = commandMap[validated.action];
      if (!command) {
        return this.createErrorResult('INVALID_ACTION', `Unknown action: ${validated.action}`);
      }

      const result = await ueBridge.executeCommand({
        command,
        params: commandParams,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('EDITOR_TOOL_FAILED', result.error ?? 'Editor tool failed');
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

const GetSceneActorsSchema = z.object({
  filter: z.object({
    actor_class: z.string().optional(),
    name_contains: z.string().optional(),
    tags: z.array(z.string()).optional(),
  }).optional().describe('Filter criteria'),
});

export class GetSceneActorsTool extends BaseTool {
  readonly name = 'get_scene_actors';
  readonly description = 'Get all actors in the current level';
  readonly inputSchema = GetSceneActorsSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetSceneActorsSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'get_scene_actors',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_ACTORS_FAILED', result.error ?? 'Failed to get scene actors');
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

const GetActiveBlueprintStateSchema = z.object({});

export class GetActiveBlueprintStateTool extends BaseTool {
  readonly name = 'get_active_blueprint_state';
  readonly description = 'Get the state of the currently active blueprint editor';
  readonly inputSchema = GetActiveBlueprintStateSchema;

  async execute(_params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const result = await ueBridge.executeCommand({
        command: 'get_active_blueprint_state',
        params: {},
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_STATE_FAILED', result.error ?? 'Failed to get blueprint state');
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

const GetNodeConnectionsSchema = z.object({
  blueprint_path: z.string().describe('Path to the blueprint'),
  node_id: z.string().describe('Node ID'),
});

export class GetNodeConnectionsTool extends BaseTool {
  readonly name = 'get_node_connections';
  readonly description = 'Get connections for a specific node';
  readonly inputSchema = GetNodeConnectionsSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetNodeConnectionsSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'get_node_connections',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_CONNECTIONS_FAILED', result.error ?? 'Failed to get node connections');
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

const GetNodePinsSchema = z.object({
  blueprint_path: z.string().describe('Path to the blueprint'),
  node_id: z.string().describe('Node ID'),
});

export class GetNodePinsTool extends BaseTool {
  readonly name = 'get_node_pins';
  readonly description = 'Get pins for a specific node';
  readonly inputSchema = GetNodePinsSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetNodePinsSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'get_node_pins',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_PINS_FAILED', result.error ?? 'Failed to get node pins');
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

const GetBlueprintGraphTreeSchema = z.object({
  blueprint_path: z.string().describe('Path to the blueprint'),
});

export class GetBlueprintGraphTreeTool extends BaseTool {
  readonly name = 'get_blueprint_graph_tree';
  readonly description = 'Get the graph tree structure of a blueprint';
  readonly inputSchema = GetBlueprintGraphTreeSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetBlueprintGraphTreeSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'get_blueprint_graph_tree',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_GRAPH_TREE_FAILED', result.error ?? 'Failed to get blueprint graph tree');
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

const DeleteBlueprintVariableSchema = z.object({
  blueprint_path: z.string().describe('Path to the blueprint'),
  variable_name: z.string().describe('Name of the variable to delete'),
});

export class DeleteBlueprintVariableTool extends BaseTool {
  readonly name = 'delete_blueprint_variable';
  readonly description = 'Delete a variable from a blueprint';
  readonly inputSchema = DeleteBlueprintVariableSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof DeleteBlueprintVariableSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'delete_blueprint_variable',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('DELETE_VARIABLE_FAILED', result.error ?? 'Failed to delete blueprint variable');
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

const CreateBlueprintFunctionSchema = z.object({
  blueprint_path: z.string().describe('Path to the blueprint'),
  function_name: z.string().describe('Name of the function to create'),
});

export class CreateBlueprintFunctionTool extends BaseTool {
  readonly name = 'create_blueprint_function';
  readonly description = 'Create a function in a blueprint';
  readonly inputSchema = CreateBlueprintFunctionSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof CreateBlueprintFunctionSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'create_blueprint_function',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('CREATE_FUNCTION_FAILED', result.error ?? 'Failed to create blueprint function');
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

const AddComponentSchema = z.object({
  actor_id: z.string().describe('The ID of the actor'),
  component_type: z.string().describe('Type of component to add'),
  component_name: z.string().optional().describe('Name for the component'),
});

export class AddComponentTool extends BaseTool {
  readonly name = 'add_component';
  readonly description = 'Add a component to an actor';
  readonly inputSchema = AddComponentSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof AddComponentSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'add_component',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('ADD_COMPONENT_FAILED', result.error ?? 'Failed to add component');
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

const GetAnimationAssetsSchema = z.object({
  search_path: z.string().optional().describe('Path to search in'),
  filter: z.object({
    name_contains: z.string().optional(),
  }).optional().describe('Filter criteria'),
});

export class GetAnimationAssetsTool extends BaseTool {
  readonly name = 'get_animation_assets';
  readonly description = 'Get animation assets in the project';
  readonly inputSchema = GetAnimationAssetsSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetAnimationAssetsSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'get_animation_assets',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_ASSETS_FAILED', result.error ?? 'Failed to get animation assets');
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

const GetAnimBlueprintGraphsSchema = z.object({
  blueprint_path: z.string().describe('Path to the animation blueprint'),
});

export class GetAnimBlueprintGraphsTool extends BaseTool {
  readonly name = 'get_anim_blueprint_graphs';
  readonly description = 'Get all graphs in an animation blueprint';
  readonly inputSchema = GetAnimBlueprintGraphsSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetAnimBlueprintGraphsSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'get_anim_blueprint_graphs',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_GRAPHS_FAILED', result.error ?? 'Failed to get anim blueprint graphs');
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

const EditorUIPanelOperationSchema = z.object({
  action: z.enum(['open_tab', 'close_tab', 'focus_viewport', 'toggle_sidebar', 'show_notification', 'set_window_layout', 'open_asset', 'save_all', 'compile_all']).describe('UI panel operation'),
  tab_name: z.enum(['ContentBrowser', 'Details', 'WorldOutliner', 'OutputLog', 'BlueprintEditor', 'MaterialEditor', 'AnimationEditor', 'Sequencer', 'LevelEditor']).optional().describe('Tab name (for open/close tab)'),
  asset_path: z.string().optional().describe('Asset path (for open_asset)'),
  message: z.string().optional().describe('Notification message'),
  notification_type: z.enum(['Info', 'Success', 'Warning', 'Error']).optional().default('Info').describe('Notification type'),
  duration: z.number().optional().default(5).describe('Notification duration in seconds'),
});

export class EditorUIPanelOperationTool extends BaseTool {
  readonly name = 'editor_ui_panel_operation';
  readonly description = 'Perform editor UI panel operations: open/close tabs, focus viewport, show notifications';
  readonly inputSchema = EditorUIPanelOperationSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof EditorUIPanelOperationSchema>;
      
      const commandMap: Record<string, string> = {
        open_tab: 'open_editor_tab',
        close_tab: 'close_editor_tab',
        focus_viewport: 'focus_editor_viewport',
        toggle_sidebar: 'toggle_editor_sidebar',
        show_notification: 'show_notification',
        set_window_layout: 'set_editor_window_layout',
        open_asset: 'open_asset_in_editor',
        save_all: 'save_all_assets',
        compile_all: 'compile_all_blueprints',
      };

      const command = commandMap[validated.action];
      if (!command) {
        return this.createErrorResult('INVALID_ACTION', `Unknown action: ${validated.action}`);
      }

      if (validated.action === 'show_notification') {
        const result = await ueBridge.executeCommand({
          command: 'show_notification',
          params: {
            message: validated.message,
          },
          expectsResult: true,
        });

        if (!result.success) {
          return this.createErrorResult('UI_OPERATION_FAILED', result.error ?? 'UI operation failed');
        }

        return this.createSuccessResult(result.result);
      }

      const result = await ueBridge.executeCommand({
        command,
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('UI_OPERATION_FAILED', result.error ?? 'UI operation failed');
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

const DetailsPanelOperationSchema = z.object({
  action: z.enum(['select_object', 'get_selected_object', 'get_properties', 'set_property', 'reset_property', 'get_categories', 'filter_by_category', 'toggle_advanced']).describe('Details panel operation'),
  object_type: z.enum(['actor', 'component', 'blueprint', 'asset', 'class']).optional().describe('Object type to select'),
  object_path: z.string().optional().describe('Object path or ID'),
  actor_id: z.string().optional().describe('Actor ID (for component operations)'),
  component_name: z.string().optional().describe('Component name (for component operations)'),
  property_name: z.string().optional().describe('Property name (for set_property/reset_property)'),
  property_value: z.any().optional().describe('Property value (for set_property)'),
  category: z.string().optional().describe('Category name (for filter_by_category)'),
  show_advanced: z.boolean().optional().describe('Show advanced properties (for toggle_advanced)'),
});

export class DetailsPanelOperationTool extends BaseTool {
  readonly name = 'details_panel_operation';
  readonly description = 'Operate on the Details panel: select objects, get/set properties, filter categories';
  readonly inputSchema = DetailsPanelOperationSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof DetailsPanelOperationSchema>;
      
      if (validated.action === 'set_property') {
        if (validated.object_type === 'component') {
          if (!validated.actor_id || !validated.component_name || !validated.property_name) {
            return this.createErrorResult('MISSING_PARAMS', 'Missing required parameters for component property set: actor_id, component_name, and property_name are required');
          }
          
          const result = await ueBridge.executeCommand({
            command: 'set_component_property',
            params: {
              actor_id: validated.actor_id,
              component_name: validated.component_name,
              property_name: validated.property_name,
              value: validated.property_value,
            },
            expectsResult: true,
          });

          if (!result.success) {
            return this.createErrorResult('SET_PROPERTY_FAILED', result.error ?? 'Failed to set property');
          }

          return this.createSuccessResult(result.result);
        }

        if (!validated.property_name) {
          return this.createErrorResult('MISSING_PARAMS', 'property_name is required for set_property action');
        }

        const result = await ueBridge.executeCommand({
          command: 'set_actor_property',
          params: {
            actor_id: validated.object_path,
            property_name: validated.property_name,
            property_value: validated.property_value,
          },
          expectsResult: true,
        });

        if (!result.success) {
          return this.createErrorResult('SET_PROPERTY_FAILED', result.error ?? 'Failed to set property');
        }

        return this.createSuccessResult(result.result);
      }

      const commandMap: Record<string, string> = {
        select_object: 'select_object_in_details_panel',
        get_selected_object: 'get_selected_object_in_details_panel',
        get_properties: 'get_details_panel_properties',
        reset_property: 'reset_details_panel_property',
        get_categories: 'get_details_panel_categories',
        filter_by_category: 'filter_details_panel_by_category',
        toggle_advanced: 'toggle_details_panel_advanced',
      };

      const command = commandMap[validated.action];
      if (!command) {
        return this.createErrorResult('INVALID_ACTION', `Unknown action: ${validated.action}`);
      }

      if (validated.action === 'select_object') {
        if (validated.object_type === 'component' && validated.actor_id && validated.component_name) {
          const result = await ueBridge.executeCommand({
            command: 'select_component_in_details_panel',
            params: {
              actor_id: validated.actor_id,
              component_name: validated.component_name,
            },
            expectsResult: true,
          });

          if (!result.success) {
            return this.createErrorResult('SELECT_COMPONENT_FAILED', result.error ?? 'Failed to select component');
          }

          return this.createSuccessResult(result.result);
        }

        const result = await ueBridge.executeCommand({
          command: 'select_object_in_details_panel',
          params: {
            object_type: validated.object_type,
            object_path: validated.object_path,
          },
          expectsResult: true,
        });

        if (!result.success) {
          return this.createErrorResult('SELECT_OBJECT_FAILED', result.error ?? 'Failed to select object');
        }

        return this.createSuccessResult(result.result);
      }

      if (validated.action === 'get_properties') {
        const result = await ueBridge.executeCommand({
          command: 'get_details_panel_properties',
          params: {
            object_type: validated.object_type,
            object_id: validated.object_path,
            actor_id: validated.actor_id,
            category: validated.category,
          },
          expectsResult: true,
        });

        if (!result.success) {
          return this.createErrorResult('GET_PROPERTIES_FAILED', result.error ?? 'Failed to get properties');
        }

        return this.createSuccessResult(result.result);
      }

      const result = await ueBridge.executeCommand({
        command,
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('DETAILS_PANEL_OPERATION_FAILED', result.error ?? 'Details panel operation failed');
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

const ComponentTreeOperationSchema = z.object({
  action: z.enum(['get_tree', 'select_component', 'expand_node', 'collapse_node', 'add_component', 'remove_component', 'rename_component', 'move_component', 'duplicate_component', 'get_selected_component', 'get_component_hierarchy']).describe('Component tree operation'),
  actor_id: z.string().optional().describe('Actor ID or blueprint path'),
  component_name: z.string().optional().describe('Component name'),
  new_name: z.string().optional().describe('New name (for rename)'),
  parent_component: z.string().optional().describe('Parent component (for move/add)'),
  component_type: z.string().optional().describe('Component type (for add)'),
  expand_recursive: z.boolean().optional().describe('Expand recursively'),
});

export class ComponentTreeOperationTool extends BaseTool {
  readonly name = 'component_tree_operation';
  readonly description = 'Operate on the component tree: select, expand, add, remove, rename components';
  readonly inputSchema = ComponentTreeOperationSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof ComponentTreeOperationSchema>;
      
      const commandMap: Record<string, string> = {
        get_tree: 'get_component_tree',
        select_component: 'select_component_in_tree',
        expand_node: 'expand_component_node',
        collapse_node: 'collapse_component_node',
        add_component: 'add_component',
        remove_component: 'remove_component',
        rename_component: 'rename_component',
        move_component: 'move_component',
        duplicate_component: 'duplicate_component',
        get_selected_component: 'get_selected_component_in_tree',
        get_component_hierarchy: 'get_component_hierarchy',
      };

      const command = commandMap[validated.action];
      if (!command) {
        return this.createErrorResult('INVALID_ACTION', `Unknown action: ${validated.action}`);
      }

      if (validated.action === 'get_tree' || validated.action === 'get_component_hierarchy') {
        const result = await ueBridge.executeCommand({
          command: 'get_actor_components',
          params: {
            actor_id: validated.actor_id,
            include_properties: true,
          },
          expectsResult: true,
        });

        if (!result.success) {
          return this.createErrorResult('GET_TREE_FAILED', result.error ?? 'Failed to get component tree');
        }

        return this.createSuccessResult(result.result);
      }

      if (validated.action === 'select_component') {
        const result = await ueBridge.executeCommand({
          command: 'select_component_in_details_panel',
          params: {
            actor_id: validated.actor_id,
            component_name: validated.component_name,
          },
          expectsResult: true,
        });

        if (!result.success) {
          return this.createErrorResult('SELECT_COMPONENT_FAILED', result.error ?? 'Failed to select component');
        }

        return this.createSuccessResult(result.result);
      }

      if (validated.action === 'add_component') {
        const result = await ueBridge.executeCommand({
          command: 'add_component',
          params: {
            actor_id: validated.actor_id,
            component_type: validated.component_type,
            component_name: validated.new_name,
          },
          expectsResult: true,
        });

        if (!result.success) {
          return this.createErrorResult('ADD_COMPONENT_FAILED', result.error ?? 'Failed to add component');
        }

        return this.createSuccessResult(result.result);
      }

      if (validated.action === 'remove_component') {
        const result = await ueBridge.executeCommand({
          command: 'remove_component',
          params: {
            actor_id: validated.actor_id,
            component_name: validated.component_name,
          },
          expectsResult: true,
        });

        if (!result.success) {
          return this.createErrorResult('REMOVE_COMPONENT_FAILED', result.error ?? 'Failed to remove component');
        }

        return this.createSuccessResult(result.result);
      }

      const result = await ueBridge.executeCommand({
        command,
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('COMPONENT_TREE_OPERATION_FAILED', result.error ?? 'Component tree operation failed');
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

const EditorUIElementOperationSchema = z.object({
  action: z.enum(['find_element', 'find_all_elements', 'get_element_info', 'click_element', 'set_value', 'get_value', 'hover_element', 'wait_for_element', 'is_visible', 'is_enabled', 'get_children', 'get_parent', 'scroll_to_element', 'focus_element', 'select_option', 'get_options']).describe('UI element operation'),
  element_type: z.enum(['button', 'text', 'input', 'dropdown', 'combobox', 'select', 'checkbox', 'tree_item', 'menu_item', 'tab', 'panel', 'window', 'viewport', 'slider', 'spinbox', 'color_picker', 'asset_picker', 'any']).optional().describe('Element type to find'),
  element_name: z.string().optional().describe('Element name or label text'),
  element_path: z.string().optional().describe('Element path (e.g., "Window->Panel->Button")'),
  element_id: z.string().optional().describe('Element ID'),
  parent_element: z.string().optional().describe('Parent element ID or path'),
  value: z.any().optional().describe('Value to set (for input, dropdown, checkbox)'),
  option_value: z.string().optional().describe('Option value to select (for select_option)'),
  option_index: z.number().optional().describe('Option index to select (for select_option)'),
  timeout: z.number().optional().default(5000).describe('Timeout in ms (for wait_for_element)'),
  scroll_direction: z.enum(['up', 'down', 'left', 'right']).optional().describe('Scroll direction'),
  scroll_amount: z.number().optional().describe('Scroll amount'),
  filter: z.object({
    visible: z.boolean().optional(),
    enabled: z.boolean().optional(),
    text_contains: z.string().optional(),
    class_name: z.string().optional(),
  }).optional().describe('Filter criteria'),
});

export class EditorUIElementOperationTool extends BaseTool {
  readonly name = 'editor_ui_element_operation';
  readonly description = 'Operate on any visible editor UI element: find, click, set values, get info';
  readonly inputSchema = EditorUIElementOperationSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof EditorUIElementOperationSchema>;
      
      const commandMap: Record<string, string> = {
        find_element: 'find_editor_ui_element',
        find_all_elements: 'find_all_editor_ui_elements',
        get_element_info: 'get_editor_ui_element_info',
        click_element: 'click_editor_ui_element',
        set_value: 'set_editor_ui_element_value',
        get_value: 'get_editor_ui_element_value',
        hover_element: 'hover_editor_ui_element',
        wait_for_element: 'wait_for_editor_ui_element',
        is_visible: 'is_editor_ui_element_visible',
        is_enabled: 'is_editor_ui_element_enabled',
        get_children: 'get_editor_ui_element_children',
        get_parent: 'get_editor_ui_element_parent',
        scroll_to_element: 'scroll_to_editor_ui_element',
        focus_element: 'focus_editor_ui_element',
        select_option: 'select_editor_ui_option',
        get_options: 'get_editor_ui_options',
      };

      const command = commandMap[validated.action];
      if (!command) {
        return this.createErrorResult('INVALID_ACTION', `Unknown action: ${validated.action}`);
      }

      if (validated.action === 'find_element' || validated.action === 'find_all_elements') {
        const result = await ueBridge.executeCommand({
          command,
          params: {
            element_type: validated.element_type,
            element_name: validated.element_name,
            element_path: validated.element_path,
            parent_element: validated.parent_element,
            filter: validated.filter,
          },
          expectsResult: true,
        });

        if (!result.success) {
          return this.createErrorResult('FIND_ELEMENT_FAILED', result.error ?? 'Failed to find UI element');
        }

        return this.createSuccessResult(result.result);
      }

      if (validated.action === 'click_element') {
        const result = await ueBridge.executeCommand({
          command: 'click_editor_ui_element',
          params: {
            element_id: validated.element_id,
            element_name: validated.element_name,
            element_path: validated.element_path,
          },
          expectsResult: true,
        });

        if (!result.success) {
          return this.createErrorResult('CLICK_ELEMENT_FAILED', result.error ?? 'Failed to click UI element');
        }

        return this.createSuccessResult(result.result);
      }

      if (validated.action === 'set_value') {
        const result = await ueBridge.executeCommand({
          command: 'set_editor_ui_element_value',
          params: {
            element_id: validated.element_id,
            element_name: validated.element_name,
            element_path: validated.element_path,
            value: validated.value,
          },
          expectsResult: true,
        });

        if (!result.success) {
          return this.createErrorResult('SET_VALUE_FAILED', result.error ?? 'Failed to set UI element value');
        }

        return this.createSuccessResult(result.result);
      }

      if (validated.action === 'wait_for_element') {
        const result = await ueBridge.executeCommand({
          command: 'wait_for_editor_ui_element',
          params: {
            element_type: validated.element_type,
            element_name: validated.element_name,
            element_path: validated.element_path,
            timeout: validated.timeout,
          },
          expectsResult: true,
        });

        if (!result.success) {
          return this.createErrorResult('WAIT_ELEMENT_FAILED', result.error ?? 'UI element did not appear within timeout');
        }

        return this.createSuccessResult(result.result);
      }

      if (validated.action === 'select_option') {
        const result = await ueBridge.executeCommand({
          command: 'select_editor_ui_option',
          params: {
            element_id: validated.element_id,
            element_name: validated.element_name,
            element_path: validated.element_path,
            option_value: validated.option_value,
            option_index: validated.option_index,
          },
          expectsResult: true,
        });

        if (!result.success) {
          return this.createErrorResult('SELECT_OPTION_FAILED', result.error ?? 'Failed to select option');
        }

        return this.createSuccessResult(result.result);
      }

      if (validated.action === 'get_options') {
        const result = await ueBridge.executeCommand({
          command: 'get_editor_ui_options',
          params: {
            element_id: validated.element_id,
            element_name: validated.element_name,
            element_path: validated.element_path,
          },
          expectsResult: true,
        });

        if (!result.success) {
          return this.createErrorResult('GET_OPTIONS_FAILED', result.error ?? 'Failed to get options');
        }

        return this.createSuccessResult(result.result);
      }

      const result = await ueBridge.executeCommand({
        command,
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('UI_ELEMENT_OPERATION_FAILED', result.error ?? 'UI element operation failed');
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

export const unifiedTools: BaseTool<unknown>[] = [
  new SceneOperationTool(),
  new AssetOperationTool(),
  new BlueprintOperationTool(),
  new MaterialOperationTool(),
  new WidgetOperationTool(),
  new SequencerOperationTool(),
  new AnimationOperationTool(),
  new NiagaraOperationTool(),
  new ProjectOperationTool(),
  new DiscoveryTool(),
  new EditorTool(),
  new EditorUIPanelOperationTool(),
  new EditorUIElementOperationTool(),
  new DetailsPanelOperationTool(),
  new ComponentTreeOperationTool(),
  new GetSceneActorsTool(),
  new GetActiveBlueprintStateTool(),
  new GetNodeConnectionsTool(),
  new GetNodePinsTool(),
  new GetBlueprintGraphTreeTool(),
  new DeleteBlueprintVariableTool(),
  new CreateBlueprintFunctionTool(),
  new AddComponentTool(),
  new GetAnimationAssetsTool(),
  new GetAnimBlueprintGraphsTool(),
];
