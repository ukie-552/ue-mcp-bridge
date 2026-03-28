import { z } from 'zod';
import { BaseTool } from '../../core/BaseTool.js';
import type { OperationContext, OperationResult, ConnectionInfo, PinInfo } from '../../types/index.js';
import { ueBridge } from '../../bridge/index.js';

const ConnectBlueprintPinsSchema = z.object({
  blueprint_path: z.string().describe('Full path to the blueprint asset'),
  source_node_id: z.string().describe('ID of the source node'),
  source_pin_name: z.string().describe('Name of the source pin'),
  target_node_id: z.string().describe('ID of the target node'),
  target_pin_name: z.string().describe('Name of the target pin'),
  validate_types: z.boolean().optional().default(true).describe('Validate pin type compatibility'),
});

export class ConnectBlueprintPinsTool extends BaseTool {
  readonly name = 'connect_blueprint_pins';
  readonly description = 'Connect two pins in a blueprint graph to create a wire';
  readonly inputSchema = ConnectBlueprintPinsSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<ConnectionInfo>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof ConnectBlueprintPinsSchema>;
      
      if (validated.validate_types) {
        const validationResult = await this.validatePinCompatibility(validated);
        if (!validationResult.success) {
          return this.createErrorResult<ConnectionInfo>(
            validationResult.error?.code ?? 'VALIDATION_FAILED',
            validationResult.error?.message ?? 'Pin validation failed'
          );
        }
      }

      const result = await ueBridge.executeCommand({
        command: 'connect_blueprint_pins',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('CONNECT_FAILED', result.error ?? 'Failed to connect pins');
      }

      const connectionInfo = result.result as ConnectionInfo;
      return this.createSuccessResult(connectionInfo);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }

  private async validatePinCompatibility(
    params: z.infer<typeof ConnectBlueprintPinsSchema>
  ): Promise<OperationResult<unknown>> {
    const pinsResult = await ueBridge.executeCommand({
      command: 'get_node_pins',
      params: {
        blueprint_path: params.blueprint_path,
        node_ids: [params.source_node_id, params.target_node_id],
      },
      expectsResult: true,
    });

    if (!pinsResult.success) {
      return this.createErrorResult('GET_PINS_FAILED', 'Failed to get pin information');
    }

    const pins = pinsResult.result as PinInfo[];
    const sourcePin = pins.find(
      p => p.owningNodeId === params.source_node_id && p.pinName === params.source_pin_name
    );
    const targetPin = pins.find(
      p => p.owningNodeId === params.target_node_id && p.pinName === params.target_pin_name
    );

    if (!sourcePin || !targetPin) {
      return this.createErrorResult('PIN_NOT_FOUND', 'One or both pins not found');
    }

    if (sourcePin.isInput === targetPin.isInput) {
      return this.createErrorResult(
        'INVALID_CONNECTION',
        'Cannot connect two input pins or two output pins'
      );
    }

    if (!this.areTypesCompatible(sourcePin.pinType, targetPin.pinType)) {
      return this.createErrorResult(
        'TYPE_MISMATCH',
        `Pin types are not compatible: ${sourcePin.pinType.category} -> ${targetPin.pinType.category}`
      );
    }

    return this.createSuccessResult();
  }

  private areTypesCompatible(
    sourceType: PinInfo['pinType'],
    targetType: PinInfo['pinType']
  ): boolean {
    if (sourceType.category === 'exec' || targetType.category === 'exec') {
      return sourceType.category === targetType.category;
    }

    const compatibleTypes: Record<string, Set<string>> = {
      bool: new Set(['bool']),
      int: new Set(['int', 'float']),
      float: new Set(['int', 'float']),
      string: new Set(['string', 'name', 'text']),
      object: new Set(['object']),
    };

    const sourceCompatible = compatibleTypes[sourceType.category];
    return sourceCompatible?.has(targetType.category) ?? false;
  }
}

const DisconnectBlueprintPinsSchema = z.object({
  blueprint_path: z.string().describe('Full path to the blueprint asset'),
  connection_id: z.string().optional().describe('ID of the connection to disconnect'),
  source_node_id: z.string().optional().describe('Source node ID (alternative to connection_id)'),
  source_pin_name: z.string().optional().describe('Source pin name (alternative to connection_id)'),
  target_node_id: z.string().optional().describe('Target node ID (alternative to connection_id)'),
  target_pin_name: z.string().optional().describe('Target pin name (alternative to connection_id)'),
});

export class DisconnectBlueprintPinsTool extends BaseTool {
  readonly name = 'disconnect_blueprint_pins';
  readonly description = 'Disconnect two pins in a blueprint graph';
  readonly inputSchema = DisconnectBlueprintPinsSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof DisconnectBlueprintPinsSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'disconnect_blueprint_pins',
        params: validated,
        expectsResult: false,
      });

      if (!result.success) {
        return this.createErrorResult('DISCONNECT_FAILED', result.error ?? 'Failed to disconnect pins');
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

const GetNodeConnectionsSchema = z.object({
  blueprint_path: z.string().describe('Full path to the blueprint asset'),
  node_id: z.string().describe('ID of the node to get connections for'),
  include_input: z.boolean().optional().default(true).describe('Include input connections'),
  include_output: z.boolean().optional().default(true).describe('Include output connections'),
});

export class GetNodeConnectionsTool extends BaseTool {
  readonly name = 'get_node_connections';
  readonly description = 'Get all connections for a specific node';
  readonly inputSchema = GetNodeConnectionsSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<ConnectionInfo[]>> {
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

      const connections = result.result as ConnectionInfo[];
      return this.createSuccessResult(connections);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const GetNodePinsSchema = z.object({
  blueprint_path: z.string().describe('Full path to the blueprint asset'),
  node_id: z.string().describe('ID of the node to get pins for'),
  include_hidden: z.boolean().optional().default(false).describe('Include hidden pins'),
});

export class GetNodePinsTool extends BaseTool {
  readonly name = 'get_node_pins';
  readonly description = 'Get all pins (inputs and outputs) for a specific node';
  readonly inputSchema = GetNodePinsSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<PinInfo[]>> {
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

      const pins = result.result as PinInfo[];
      return this.createSuccessResult(pins);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}
