export * from './ActorTools.js';
export * from './LevelTools.js';

import { 
  SpawnActorTool, 
  MoveActorTool, 
  RotateActorTool, 
  ScaleActorTool, 
  DeleteActorTool, 
  GetSceneActorsTool,
  GetActorInfoTool,
  SetActorPropertyTool,
  DuplicateActorTool 
} from './ActorTools.js';
import { 
  CreateLevelTool, 
  LoadLevelTool, 
  SaveCurrentLevelTool, 
  AddLevelStreamingTool,
  SetWorldSettingsTool 
} from './LevelTools.js';
import type { BaseTool } from '../../core/BaseTool.js';

export const sceneTools: BaseTool<unknown>[] = [
  new SpawnActorTool(),
  new MoveActorTool(),
  new RotateActorTool(),
  new ScaleActorTool(),
  new DeleteActorTool(),
  new GetSceneActorsTool(),
  new GetActorInfoTool(),
  new SetActorPropertyTool(),
  new DuplicateActorTool(),
  new CreateLevelTool(),
  new LoadLevelTool(),
  new SaveCurrentLevelTool(),
  new AddLevelStreamingTool(),
  new SetWorldSettingsTool(),
];
