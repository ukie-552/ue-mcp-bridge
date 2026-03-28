export * from './AssetTools.js';

import { 
  CreateAssetTool, 
  RenameAssetTool, 
  MoveAssetTool, 
  DeleteAssetTool, 
  GetAssetReferencesTool,
  FindAssetsTool,
  DuplicateAssetTool 
} from './AssetTools.js';
import type { BaseTool } from '../../core/BaseTool.js';

export const assetTools: BaseTool<unknown>[] = [
  new CreateAssetTool(),
  new RenameAssetTool(),
  new MoveAssetTool(),
  new DeleteAssetTool(),
  new GetAssetReferencesTool(),
  new FindAssetsTool(),
  new DuplicateAssetTool(),
];
