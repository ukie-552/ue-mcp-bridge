import { z } from 'zod';
import { BaseTool } from '../../core/BaseTool.js';
import type { OperationContext, OperationResult } from '../../types/index.js';
import { ueBridge } from '../../bridge/index.js';

const CreateWidgetBlueprintSchema = z.object({
  widget_path: z.string().describe('Path where the widget blueprint will be created (e.g., /Game/UI)'),
  widget_name: z.string().describe('Name of the widget blueprint'),
  parent_class: z.string().optional().describe('Path to parent widget class (optional, defaults to UUserWidget)'),
});

export class CreateWidgetBlueprintTool extends BaseTool<unknown> {
  readonly name = 'create_widget_blueprint';
  readonly description = 'Create a new UMG Widget Blueprint';
  readonly inputSchema = CreateWidgetBlueprintSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof CreateWidgetBlueprintSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'create_widget_blueprint',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('CREATE_FAILED', result.error ?? 'Failed to create widget blueprint');
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

const AddWidgetControlSchema = z.object({
  widget_path: z.string().describe('Path to the widget blueprint'),
  control_type: z.string().describe('Type of control to add (e.g., Button, TextBlock, Image, CanvasPanel)'),
  control_name: z.string().optional().describe('Name for the control'),
  parent_control: z.string().optional().describe('Name of parent control to add to'),
});

export class AddWidgetControlTool extends BaseTool<unknown> {
  readonly name = 'add_widget_control';
  readonly description = 'Add a control to a UMG widget blueprint';
  readonly inputSchema = AddWidgetControlSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof AddWidgetControlSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'add_widget_control',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('ADD_FAILED', result.error ?? 'Failed to add widget control');
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

const SetWidgetPropertySchema = z.object({
  widget_path: z.string().describe('Path to the widget blueprint'),
  control_name: z.string().describe('Name of the control'),
  property_name: z.string().describe('Name of the property to set'),
  property_value: z.record(z.unknown()).describe('Value object for the property'),
});

export class SetWidgetPropertyTool extends BaseTool<unknown> {
  readonly name = 'set_widget_property';
  readonly description = 'Set a property on a widget control';
  readonly inputSchema = SetWidgetPropertySchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof SetWidgetPropertySchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'set_widget_property',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('SET_PROPERTY_FAILED', result.error ?? 'Failed to set widget property');
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

const BindWidgetEventSchema = z.object({
  widget_path: z.string().describe('Path to the widget blueprint'),
  control_name: z.string().describe('Name of the control'),
  event_type: z.string().describe('Type of event (e.g., OnClicked, OnHovered, OnValueChanged)'),
  event_name: z.string().optional().describe('Custom name for the event function'),
});

export class BindWidgetEventTool extends BaseTool<unknown> {
  readonly name = 'bind_widget_event';
  readonly description = 'Bind an event to a widget control';
  readonly inputSchema = BindWidgetEventSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof BindWidgetEventSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'bind_widget_event',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('BIND_EVENT_FAILED', result.error ?? 'Failed to bind widget event');
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

const SetWidgetLayoutSchema = z.object({
  widget_path: z.string().describe('Path to the widget blueprint'),
  control_name: z.string().describe('Name of the control'),
  anchors: z.object({
    minimum: z.object({ x: z.number(), y: z.number() }).optional(),
    maximum: z.object({ x: z.number(), y: z.number() }).optional(),
  }).optional().describe('Anchor points'),
  position: z.object({
    value: z.object({ x: z.number(), y: z.number() }),
  }).optional().describe('Position'),
  size: z.object({
    value: z.object({ x: z.number(), y: z.number() }),
  }).optional().describe('Size'),
  alignment: z.object({
    value: z.object({ x: z.number(), y: z.number() }),
  }).optional().describe('Alignment'),
  padding: z.object({
    left: z.number().optional(),
    top: z.number().optional(),
    right: z.number().optional(),
    bottom: z.number().optional(),
  }).optional().describe('Padding'),
  h_align: z.enum(['Left', 'Center', 'Right', 'Fill']).optional().describe('Horizontal alignment'),
  v_align: z.enum(['Top', 'Center', 'Bottom', 'Fill']).optional().describe('Vertical alignment'),
  zorder: z.number().optional().describe('Z-Order for canvas panel'),
  row: z.number().optional().describe('Row for grid layouts'),
  column: z.number().optional().describe('Column for grid layouts'),
  row_span: z.number().optional().describe('Row span for grid layouts'),
  column_span: z.number().optional().describe('Column span for grid layouts'),
});

export class SetWidgetLayoutTool extends BaseTool<unknown> {
  readonly name = 'set_widget_layout';
  readonly description = 'Set layout properties for a widget control (anchors, alignment, position, size)';
  readonly inputSchema = SetWidgetLayoutSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof SetWidgetLayoutSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'set_widget_layout',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('SET_LAYOUT_FAILED', result.error ?? 'Failed to set widget layout');
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

const GetWidgetControlsSchema = z.object({
  widget_path: z.string().describe('Path to the widget blueprint'),
  recursive: z.boolean().optional().default(true).describe('Get all controls recursively'),
  parent_control: z.string().optional().describe('Get children of specific control only'),
});

export class GetWidgetControlsTool extends BaseTool<unknown> {
  readonly name = 'get_widget_controls';
  readonly description = 'Get all controls in a widget blueprint';
  readonly inputSchema = GetWidgetControlsSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetWidgetControlsSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'get_widget_controls',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_CONTROLS_FAILED', result.error ?? 'Failed to get widget controls');
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

const DeleteWidgetControlSchema = z.object({
  widget_path: z.string().describe('Path to the widget blueprint'),
  control_name: z.string().describe('Name of the control to delete'),
});

export class DeleteWidgetControlTool extends BaseTool<unknown> {
  readonly name = 'delete_widget_control';
  readonly description = 'Delete a control from a widget blueprint';
  readonly inputSchema = DeleteWidgetControlSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof DeleteWidgetControlSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'delete_widget_control',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('DELETE_FAILED', result.error ?? 'Failed to delete widget control');
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

const RenameWidgetControlSchema = z.object({
  widget_path: z.string().describe('Path to the widget blueprint'),
  old_name: z.string().describe('Current name of the control'),
  new_name: z.string().describe('New name for the control'),
});

export class RenameWidgetControlTool extends BaseTool<unknown> {
  readonly name = 'rename_widget_control';
  readonly description = 'Rename a control in a widget blueprint';
  readonly inputSchema = RenameWidgetControlSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof RenameWidgetControlSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'rename_widget_control',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('RENAME_FAILED', result.error ?? 'Failed to rename widget control');
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

export const umgTools: BaseTool<unknown>[] = [
  new CreateWidgetBlueprintTool(),
  new AddWidgetControlTool(),
  new SetWidgetPropertyTool(),
  new BindWidgetEventTool(),
  new SetWidgetLayoutTool(),
  new GetWidgetControlsTool(),
  new DeleteWidgetControlTool(),
  new RenameWidgetControlTool(),
];
