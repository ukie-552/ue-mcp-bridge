export * from './NodeTools.js';
export * from './ConnectionTools.js';
export * from './VariableTools.js';

import { GetBlueprintNodesTool, CreateBlueprintNodeTool, MoveBlueprintNodeTool, DeleteBlueprintNodeTool, GetBlueprintGraphTreeTool, GetActiveBlueprintStateTool } from './NodeTools.js';
import { ConnectBlueprintPinsTool, DisconnectBlueprintPinsTool, GetNodeConnectionsTool, GetNodePinsTool } from './ConnectionTools.js';
import { CreateBlueprintVariableTool, SetVariableValueTool, GetBlueprintVariablesTool, CreateBlueprintFunctionTool, GetBlueprintFunctionsTool, DeleteBlueprintVariableTool } from './VariableTools.js';
import type { BaseTool } from '../../core/BaseTool.js';

export const blueprintTools: BaseTool<unknown>[] = [
  new GetBlueprintNodesTool(),
  new CreateBlueprintNodeTool(),
  new MoveBlueprintNodeTool(),
  new DeleteBlueprintNodeTool(),
  new GetBlueprintGraphTreeTool(),
  new GetActiveBlueprintStateTool(),
  new ConnectBlueprintPinsTool(),
  new DisconnectBlueprintPinsTool(),
  new GetNodeConnectionsTool(),
  new GetNodePinsTool(),
  new CreateBlueprintVariableTool(),
  new SetVariableValueTool(),
  new GetBlueprintVariablesTool(),
  new CreateBlueprintFunctionTool(),
  new GetBlueprintFunctionsTool(),
  new DeleteBlueprintVariableTool(),
];
