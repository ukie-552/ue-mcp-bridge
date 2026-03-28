import { z } from 'zod';
import { BaseTool } from '../../core/BaseTool.js';
import type { OperationContext, OperationResult, ActorInfo, Transform } from '../../types/index.js';
import { ueBridge } from '../../bridge/index.js';

function safeCast<T>(value: unknown): T {
  return value as T;
}

const SpawnActorSchema = z.object({
  actor_class: z.string().describe('Path to the actor class blueprint'),
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
  name: z.string().optional().describe('Name for the spawned actor'),
  attach_to: z.string().optional().describe('Actor ID to attach to'),
});

export class SpawnActorTool extends BaseTool<ActorInfo> {
  readonly name = 'spawn_actor';
  readonly description = 'Spawn a new actor in the current level';
  readonly inputSchema = SpawnActorSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<ActorInfo>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof SpawnActorSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'spawn_actor',
        params: {
          actor_class: validated.actor_class,
          location: validated.location ?? { x: 0, y: 0, z: 0 },
          rotation: validated.rotation ?? { pitch: 0, yaw: 0, roll: 0 },
          name: validated.name,
          attach_to: validated.attach_to,
        },
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('SPAWN_FAILED', result.error ?? 'Failed to spawn actor');
      }

      const actorInfo = result.result as ActorInfo;
      return this.createSuccessResult(actorInfo);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const MoveActorSchema = z.object({
  actor_id: z.string().describe('ID of the actor to move'),
  location: z.object({
    x: z.number(),
    y: z.number(),
    z: z.number(),
  }).describe('New location'),
  relative: z.boolean().optional().default(false).describe('Move relative to current position'),
});

export class MoveActorTool extends BaseTool<Transform> {
  readonly name = 'move_actor';
  readonly description = 'Move an actor to a new location';
  readonly inputSchema = MoveActorSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<Transform>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof MoveActorSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'move_actor',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('MOVE_FAILED', result.error ?? 'Failed to move actor');
      }

      const transform: Transform = safeCast<Transform>(result.result);
      return this.createSuccessResult(transform);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const RotateActorSchema = z.object({
  actor_id: z.string().describe('ID of the actor to rotate'),
  rotation: z.object({
    pitch: z.number(),
    yaw: z.number(),
    roll: z.number(),
  }).describe('New rotation'),
  relative: z.boolean().optional().default(false).describe('Rotate relative to current rotation'),
});

export class RotateActorTool extends BaseTool<Transform> {
  readonly name = 'rotate_actor';
  readonly description = 'Rotate an actor to a new orientation';
  readonly inputSchema = RotateActorSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<Transform>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof RotateActorSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'rotate_actor',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('ROTATE_FAILED', result.error ?? 'Failed to rotate actor');
      }

      const transform: Transform = safeCast<Transform>(result.result);
      return this.createSuccessResult(transform);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const ScaleActorSchema = z.object({
  actor_id: z.string().describe('ID of the actor to scale'),
  scale: z.object({
    x: z.number(),
    y: z.number(),
    z: z.number(),
  }).describe('New scale'),
  uniform: z.boolean().optional().default(false).describe('Apply uniform scale using X value'),
});

export class ScaleActorTool extends BaseTool<Transform> {
  readonly name = 'scale_actor';
  readonly description = 'Scale an actor to a new size';
  readonly inputSchema = ScaleActorSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<Transform>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof ScaleActorSchema>;
      
      if (validated.uniform) {
        validated.scale = { x: validated.scale.x, y: validated.scale.x, z: validated.scale.x };
      }

      const result = await ueBridge.executeCommand({
        command: 'scale_actor',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('SCALE_FAILED', result.error ?? 'Failed to scale actor');
      }

      const transform: Transform = safeCast<Transform>(result.result);
      return this.createSuccessResult(transform);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const DeleteActorSchema = z.object({
  actor_id: z.string().describe('ID of the actor to delete'),
  force: z.boolean().optional().default(false).describe('Force delete even if referenced'),
});

export class DeleteActorTool extends BaseTool {
  readonly name = 'delete_actor';
  readonly description = 'Delete an actor from the level';
  readonly inputSchema = DeleteActorSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof DeleteActorSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'delete_actor',
        params: validated,
        expectsResult: false,
      });

      if (!result.success) {
        return this.createErrorResult('DELETE_FAILED', result.error ?? 'Failed to delete actor');
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

const GetSceneActorsSchema = z.object({
  filter: z.object({
    class_name: z.string().optional(),
    tag: z.string().optional(),
    name_contains: z.string().optional(),
  }).optional().describe('Filter criteria'),
  include_hidden: z.boolean().optional().default(false).describe('Include hidden actors'),
});

export class GetSceneActorsTool extends BaseTool<ActorInfo[]> {
  readonly name = 'get_scene_actors';
  readonly description = 'Get all actors in the current level';
  readonly inputSchema = GetSceneActorsSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<ActorInfo[]>> {
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

      const actors = result.result as ActorInfo[];
      return this.createSuccessResult(actors);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const GetActorInfoSchema = z.object({
  actor_id: z.string().describe('ID of the actor to get info for'),
});

export class GetActorInfoTool extends BaseTool<ActorInfo> {
  readonly name = 'get_actor_info';
  readonly description = 'Get detailed information about a specific actor';
  readonly inputSchema = GetActorInfoSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<ActorInfo>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetActorInfoSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'get_actor_info',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_INFO_FAILED', result.error ?? 'Failed to get actor info');
      }

      const actorInfo = result.result as ActorInfo;
      return this.createSuccessResult(actorInfo);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const SetActorPropertySchema = z.object({
  actor_id: z.string().describe('ID of the actor'),
  property_name: z.string().describe('Name of the property to set'),
  property_value: z.union([z.string(), z.number(), z.boolean(), z.record(z.unknown())]).describe('New value for the property'),
});

export class SetActorPropertyTool extends BaseTool {
  readonly name = 'set_actor_property';
  readonly description = 'Set a property value on an actor';
  readonly inputSchema = SetActorPropertySchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof SetActorPropertySchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'set_actor_property',
        params: validated,
        expectsResult: false,
      });

      if (!result.success) {
        return this.createErrorResult('SET_PROPERTY_FAILED', result.error ?? 'Failed to set actor property');
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

const DuplicateActorSchema = z.object({
  actor_id: z.string().describe('ID of the actor to duplicate'),
  location_offset: z.object({
    x: z.number(),
    y: z.number(),
    z: z.number(),
  }).optional().describe('Offset from original location'),
  new_name: z.string().optional().describe('Name for the duplicated actor'),
});

export class DuplicateActorTool extends BaseTool<ActorInfo> {
  readonly name = 'duplicate_actor';
  readonly description = 'Duplicate an actor in the level';
  readonly inputSchema = DuplicateActorSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<ActorInfo>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof DuplicateActorSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'duplicate_actor',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('DUPLICATE_FAILED', result.error ?? 'Failed to duplicate actor');
      }

      const actorInfo = result.result as ActorInfo;
      return this.createSuccessResult(actorInfo);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}
