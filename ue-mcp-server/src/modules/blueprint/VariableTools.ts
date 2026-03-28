import { z } from 'zod';
import { BaseTool } from '../../core/BaseTool.js';
import type { OperationContext, OperationResult, VariableInfo, FunctionInfo } from '../../types/index.js';
import { ueBridge } from '../../bridge/index.js';

const CreateBlueprintVariableSchema = z.object({
  blueprint_path: z.string().describe('Full path to the blueprint asset'),
  variable_name: z.string().describe('Name of the variable to create'),
  variable_type: z.enum(['bool', 'int', 'float', 'string', 'vector', 'rotator', 'transform', 'object', 'class', 'struct']).describe('Type of the variable'),
  default_value: z.union([z.string(), z.number(), z.boolean(), z.record(z.unknown())]).optional().describe('Default value for the variable'),
  is_instance_editable: z.boolean().optional().default(false).describe('Make variable editable in the Details panel'),
  is_blueprint_read_only: z.boolean().optional().default(false).describe('Make variable read-only'),
  category: z.string().optional().default('Default').describe('Category for the variable'),
  tooltip: z.string().optional().describe('Tooltip text for the variable'),
  subtype: z.string().optional().describe('Sub-type for object/class/struct types'),
});

export class CreateBlueprintVariableTool extends BaseTool {
  readonly name = 'create_blueprint_variable';
  readonly description = 'Create a new variable in a blueprint';
  readonly inputSchema = CreateBlueprintVariableSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<VariableInfo>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof CreateBlueprintVariableSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'create_blueprint_variable',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('CREATE_VARIABLE_FAILED', result.error ?? 'Failed to create variable');
      }

      const variableInfo = result.result as VariableInfo;
      return this.createSuccessResult(variableInfo);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const SetVariableValueSchema = z.object({
  blueprint_path: z.string().describe('Full path to the blueprint asset'),
  variable_name: z.string().describe('Name of the variable to modify'),
  default_value: z.union([z.string(), z.number(), z.boolean(), z.record(z.unknown())]).describe('New default value'),
});

export class SetVariableValueTool extends BaseTool {
  readonly name = 'set_variable_value';
  readonly description = 'Set the default value of a blueprint variable';
  readonly inputSchema = SetVariableValueSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof SetVariableValueSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'set_variable_value',
        params: validated,
        expectsResult: false,
      });

      if (!result.success) {
        return this.createErrorResult('SET_VALUE_FAILED', result.error ?? 'Failed to set variable value');
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

const GetBlueprintVariablesSchema = z.object({
  blueprint_path: z.string().describe('Full path to the blueprint asset'),
  include_inherited: z.boolean().optional().default(true).describe('Include inherited variables'),
});

export class GetBlueprintVariablesTool extends BaseTool {
  readonly name = 'get_blueprint_variables';
  readonly description = 'Get all variables in a blueprint';
  readonly inputSchema = GetBlueprintVariablesSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<VariableInfo[]>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetBlueprintVariablesSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'get_blueprint_variables',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_VARIABLES_FAILED', result.error ?? 'Failed to get variables');
      }

      const variables = result.result as VariableInfo[];
      return this.createSuccessResult(variables);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const CreateBlueprintFunctionSchema = z.object({
  blueprint_path: z.string().describe('Full path to the blueprint asset'),
  function_name: z.string().describe('Name of the function to create'),
  parameters: z.array(z.object({
    name: z.string(),
    type: z.string(),
    default_value: z.union([z.string(), z.number(), z.boolean()]).optional(),
  })).optional().describe('Function parameters'),
  return_type: z.string().optional().describe('Return type of the function'),
  is_pure: z.boolean().optional().default(false).describe('Create a pure function (no execution pins)'),
  access_modifier: z.enum(['public', 'protected', 'private']).optional().default('public').describe('Access level'),
  description: z.string().optional().describe('Function description'),
});

export class CreateBlueprintFunctionTool extends BaseTool {
  readonly name = 'create_blueprint_function';
  readonly description = 'Create a new function in a blueprint';
  readonly inputSchema = CreateBlueprintFunctionSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<FunctionInfo>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof CreateBlueprintFunctionSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'create_blueprint_function',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('CREATE_FUNCTION_FAILED', result.error ?? 'Failed to create function');
      }

      const functionInfo = result.result as FunctionInfo;
      return this.createSuccessResult(functionInfo);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const GetBlueprintFunctionsSchema = z.object({
  blueprint_path: z.string().describe('Full path to the blueprint asset'),
  include_inherited: z.boolean().optional().default(true).describe('Include inherited functions'),
});

export class GetBlueprintFunctionsTool extends BaseTool {
  readonly name = 'get_blueprint_functions';
  readonly description = 'Get all functions in a blueprint';
  readonly inputSchema = GetBlueprintFunctionsSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<FunctionInfo[]>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetBlueprintFunctionsSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'get_blueprint_functions',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_FUNCTIONS_FAILED', result.error ?? 'Failed to get functions');
      }

      const functions = result.result as FunctionInfo[];
      return this.createSuccessResult(functions);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const DeleteBlueprintVariableSchema = z.object({
  blueprint_path: z.string().describe('Full path to the blueprint asset'),
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
        expectsResult: false,
      });

      if (!result.success) {
        return this.createErrorResult('DELETE_VARIABLE_FAILED', result.error ?? 'Failed to delete variable');
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
