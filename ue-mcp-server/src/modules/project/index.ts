export * from './ProjectTools.js';

import { 
  ConfigureInputTool, 
  CreateGameModeTool, 
  SetProjectSettingsTool,
  CreateFolderTool,
  GetProjectInfoTool 
} from './ProjectTools.js';
import type { BaseTool } from '../../core/BaseTool.js';

export const projectTools: BaseTool<unknown>[] = [
  new ConfigureInputTool(),
  new CreateGameModeTool(),
  new SetProjectSettingsTool(),
  new CreateFolderTool(),
  new GetProjectInfoTool(),
];
