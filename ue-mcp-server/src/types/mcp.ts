export * from './core.js';

export interface MCPToolDefinition {
  name: string;
  description: string;
  inputSchema: unknown;
}

export interface MCPToolResult {
  content: Array<{
    type: 'text' | 'image' | 'resource';
    text?: string;
    data?: unknown;
  }>;
  isError?: boolean;
}

export interface MCPResourceDefinition {
  uri: string;
  name: string;
  description?: string;
  mimeType?: string;
}

export interface MCPPromptDefinition {
  name: string;
  description?: string;
  arguments?: Array<{
    name: string;
    description?: string;
    required?: boolean;
  }>;
}

export interface UEPythonCommand {
  command: string;
  params: Record<string, unknown>;
  expectsResult: boolean;
  timeout?: number;
}

export interface UEPythonResult {
  success: boolean;
  result?: unknown;
  error?: string;
  executionTime: number;
}

export interface SafetyPolicy {
  operationType: string;
  requiresConfirmation: boolean;
  maxRetries: number;
  allowedInTransaction: boolean;
  riskLevel: 'low' | 'medium' | 'high' | 'critical';
}

export interface BackupInfo {
  backupId: string;
  timestamp: Date;
  operationType: string;
  affectedAssets: string[];
  size: number;
}

export interface TransactionInfo {
  transactionId: string;
  startTime: Date;
  endTime?: Date;
  operations: string[];
  status: 'active' | 'committed' | 'rolledback';
  backupId?: string;
}
