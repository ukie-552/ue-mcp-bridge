import { z } from 'zod';
import { BaseTool } from '../../core/BaseTool.js';
import type { OperationContext, OperationResult } from '../../types/index.js';
import { ueBridge } from '../../bridge/index.js';

const CreateLevelSequenceSchema = z.object({
  sequence_path: z.string().describe('Path where the sequence will be created (e.g., /Game/Cinematics)'),
  sequence_name: z.string().describe('Name of the level sequence'),
  duration: z.number().optional().default(5.0).describe('Duration in seconds'),
  frame_rate: z.number().optional().default(30).describe('Frame rate'),
});

export class CreateLevelSequenceTool extends BaseTool<unknown> {
  readonly name = 'create_level_sequence';
  readonly description = 'Create a new level sequence asset';
  readonly inputSchema = CreateLevelSequenceSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof CreateLevelSequenceSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'create_level_sequence',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('CREATE_FAILED', result.error ?? 'Failed to create level sequence');
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

const AddSequenceTrackSchema = z.object({
  sequence_path: z.string().describe('Path to the level sequence'),
  track_type: z.enum(['Transform', 'CameraCut', 'Audio', 'Fade', 'Float']).describe('Type of track to add'),
  binding_name: z.string().optional().describe('Name of the binding (actor) to add track to'),
  actor_path: z.string().optional().describe('Path to the actor'),
});

export class AddSequenceTrackTool extends BaseTool<unknown> {
  readonly name = 'add_sequence_track';
  readonly description = 'Add a track to a level sequence';
  readonly inputSchema = AddSequenceTrackSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof AddSequenceTrackSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'add_sequence_track',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('ADD_TRACK_FAILED', result.error ?? 'Failed to add sequence track');
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

const AddActorToSequenceSchema = z.object({
  sequence_path: z.string().describe('Path to the level sequence'),
  actor_name: z.string().describe('Name of the actor to add'),
  possessable: z.boolean().optional().default(true).describe('Whether to create as possessable (true) or spawnable (false)'),
  add_transform_track: z.boolean().optional().default(true).describe('Whether to automatically add a transform track'),
});

export class AddActorToSequenceTool extends BaseTool<unknown> {
  readonly name = 'add_actor_to_sequence';
  readonly description = 'Add an actor to a level sequence';
  readonly inputSchema = AddActorToSequenceSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof AddActorToSequenceSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'add_actor_to_sequence',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('ADD_ACTOR_FAILED', result.error ?? 'Failed to add actor to sequence');
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

const AddKeyframeSchema = z.object({
  sequence_path: z.string().describe('Path to the level sequence'),
  binding_name: z.string().optional().describe('Name of the binding'),
  track_type: z.enum(['Transform', 'Float']).describe('Type of track'),
  channel: z.string().describe('Channel name (e.g., LocationX, RotationYaw, ScaleZ)'),
  time: z.number().describe('Time in seconds'),
  value: z.number().describe('Value for the keyframe'),
  frame_rate: z.number().optional().default(30).describe('Frame rate'),
});

export class AddKeyframeTool extends BaseTool<unknown> {
  readonly name = 'add_keyframe';
  readonly description = 'Add a keyframe to a track';
  readonly inputSchema = AddKeyframeSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof AddKeyframeSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'add_keyframe',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('ADD_KEYFRAME_FAILED', result.error ?? 'Failed to add keyframe');
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

const CreateCineCameraSchema = z.object({
  camera_name: z.string().optional().describe('Name for the camera'),
  location: z.object({
    x: z.number(),
    y: z.number(),
    z: z.number(),
  }).optional().describe('Camera location'),
  rotation: z.object({
    pitch: z.number(),
    yaw: z.number(),
    roll: z.number(),
  }).optional().describe('Camera rotation'),
  look_at_enabled: z.boolean().optional().default(false).describe('Enable look-at tracking'),
  look_at_target: z.string().optional().describe('Target actor name for look-at'),
});

export class CreateCineCameraTool extends BaseTool<unknown> {
  readonly name = 'create_cine_camera';
  readonly description = 'Create a cinematic camera in the level';
  readonly inputSchema = CreateCineCameraSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof CreateCineCameraSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'create_cine_camera',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('CREATE_CAMERA_FAILED', result.error ?? 'Failed to create cine camera');
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

const PlaySequenceSchema = z.object({
  sequence_path: z.string().describe('Path to the level sequence'),
  start_time: z.number().optional().default(0.0).describe('Start time in seconds'),
  loop: z.boolean().optional().default(false).describe('Whether to loop the sequence'),
});

export class PlaySequenceTool extends BaseTool<unknown> {
  readonly name = 'play_sequence';
  readonly description = 'Play a level sequence in the editor';
  readonly inputSchema = PlaySequenceSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof PlaySequenceSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'play_sequence',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('PLAY_FAILED', result.error ?? 'Failed to play sequence');
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

const GetSequenceInfoSchema = z.object({
  sequence_path: z.string().describe('Path to the level sequence'),
});

export class GetSequenceInfoTool extends BaseTool<unknown> {
  readonly name = 'get_sequence_info';
  readonly description = 'Get information about a level sequence';
  readonly inputSchema = GetSequenceInfoSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetSequenceInfoSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'get_sequence_info',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_INFO_FAILED', result.error ?? 'Failed to get sequence info');
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

export const sequencerTools: BaseTool<unknown>[] = [
  new CreateLevelSequenceTool(),
  new AddSequenceTrackTool(),
  new AddActorToSequenceTool(),
  new AddKeyframeTool(),
  new CreateCineCameraTool(),
  new PlaySequenceTool(),
  new GetSequenceInfoTool(),
];
