import { blueprintTools } from './blueprint/index.js';
import { sceneTools } from './scene/index.js';
import { assetTools } from './asset/index.js';
import { projectTools } from './project/index.js';
import { umgTools } from './umg/index.js';
import { materialTools } from './material/index.js';
import { animationTools } from './animation/index.js';
import { sequencerTools } from './sequencer/index.js';
import { niagaraTools } from './niagara/index.js';
import { nodeDiscoveryTools } from './discovery/index.js';
import { editorTools } from './editor/index.js';
import type { BaseTool } from '../core/BaseTool.js';

export { blueprintTools, sceneTools, assetTools, projectTools, umgTools, materialTools, animationTools, sequencerTools, niagaraTools, nodeDiscoveryTools, editorTools };

export const allTools: BaseTool<unknown>[] = [
  ...blueprintTools,
  ...sceneTools,
  ...assetTools,
  ...projectTools,
  ...umgTools,
  ...materialTools,
  ...animationTools,
  ...sequencerTools,
  ...niagaraTools,
  ...nodeDiscoveryTools,
  ...editorTools,
];
