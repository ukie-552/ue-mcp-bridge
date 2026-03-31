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

const CreateAnimNodeSchema = z.object({
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
  node_type: z.enum(['SequencePlayer', 'BlendSpacePlayer', 'StateMachine', 'LinkedAnimLayer', 'LinkedAnimGraph']).describe('Type of animation node to create'),
  graph_name: z.string().optional().describe('Name of the graph to add to (defaults to AnimGraph)'),
  position: z.object({
    x: z.number(),
    y: z.number(),
  }).optional().describe('Position in the graph'),
});

export class CreateAnimNodeTool extends BaseTool {
  readonly name = 'create_anim_node';
  readonly description = 'Create a new animation node in an animation blueprint';
  readonly inputSchema = CreateAnimNodeSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof CreateAnimNodeSchema>;

      const result = await ueBridge.executeCommand({
        command: 'create_anim_node',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('CREATE_NODE_FAILED', result.error ?? 'Failed to create anim node');
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

const GetAnimBlueprintNodesSchema = z.object({
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
  graph_name: z.string().optional().describe('Name of specific graph to query (optional, returns all graphs if not specified)'),
});

export class GetAnimBlueprintNodesTool extends BaseTool {
  readonly name = 'get_anim_blueprint_nodes';
  readonly description = 'Get all nodes in an animation blueprint';
  readonly inputSchema = GetAnimBlueprintNodesSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetAnimBlueprintNodesSchema>;

      const result = await ueBridge.executeCommand({
        command: 'get_anim_blueprint_nodes',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_NODES_FAILED', result.error ?? 'Failed to get anim blueprint nodes');
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

const DeleteAnimNodeSchema = z.object({
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
  node_id: z.string().describe('ID of the node to delete'),
});

export class DeleteAnimNodeTool extends BaseTool {
  readonly name = 'delete_anim_node';
  readonly description = 'Delete a node from an animation blueprint';
  readonly inputSchema = DeleteAnimNodeSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof DeleteAnimNodeSchema>;

      const result = await ueBridge.executeCommand({
        command: 'delete_anim_node',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('DELETE_NODE_FAILED', result.error ?? 'Failed to delete anim node');
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

const ConnectAnimNodePinsSchema = z.object({
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
  source_node_id: z.string().describe('ID of the source node'),
  source_pin_name: z.string().describe('Name of the source pin'),
  target_node_id: z.string().describe('ID of the target node'),
  target_pin_name: z.string().describe('Name of the target pin'),
});

export class ConnectAnimNodePinsTool extends BaseTool {
  readonly name = 'connect_anim_node_pins';
  readonly description = 'Connect two pins in an animation blueprint';
  readonly inputSchema = ConnectAnimNodePinsSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof ConnectAnimNodePinsSchema>;

      const result = await ueBridge.executeCommand({
        command: 'connect_anim_node_pins',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('CONNECT_PINS_FAILED', result.error ?? 'Failed to connect anim node pins');
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

const SetAnimNodePropertySchema = z.object({
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
  node_id: z.string().describe('ID of the node to modify'),
  property_name: z.string().describe('Name of the property to set'),
  property_value: z.string().describe('Value to set'),
});

export class SetAnimNodePropertyTool extends BaseTool {
  readonly name = 'set_anim_node_property';
  readonly description = 'Set a property value on an animation node';
  readonly inputSchema = SetAnimNodePropertySchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof SetAnimNodePropertySchema>;

      const result = await ueBridge.executeCommand({
        command: 'set_anim_node_property',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('SET_PROPERTY_FAILED', result.error ?? 'Failed to set anim node property');
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

const CreateAnimLayerInterfaceSchema = z.object({
  package_path: z.string().describe('Path where the interface will be created (e.g., /Game/Animations)'),
  interface_name: z.string().describe('Name of the interface'),
  input_pose_names: z.array(z.string()).optional().describe('List of input pose names'),
  output_pose_names: z.array(z.string()).optional().describe('List of output pose names'),
});

export class CreateAnimLayerInterfaceTool extends BaseTool {
  readonly name = 'create_anim_layer_interface';
  readonly description = 'Create a new anim layer interface';
  readonly inputSchema = CreateAnimLayerInterfaceSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof CreateAnimLayerInterfaceSchema>;

      const result = await ueBridge.executeCommand({
        command: 'create_anim_layer_interface',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('CREATE_INTERFACE_FAILED', result.error ?? 'Failed to create anim layer interface');
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

const GetAnimLayerInfoSchema = z.object({
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
});

export class GetAnimLayerInfoTool extends BaseTool {
  readonly name = 'get_anim_layer_info';
  readonly description = 'Get information about linked anim layers in an animation blueprint';
  readonly inputSchema = GetAnimLayerInfoSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetAnimLayerInfoSchema>;

      const result = await ueBridge.executeCommand({
        command: 'get_anim_layer_info',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_LAYER_INFO_FAILED', result.error ?? 'Failed to get anim layer info');
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

const ConfigureAnimLayerNodeSchema = z.object({
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
  node_id: z.string().describe('ID of the linked layer node to configure'),
  interface_path: z.string().optional().describe('Path to the anim layer interface'),
  layer_name: z.string().optional().describe('Name of the layer'),
  graph_reference: z.string().optional().describe('Path to the referenced anim graph (for LinkedAnimGraph)'),
});

export class ConfigureAnimLayerNodeTool extends BaseTool {
  readonly name = 'configure_anim_layer_node';
  readonly description = 'Configure a linked anim layer node';
  readonly inputSchema = ConfigureAnimLayerNodeSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof ConfigureAnimLayerNodeSchema>;

      const result = await ueBridge.executeCommand({
        command: 'configure_anim_layer_node',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('CONFIGURE_LAYER_FAILED', result.error ?? 'Failed to configure anim layer node');
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

const AddAnimLayerSchema = z.object({
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
  layer_name: z.string().describe('Name of the layer to add'),
  interface_path: z.string().describe('Path to the anim layer interface'),
});

export class AddAnimLayerTool extends BaseTool {
  readonly name = 'add_anim_layer';
  readonly description = 'Add a new anim layer to an animation blueprint';
  readonly inputSchema = AddAnimLayerSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof AddAnimLayerSchema>;

      const result = await ueBridge.executeCommand({
        command: 'add_anim_layer',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('ADD_LAYER_FAILED', result.error ?? 'Failed to add anim layer');
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

const GetAnimNodePinsSchema = z.object({
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
  node_id: z.string().describe('ID of the node to get pins for'),
});

export class GetAnimNodePinsTool extends BaseTool {
  readonly name = 'get_anim_node_pins';
  readonly description = 'Get detailed pin information for a specific animation node';
  readonly inputSchema = GetAnimNodePinsSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetAnimNodePinsSchema>;

      const result = await ueBridge.executeCommand({
        command: 'get_anim_node_pins',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_PINS_FAILED', result.error ?? 'Failed to get anim node pins');
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

const GetAnimNodeConnectionsSchema = z.object({
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
  node_id: z.string().describe('ID of the node to get connections for'),
});

export class GetAnimNodeConnectionsTool extends BaseTool {
  readonly name = 'get_anim_node_connections';
  readonly description = 'Get all connections for a specific animation node';
  readonly inputSchema = GetAnimNodeConnectionsSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetAnimNodeConnectionsSchema>;

      const result = await ueBridge.executeCommand({
        command: 'get_anim_node_connections',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_CONNECTIONS_FAILED', result.error ?? 'Failed to get anim node connections');
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

const DisconnectAnimNodePinsSchema = z.object({
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
  node_id: z.string().describe('ID of the node to disconnect'),
  pin_name: z.string().optional().describe('Specific pin name to disconnect (all pins if not specified)'),
  target_node_id: z.string().optional().describe('Target node ID for specific disconnection'),
  target_pin_name: z.string().optional().describe('Target pin name for specific disconnection'),
});

export class DisconnectAnimNodePinsTool extends BaseTool {
  readonly name = 'disconnect_anim_node_pins';
  readonly description = 'Disconnect pins on an animation node';
  readonly inputSchema = DisconnectAnimNodePinsSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof DisconnectAnimNodePinsSchema>;

      const result = await ueBridge.executeCommand({
        command: 'disconnect_anim_node_pins',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('DISCONNECT_FAILED', result.error ?? 'Failed to disconnect anim node pins');
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

const MoveAnimNodeSchema = z.object({
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
  node_id: z.string().describe('ID of the node to move'),
  position: z.object({
    x: z.number(),
    y: z.number(),
  }).describe('New position for the node'),
});

export class MoveAnimNodeTool extends BaseTool {
  readonly name = 'move_anim_node';
  readonly description = 'Move an animation node to a new position';
  readonly inputSchema = MoveAnimNodeSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof MoveAnimNodeSchema>;

      const result = await ueBridge.executeCommand({
        command: 'move_anim_node',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('MOVE_NODE_FAILED', result.error ?? 'Failed to move anim node');
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

const SetAnimNodePinValueSchema = z.object({
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
  node_id: z.string().describe('ID of the node'),
  pin_name: z.string().describe('Name of the pin to set'),
  pin_value: z.string().describe('Value to set on the pin'),
});

export class SetAnimNodePinValueTool extends BaseTool {
  readonly name = 'set_anim_node_pin_value';
  readonly description = 'Set a default value on an animation node pin';
  readonly inputSchema = SetAnimNodePinValueSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof SetAnimNodePinValueSchema>;

      const result = await ueBridge.executeCommand({
        command: 'set_anim_node_pin_value',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('SET_PIN_VALUE_FAILED', result.error ?? 'Failed to set anim node pin value');
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

const GetAnimBlueprintVariablesSchema = z.object({
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
});

export class GetAnimBlueprintVariablesTool extends BaseTool {
  readonly name = 'get_anim_blueprint_variables';
  readonly description = 'Get all variables from an animation blueprint';
  readonly inputSchema = GetAnimBlueprintVariablesSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetAnimBlueprintVariablesSchema>;

      const result = await ueBridge.executeCommand({
        command: 'get_anim_blueprint_variables',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_VARIABLES_FAILED', result.error ?? 'Failed to get anim blueprint variables');
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

const CreateAnimBlueprintVariableSchema = z.object({
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
  variable_name: z.string().describe('Name of the variable to create'),
  variable_type: z.string().optional().describe('Type of the variable (bool, int, float, string, etc.)'),
  is_array: z.boolean().optional().describe('Whether the variable is an array'),
  default_value: z.string().optional().describe('Default value for the variable'),
  category: z.string().optional().describe('Category for the variable'),
  is_exposed: z.boolean().optional().describe('Whether the variable is exposed'),
});

export class CreateAnimBlueprintVariableTool extends BaseTool {
  readonly name = 'create_anim_blueprint_variable';
  readonly description = 'Create a new variable in an animation blueprint';
  readonly inputSchema = CreateAnimBlueprintVariableSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof CreateAnimBlueprintVariableSchema>;

      const result = await ueBridge.executeCommand({
        command: 'create_anim_blueprint_variable',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('CREATE_VARIABLE_FAILED', result.error ?? 'Failed to create anim blueprint variable');
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

const DeleteAnimBlueprintVariableSchema = z.object({
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
  variable_name: z.string().describe('Name of the variable to delete'),
});

export class DeleteAnimBlueprintVariableTool extends BaseTool {
  readonly name = 'delete_anim_blueprint_variable';
  readonly description = 'Delete a variable from an animation blueprint';
  readonly inputSchema = DeleteAnimBlueprintVariableSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof DeleteAnimBlueprintVariableSchema>;

      const result = await ueBridge.executeCommand({
        command: 'delete_anim_blueprint_variable',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('DELETE_VARIABLE_FAILED', result.error ?? 'Failed to delete anim blueprint variable');
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

const GetAnimBlueprintFunctionsSchema = z.object({
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
});

export class GetAnimBlueprintFunctionsTool extends BaseTool {
  readonly name = 'get_anim_blueprint_functions';
  readonly description = 'Get all functions from an animation blueprint';
  readonly inputSchema = GetAnimBlueprintFunctionsSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetAnimBlueprintFunctionsSchema>;

      const result = await ueBridge.executeCommand({
        command: 'get_anim_blueprint_functions',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_FUNCTIONS_FAILED', result.error ?? 'Failed to get anim blueprint functions');
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

const CreateAnimBlueprintFunctionSchema = z.object({
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
  function_name: z.string().describe('Name of the function to create'),
  description: z.string().optional().describe('Description of the function'),
  is_macro: z.boolean().optional().describe('Whether to create as a macro instead of function'),
});

export class CreateAnimBlueprintFunctionTool extends BaseTool {
  readonly name = 'create_anim_blueprint_function';
  readonly description = 'Create a new function in an animation blueprint';
  readonly inputSchema = CreateAnimBlueprintFunctionSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof CreateAnimBlueprintFunctionSchema>;

      const result = await ueBridge.executeCommand({
        command: 'create_anim_blueprint_function',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('CREATE_FUNCTION_FAILED', result.error ?? 'Failed to create anim blueprint function');
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

const GetAnimLayerGraphTreeSchema = z.object({
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
  include_pins: z.boolean().optional().describe('Include pin information'),
  include_connections: z.boolean().optional().describe('Include connection information'),
});

export class GetAnimLayerGraphTreeTool extends BaseTool {
  readonly name = 'get_anim_layer_graph_tree';
  readonly description = 'Get the complete graph tree structure of an animation blueprint including all layers';
  readonly inputSchema = GetAnimLayerGraphTreeSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetAnimLayerGraphTreeSchema>;

      const result = await ueBridge.executeCommand({
        command: 'get_anim_layer_graph_tree',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_GRAPH_TREE_FAILED', result.error ?? 'Failed to get anim layer graph tree');
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

const CompileAnimBlueprintSchema = z.object({
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
});

export class CompileAnimBlueprintTool extends BaseTool {
  readonly name = 'compile_anim_blueprint';
  readonly description = 'Compile an animation blueprint to check for errors';
  readonly inputSchema = CompileAnimBlueprintSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof CompileAnimBlueprintSchema>;

      const result = await ueBridge.executeCommand({
        command: 'compile_anim_blueprint',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('COMPILE_FAILED', result.error ?? 'Failed to compile anim blueprint');
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

const ValidateAnimBlueprintSchema = z.object({
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
  check_connections: z.boolean().optional().describe('Check for unconnected pins'),
  check_variables: z.boolean().optional().describe('Check for variable issues'),
  check_state_machines: z.boolean().optional().describe('Check for state machine issues'),
});

export class ValidateAnimBlueprintTool extends BaseTool {
  readonly name = 'validate_anim_blueprint';
  readonly description = 'Validate an animation blueprint for common issues';
  readonly inputSchema = ValidateAnimBlueprintSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof ValidateAnimBlueprintSchema>;

      const result = await ueBridge.executeCommand({
        command: 'validate_anim_blueprint',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('VALIDATE_FAILED', result.error ?? 'Failed to validate anim blueprint');
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

const SelectAnimNodeSchema = z.object({
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
  node_id: z.string().describe('ID of the node to select'),
  append: z.boolean().optional().describe('Whether to append to current selection'),
});

export class SelectAnimNodeTool extends BaseTool {
  readonly name = 'select_anim_node';
  readonly description = 'Select a specific node in an animation blueprint graph';
  readonly inputSchema = SelectAnimNodeSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof SelectAnimNodeSchema>;

      const result = await ueBridge.executeCommand({
        command: 'select_anim_node',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('SELECT_NODE_FAILED', result.error ?? 'Failed to select anim node');
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

const GetAnimNodeDetailsSchema = z.object({
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
  node_id: z.string().describe('ID of the node to get details for'),
  category: z.string().optional().describe('Filter by category name'),
});

export interface AnimNodeProperty {
  name: string;
  display_name: string;
  category: string;
  type: string;
  tooltip: string;
  is_read_only: boolean;
  is_array: boolean;
  is_struct: boolean;
  is_enum: boolean;
  is_object: boolean;
  current_value: string;
  default_value?: string;
  metadata?: {
    ui_min?: string;
    ui_max?: string;
    clamp_min?: string;
    clamp_max?: string;
  };
}

export interface AnimNodePin {
  pin_id: string;
  pin_name: string;
  direction: 'Input' | 'Output';
  pin_category: string;
  pin_sub_category: string;
  default_value?: string;
  linked_count: number;
  linked_to?: Array<{
    node_id: string;
    node_name: string;
    pin_name: string;
  }>;
}

export interface AnimNodeDetailsResult {
  anim_blueprint_path: string;
  node_id: string;
  node_name: string;
  node_display_name: string;
  node_type: string;
  graph_name: string;
  anim_node_class?: string;
  properties: AnimNodeProperty[];
  categories: string[];
  property_count: number;
  pins: AnimNodePin[];
  pin_count: number;
}

export class GetAnimNodeDetailsTool extends BaseTool {
  readonly name = 'get_anim_node_details';
  readonly description = 'Get detailed properties and pins information for a selected animation node, similar to the Details Panel in the editor';
  readonly inputSchema = GetAnimNodeDetailsSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<AnimNodeDetailsResult>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetAnimNodeDetailsSchema>;

      const result = await ueBridge.executeCommand({
        command: 'get_anim_node_details',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_DETAILS_FAILED', result.error ?? 'Failed to get anim node details');
      }

      return this.createSuccessResult(result.result as AnimNodeDetailsResult);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const SetAnimNodeDetailsPropertySchema = z.object({
  anim_blueprint_path: z.string().describe('Path to the animation blueprint'),
  node_id: z.string().describe('ID of the node'),
  property_name: z.string().describe('Name of the property to set'),
  property_value: z.string().describe('Value to set (for bool: true/false, for numbers: numeric string, for enums: enum value name)'),
});

export class SetAnimNodeDetailsPropertyTool extends BaseTool {
  readonly name = 'set_anim_node_details_property';
  readonly description = 'Set a property value on an animation node via its details panel interface';
  readonly inputSchema = SetAnimNodeDetailsPropertySchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof SetAnimNodeDetailsPropertySchema>;

      const result = await ueBridge.executeCommand({
        command: 'set_anim_node_details_property',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('SET_PROPERTY_FAILED', result.error ?? 'Failed to set anim node property');
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
  new CreateAnimNodeTool(),
  new GetAnimBlueprintNodesTool(),
  new DeleteAnimNodeTool(),
  new ConnectAnimNodePinsTool(),
  new SetAnimNodePropertyTool(),
  new CreateAnimLayerInterfaceTool(),
  new GetAnimLayerInfoTool(),
  new ConfigureAnimLayerNodeTool(),
  new AddAnimLayerTool(),
  new GetAnimNodePinsTool(),
  new GetAnimNodeConnectionsTool(),
  new DisconnectAnimNodePinsTool(),
  new MoveAnimNodeTool(),
  new SetAnimNodePinValueTool(),
  new GetAnimBlueprintVariablesTool(),
  new CreateAnimBlueprintVariableTool(),
  new DeleteAnimBlueprintVariableTool(),
  new GetAnimBlueprintFunctionsTool(),
  new CreateAnimBlueprintFunctionTool(),
  new GetAnimLayerGraphTreeTool(),
  new CompileAnimBlueprintTool(),
  new ValidateAnimBlueprintTool(),
  new SelectAnimNodeTool(),
  new GetAnimNodeDetailsTool(),
  new SetAnimNodeDetailsPropertyTool(),
];
