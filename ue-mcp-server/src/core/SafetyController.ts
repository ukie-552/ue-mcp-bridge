import type { 
  SafetyPolicy, 
  TransactionInfo, 
  BackupInfo,
  OperationContext 
} from '../types/index.js';

export interface SafetyCheckResult {
  allowed: boolean;
  requiresConfirmation: boolean;
  riskLevel: 'low' | 'medium' | 'high' | 'critical';
  warnings: string[];
  suggestedBackup: boolean;
}

export class SafetyController {
  private readonly policies: Map<string, SafetyPolicy> = new Map();
  private readonly activeTransactions: Map<string, TransactionInfo> = new Map();
  private readonly backups: Map<string, BackupInfo> = new Map();
  private readonly dangerousOperations = new Set([
    'delete_asset',
    'delete_actor',
    'delete_node',
    'batch_delete',
    'modify_project_settings',
    'delete_level',
  ]);

  constructor() {
    this.initializePolicies();
  }

  private initializePolicies(): void {
    this.policies.set('delete_asset', {
      operationType: 'delete_asset',
      requiresConfirmation: true,
      maxRetries: 1,
      allowedInTransaction: true,
      riskLevel: 'high',
    });

    this.policies.set('delete_actor', {
      operationType: 'delete_actor',
      requiresConfirmation: true,
      maxRetries: 1,
      allowedInTransaction: true,
      riskLevel: 'medium',
    });

    this.policies.set('batch_operation', {
      operationType: 'batch_operation',
      requiresConfirmation: true,
      maxRetries: 0,
      allowedInTransaction: true,
      riskLevel: 'high',
    });

    this.policies.set('modify_blueprint', {
      operationType: 'modify_blueprint',
      requiresConfirmation: false,
      maxRetries: 3,
      allowedInTransaction: true,
      riskLevel: 'low',
    });

    this.policies.set('create_asset', {
      operationType: 'create_asset',
      requiresConfirmation: false,
      maxRetries: 3,
      allowedInTransaction: true,
      riskLevel: 'low',
    });
  }

  checkOperation(
    operationName: string, 
    params: unknown,
    context: OperationContext
  ): SafetyCheckResult {
    const policy = this.policies.get(operationName);
    const isDangerous = this.dangerousOperations.has(operationName);

    const result: SafetyCheckResult = {
      allowed: true,
      requiresConfirmation: policy?.requiresConfirmation ?? isDangerous,
      riskLevel: policy?.riskLevel ?? (isDangerous ? 'medium' : 'low'),
      warnings: [],
      suggestedBackup: false,
    };

    if (isDangerous) {
      result.warnings.push(
        `Operation "${operationName}" is marked as dangerous. Please confirm before execution.`
      );
      result.suggestedBackup = true;
    }

    if (policy && !policy.allowedInTransaction && context.parentTransactionId) {
      result.allowed = false;
      result.warnings.push(
        `Operation "${operationName}" is not allowed within a transaction.`
      );
    }

    return result;
  }

  beginTransaction(
    _operationType: string, 
    _context: OperationContext
  ): string {
    const transactionId = `txn_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`;
    
    const transaction: TransactionInfo = {
      transactionId,
      startTime: new Date(),
      operations: [],
      status: 'active',
    };

    this.activeTransactions.set(transactionId, transaction);
    return transactionId;
  }

  commitTransaction(transactionId: string): boolean {
    const transaction = this.activeTransactions.get(transactionId);
    if (!transaction || transaction.status !== 'active') {
      return false;
    }

    transaction.status = 'committed';
    transaction.endTime = new Date();
    return true;
  }

  rollbackTransaction(transactionId: string): boolean {
    const transaction = this.activeTransactions.get(transactionId);
    if (!transaction || transaction.status !== 'active') {
      return false;
    }

    transaction.status = 'rolledback';
    transaction.endTime = new Date();
    return true;
  }

  createBackup(
    affectedAssets: string[], 
    operationType: string
  ): string {
    const backupId = `backup_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`;
    
    const backup: BackupInfo = {
      backupId,
      timestamp: new Date(),
      operationType,
      affectedAssets,
      size: 0,
    };

    this.backups.set(backupId, backup);
    return backupId;
  }

  getPolicy(operationName: string): SafetyPolicy | undefined {
    return this.policies.get(operationName);
  }

  registerPolicy(policy: SafetyPolicy): void {
    this.policies.set(policy.operationType, policy);
  }
}
