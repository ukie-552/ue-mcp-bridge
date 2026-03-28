import { z } from 'zod';
import { BaseTool } from '../../core/BaseTool.js';
import type { OperationContext, OperationResult } from '../../types/index.js';
import { ueBridge } from '../../bridge/index.js';

const CreateAnimBlueprintSchema = z.object({
  package_path: z.string().describe('Path where the anim blueprint will be created (e.g., /Game/Animations)'),
  anim_blueprint_name: z.string().describe('Name of the anim blueprint'),
  target_skeleton: z.string().describe('Path to the target skeleton'),
  parent_class: z.string().optional().describe('Path to parent anim instance class'),
});

export class CreateAnimBlueprintTool extends BaseTool {
  readonly name = 'create_anim_blueprint';
  readonly description = 'Create a new animation blueprint';
  readonly inputSchema = CreateAnimBlueprintSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof CreateAnimBlueprintSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'create_anim_blueprint',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('CREATE_FAILED', result.error ?? 'Failed to create anim blueprint');
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

const CreateStateMachineSchema = z.object({
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
  state_machine_name: z.string().describe('Name for the state machine'),
  graph_name: z.string().optional().describe('Name of the graph to add to (defaults to AnimGraph)'),
  position: z.object({
    x: z.number(),
    y: z.number(),
  }).optional().describe('Position in the graph'),
});

export class CreateStateMachineTool extends BaseTool {
  readonly name = 'create_state_machine';
  readonly description = 'Create a new state machine in an animation blueprint';
  readonly inputSchema = CreateStateMachineSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof CreateStateMachineSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'create_state_machine',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('CREATE_FAILED', result.error ?? 'Failed to create state machine');
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

const AddAnimStateSchema = z.object({
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
  state_machine_name: z.string().describe('Name of the state machine'),
  state_name: z.string().describe('Name for the new state'),
  position: z.object({
    x: z.number(),
    y: z.number(),
  }).optional().describe('Position in the state machine graph'),
  is_entry_state: z.boolean().optional().default(false).describe('Whether this is the entry state'),
});

export class AddAnimStateTool extends BaseTool {
  readonly name = 'add_anim_state';
  readonly description = 'Add a state to a state machine';
  readonly inputSchema = AddAnimStateSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof AddAnimStateSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'add_anim_state',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('ADD_STATE_FAILED', result.error ?? 'Failed to add anim state');
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

const AddStateTransitionSchema = z.object({
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
  state_machine_name: z.string().describe('Name of the state machine'),
  from_state: z.string().describe('Name of the source state'),
  to_state: z.string().describe('Name of the target state'),
  transition_rule: z.string().optional().describe('Transition rule expression'),
  automatic: z.boolean().optional().default(false).describe('Whether this is an automatic transition'),
});

export class AddStateTransitionTool extends BaseTool {
  readonly name = 'add_state_transition';
  readonly description = 'Add a transition between two states';
  readonly inputSchema = AddStateTransitionSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof AddStateTransitionSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'add_state_transition',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('ADD_TRANSITION_FAILED', result.error ?? 'Failed to add state transition');
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

const SetStateAnimationSchema = z.object({
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
  state_machine_name: z.string().describe('Name of the state machine'),
  state_name: z.string().describe('Name of the state'),
  animation_path: z.string().describe('Path to the animation asset (AnimSequence or BlendSpace)'),
  node_type: z.enum(['SequencePlayer', 'BlendSpacePlayer']).optional().describe('Type of animation node'),
  position: z.object({
    x: z.number(),
    y: z.number(),
  }).optional().describe('Position in the state graph'),
  play_rate: z.number().optional().default(1.0).describe('Animation play rate'),
  loop: z.boolean().optional().default(true).describe('Whether the animation loops'),
});

export class SetStateAnimationTool extends BaseTool {
  readonly name = 'set_state_animation';
  readonly description = 'Set the animation for a state';
  readonly inputSchema = SetStateAnimationSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof SetStateAnimationSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'set_state_animation',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('SET_ANIMATION_FAILED', result.error ?? 'Failed to set state animation');
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

const GetStateMachineInfoSchema = z.object({
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
  state_machine_name: z.string().optional().describe('Name of specific state machine (optional, returns all if not specified)'),
});

export class GetStateMachineInfoTool extends BaseTool {
  readonly name = 'get_state_machine_info';
  readonly description = 'Get information about state machines in an animation blueprint';
  readonly inputSchema = GetStateMachineInfoSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetStateMachineInfoSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'get_state_machine_info',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_INFO_FAILED', result.error ?? 'Failed to get state machine info');
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

const PlayAnimationSchema = z.object({
  animation_path: z.string().describe('Path to the animation asset'),
  loop: z.boolean().optional().default(false).describe('Whether to loop the animation'),
  play_rate: z.number().optional().default(1.0).describe('Animation play rate'),
});

export class PlayAnimationTool extends BaseTool {
  readonly name = 'play_animation';
  readonly description = 'Play an animation in the editor (for preview)';
  readonly inputSchema = PlayAnimationSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof PlayAnimationSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'play_animation',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('PLAY_FAILED', result.error ?? 'Failed to play animation');
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

export const animationTools: BaseTool<unknown>[] = [
  new CreateAnimBlueprintTool(),
  new CreateStateMachineTool(),
  new AddAnimStateTool(),
  new AddStateTransitionTool(),
  new SetStateAnimationTool(),
  new GetStateMachineInfoTool(),
  new GetAnimBlueprintGraphsTool(),
  new PlayAnimationTool(),
];
