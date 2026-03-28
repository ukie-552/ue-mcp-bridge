import { z } from 'zod';
import { BaseTool } from '../../core/BaseTool.js';
import type { OperationContext, OperationResult, AssetInfo } from '../../types/index.js';
import { ueBridge } from '../../bridge/index.js';

const CreateAssetSchema = z.object({
  asset_type: z.enum(['Blueprint', 'Material', 'Widget', 'AnimationBlueprint', 'Texture', 'Sound', 'DataTable']).describe('Type of asset to create'),
  asset_name: z.string().describe('Name for the new asset'),
  parent_class: z.string().optional().describe('Parent class for Blueprint assets'),
  save_path: z.string().optional().describe('Path to save the asset'),
});

export class CreateAssetTool extends BaseTool {
  readonly name = 'create_asset';
  readonly description = 'Create a new asset in the project';
  readonly inputSchema = CreateAssetSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<AssetInfo>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof CreateAssetSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'create_asset',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('CREATE_ASSET_FAILED', result.error ?? 'Failed to create asset');
      }

      const assetInfo = result.result as AssetInfo;
      return this.createSuccessResult(assetInfo);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const RenameAssetSchema = z.object({
  current_path: z.string().describe('Current path to the asset'),
  new_name: z.string().describe('New name for the asset'),
});

export class RenameAssetTool extends BaseTool {
  readonly name = 'rename_asset';
  readonly description = 'Rename an existing asset';
  readonly inputSchema = RenameAssetSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<AssetInfo>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof RenameAssetSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'rename_asset',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('RENAME_ASSET_FAILED', result.error ?? 'Failed to rename asset');
      }

      const assetInfo = result.result as AssetInfo;
      return this.createSuccessResult(assetInfo);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const MoveAssetSchema = z.object({
  current_path: z.string().describe('Current path to the asset'),
  new_path: z.string().describe('New path for the asset'),
});

export class MoveAssetTool extends BaseTool {
  readonly name = 'move_asset';
  readonly description = 'Move an asset to a new location';
  readonly inputSchema = MoveAssetSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<AssetInfo>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof MoveAssetSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'move_asset',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('MOVE_ASSET_FAILED', result.error ?? 'Failed to move asset');
      }

      const assetInfo = result.result as AssetInfo;
      return this.createSuccessResult(assetInfo);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const DeleteAssetSchema = z.object({
  asset_path: z.string().describe('Path to the asset to delete'),
  force: z.boolean().optional().default(false).describe('Force delete even if referenced'),
});

export class DeleteAssetTool extends BaseTool {
  readonly name = 'delete_asset';
  readonly description = 'Delete an asset from the project';
  readonly inputSchema = DeleteAssetSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<unknown>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof DeleteAssetSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'delete_asset',
        params: validated,
        expectsResult: false,
      });

      if (!result.success) {
        return this.createErrorResult('DELETE_ASSET_FAILED', result.error ?? 'Failed to delete asset');
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

const GetAssetReferencesSchema = z.object({
  asset_path: z.string().describe('Path to the asset'),
  include_soft_references: z.boolean().optional().default(true).describe('Include soft references'),
});

export class GetAssetReferencesTool extends BaseTool {
  readonly name = 'get_asset_references';
  readonly description = 'Get all assets that reference a specific asset';
  readonly inputSchema = GetAssetReferencesSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<string[]>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof GetAssetReferencesSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'get_asset_references',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('GET_REFERENCES_FAILED', result.error ?? 'Failed to get asset references');
      }

      const references = result.result as string[];
      return this.createSuccessResult(references);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const FindAssetsSchema = z.object({
  search_path: z.string().optional().describe('Path to search in'),
  filter: z.object({
    class_name: z.string().optional(),
    name_contains: z.string().optional(),
    tags: z.array(z.string()).optional(),
  }).optional().describe('Filter criteria'),
});

export class FindAssetsTool extends BaseTool {
  readonly name = 'find_assets';
  readonly description = 'Find assets matching specific criteria';
  readonly inputSchema = FindAssetsSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<AssetInfo[]>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof FindAssetsSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'find_assets',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('FIND_ASSETS_FAILED', result.error ?? 'Failed to find assets');
      }

      const assets = result.result as AssetInfo[];
      return this.createSuccessResult(assets);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}

const DuplicateAssetSchema = z.object({
  source_path: z.string().describe('Path to the asset to duplicate'),
  new_name: z.string().describe('Name for the duplicated asset'),
  new_path: z.string().optional().describe('Path for the duplicated asset'),
});

export class DuplicateAssetTool extends BaseTool {
  readonly name = 'duplicate_asset';
  readonly description = 'Duplicate an existing asset';
  readonly inputSchema = DuplicateAssetSchema;

  async execute(params: unknown, _context: OperationContext): Promise<OperationResult<AssetInfo>> {
    try {
      const validated = this.validateInput(params) as z.infer<typeof DuplicateAssetSchema>;
      
      const result = await ueBridge.executeCommand({
        command: 'duplicate_asset',
        params: validated,
        expectsResult: true,
      });

      if (!result.success) {
        return this.createErrorResult('DUPLICATE_ASSET_FAILED', result.error ?? 'Failed to duplicate asset');
      }

      const assetInfo = result.result as AssetInfo;
      return this.createSuccessResult(assetInfo);
    } catch (error) {
      return this.createErrorResult(
        'VALIDATION_ERROR',
        error instanceof Error ? error.message : 'Unknown validation error'
      );
    }
  }
}
