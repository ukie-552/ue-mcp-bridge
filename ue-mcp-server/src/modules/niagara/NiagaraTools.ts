import { z } from 'zod';
import { BaseTool } from '../../core/BaseTool.js';
import type { OperationContext, OperationResult } from '../../types/index.js';
import { ueBridge } from '../../bridge/index.js';

const CreateNiagaraSystemSchema = z.object({
  system_path: z.string().describe('Path where the system will be created (e.g., /Game/VFX)'),
  system_name: z.string().describe('Name of the Niagara system'),
  template_path: z.string().optional().describe('Optional path to a template system to duplicate'),
});

export class CreateNiagaraSystemTool extends BaseTool {
  readonly name = 'create_niagara_system';
  readonly description = 'Create a new Niagara particle system';
  readonly inputSchema = CreateNiagaraSystemSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof CreateNiagaraSystemSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'create_niagara_system',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('CREATE_FAILED', result.error ?? 'Failed to create Niagara system');
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

const AddNiagaraEmitterSchema = z.object({
  system_path: z.string().describe('Path to the Niagara system'),
  emitter_name: z.string().describe('Name for the new emitter'),
  emitter_template_path: z.string().optional().describe('Optional path to an emitter template'),
  is_solo: z.boolean().optional().default(false).describe('Set emitter as solo'),
  is_enabled: z.boolean().optional().default(true).describe('Enable the emitter'),
});

export class AddNiagaraEmitterTool extends BaseTool {
  readonly name = 'add_niagara_emitter';
  readonly description = 'Add an emitter to a Niagara system';
  readonly inputSchema = AddNiagaraEmitterSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof AddNiagaraEmitterSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'add_niagara_emitter',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('ADD_EMITTER_FAILED', result.error ?? 'Failed to add emitter');
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

const AddNiagaraModuleSchema = z.object({
  system_path: z.string().describe('Path to the Niagara system'),
  emitter_name: z.string().describe('Name of the emitter'),
  module_name: z.string().describe('Name of the module to add'),
  stack_context: z.string().optional().describe('Stack context (ParticleSpawn, ParticleUpdate, etc.)'),
  insert_index: z.number().optional().describe('Index to insert the module'),
});

export class AddNiagaraModuleTool extends BaseTool {
  readonly name = 'add_niagara_module';
  readonly description = 'Add a module to a Niagara emitter';
  readonly inputSchema = AddNiagaraModuleSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof AddNiagaraModuleSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'add_niagara_module',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('ADD_MODULE_FAILED', result.error ?? 'Failed to add module');
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

const SetNiagaraParameterSchema = z.object({
  system_path: z.string().describe('Path to the Niagara system'),
  parameter_name: z.string().describe('Name of the parameter'),
  parameter_type: z.enum(['Float', 'Vector', 'Color', 'Bool', 'Int']).describe('Type of the parameter'),
  value: z.union([
    z.number(),
    z.object({ x: z.number(), y: z.number(), z: z.number() }),
    z.object({ r: z.number(), g: z.number(), b: z.number(), a: z.number().optional() }),
    z.boolean(),
  ]).describe('Value of the parameter'),
});

export class SetNiagaraParameterTool extends BaseTool {
  readonly name = 'set_niagara_parameter';
  readonly description = 'Set a parameter value in a Niagara system';
  readonly inputSchema = SetNiagaraParameterSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof SetNiagaraParameterSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'set_niagara_parameter',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('SET_PARAMETER_FAILED', result.error ?? 'Failed to set parameter');
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

const SpawnNiagaraActorSchema = z.object({
  system_path: z.string().describe('Path to the Niagara system'),
  actor_name: z.string().optional().describe('Name for the spawned actor'),
  location: z.object({
    x: z.number(),
    y: z.number(),
    z: z.number(),
  }).optional().describe('Spawn location'),
  rotation: z.object({
    pitch: z.number(),
    yaw: z.number(),
    roll: z.number(),
  }).optional().describe('Spawn rotation'),
  scale: z.object({
    x: z.number(),
    y: z.number(),
    z: z.number(),
  }).optional().describe('Actor scale'),
  auto_activate: z.boolean().optional().default(true).describe('Auto-activate the particle system'),
  auto_destroy: z.boolean().optional().default(false).describe('Auto-destroy when finished'),
});

export class SpawnNiagaraActorTool extends BaseTool {
  readonly name = 'spawn_niagara_actor';
  readonly description = 'Spawn a Niagara actor in the level';
  readonly inputSchema = SpawnNiagaraActorSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof SpawnNiagaraActorSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'spawn_niagara_actor',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('SPAWN_FAILED', result.error ?? 'Failed to spawn Niagara actor');
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

const GetNiagaraSystemInfoSchema = z.object({
  system_path: z.string().describe('Path to the Niagara system'),
});

export class GetNiagaraSystemInfoTool extends BaseTool {
  readonly name = 'get_niagara_system_info';
  readonly description = 'Get information about a Niagara system';
  readonly inputSchema = GetNiagaraSystemInfoSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetNiagaraSystemInfoSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'get_niagara_system_info',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_INFO_FAILED', result.error ?? 'Failed to get system info');
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

export const niagaraTools: BaseTool<unknown>[] = [
  new CreateNiagaraSystemTool(),
  new AddNiagaraEmitterTool(),
  new AddNiagaraModuleTool(),
  new SetNiagaraParameterTool(),
  new SpawnNiagaraActorTool(),
  new GetNiagaraSystemInfoTool(),
];
