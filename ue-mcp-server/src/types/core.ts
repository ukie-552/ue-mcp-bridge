export interface Vector2D {
  x: number;
  y: number;
}

export interface Vector3D {
  x: number;
  y: number;
  z: number;
}

export interface Rotator {
  pitch: number;
  yaw: number;
  roll: number;
}

export interface Transform {
  location: Vector3D;
  rotation: Rotator;
  scale: Vector3D;
}

export interface Color {
  r: number;
  g: number;
  b: number;
  a: number;
}

export type PinCategory = 
  | 'bool' 
  | 'int' 
  | 'float' 
  | 'string' 
  | 'object' 
  | 'struct' 
  | 'delegate' 
  | 'exec';

export interface PinType {
  category: PinCategory;
  subCategory?: string;
  isArray: boolean;
  isReference: boolean;
}

export interface NodeInfo {
  nodeId: string;
  nodeName: string;
  nodeType: string;
  position: Vector2D;
  size?: Vector2D;
  comment?: string;
  graphName: string;
  blueprintPath: string;
}

export interface PinInfo {
  pinId: string;
  pinName: string;
  pinType: PinType;
  isInput: boolean;
  defaultValue?: unknown;
  connectedPins: string[];
  owningNodeId: string;
}

export interface ConnectionInfo {
  connectionId: string;
  sourcePinId: string;
  targetPinId: string;
  sourceNodeId: string;
  targetNodeId: string;
}

export interface VariableInfo {
  variableName: string;
  variableType: PinType;
  defaultValue?: unknown;
  isInstanceEditable: boolean;
  isBlueprintReadOnly: boolean;
  category: string;
  tooltip: string;
}

export interface FunctionInfo {
  functionName: string;
  parameters: Array<{
    name: string;
    type: PinType;
    defaultValue?: unknown;
  }>;
  returnType?: PinType;
  isPure: boolean;
  accessModifier: 'public' | 'protected' | 'private';
}

export interface BlueprintInfo {
  blueprintPath: string;
  blueprintName: string;
  blueprintType: 'Actor' | 'Widget' | 'FunctionLibrary' | 'Interface' | 'Enum' | 'Struct';
  parentClass: string;
  graphs: string[];
  variables: VariableInfo[];
  functions: FunctionInfo[];
}

export interface ActorInfo {
  actorId: string;
  actorName: string;
  actorPath: string;
  className: string;
  transform: Transform;
  tags: string[];
  isHidden: boolean;
  isEditable: boolean;
  components: string[];
}

export interface AssetInfo {
  assetPath: string;
  assetName: string;
  assetType: string;
  packagePath: string;
  fileSize?: number;
  dependencies: string[];
  referencers: string[];
}

export interface OperationResult<T = void> {
  success: boolean;
  data?: T;
  error?: {
    code: string;
    message: string;
    details?: unknown;
  };
  warnings?: string[];
  transactionId?: string;
}

export interface SceneSnapshot {
  levelName: string;
  actors: ActorInfo[];
  selectedActors: string[];
  viewLocation: Vector3D;
  viewRotation: Rotator;
}

export interface BlueprintSnapshot {
  blueprintPath: string;
  currentGraph: string;
  nodes: NodeInfo[];
  connections: ConnectionInfo[];
  variables: VariableInfo[];
  functions: FunctionInfo[];
  selectedNodes: string[];
}

export interface OperationContext {
  operationId: string;
  timestamp: Date;
  userId?: string;
  sessionId: string;
  parentTransactionId?: string;
}

export type LogLevel = 'debug' | 'info' | 'warning' | 'error';

export interface LogEntry {
  level: LogLevel;
  message: string;
  timestamp: Date;
  context?: OperationContext;
  data?: unknown;
}
