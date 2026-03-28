export function generateId(prefix: string = 'id'): string {
  return `${prefix}_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`;
}

export function deepClone<T>(obj: T): T {
  return JSON.parse(JSON.stringify(obj)) as T;
}

export function debounce<T extends (...args: unknown[]) => unknown>(
  func: T,
  wait: number
): (...args: Parameters<T>) => void {
  let timeout: NodeJS.Timeout | null = null;
  
  return (...args: Parameters<T>) => {
    if (timeout) {
      clearTimeout(timeout);
    }
    timeout = setTimeout(() => func(...args), wait);
  };
}

export function throttle<T extends (...args: unknown[]) => unknown>(
  func: T,
  limit: number
): (...args: Parameters<T>) => void {
  let inThrottle = false;
  
  return (...args: Parameters<T>) => {
    if (!inThrottle) {
      func(...args);
      inThrottle = true;
      setTimeout(() => {
        inThrottle = false;
      }, limit);
    }
  };
}

export function sleep(ms: number): Promise<void> {
  return new Promise(resolve => setTimeout(resolve, ms));
}

export function isValidPath(path: string): boolean {
  const invalidChars = /[<>:"|?*]/;
  return !invalidChars.test(path) && path.length > 0;
}

export function normalizePath(path: string): string {
  return path.replace(/\\/g, '/');
}

export function joinPath(...parts: string[]): string {
  return parts.map(normalizePath).join('/').replace(/\/+/g, '/');
}

export function parseAssetPath(assetPath: string): {
  packagePath: string;
  assetName: string;
} {
  const normalized = normalizePath(assetPath);
  const lastSlash = normalized.lastIndexOf('/');
  const lastDot = normalized.lastIndexOf('.');
  
  if (lastSlash === -1 || lastDot === -1 || lastDot < lastSlash) {
    throw new Error(`Invalid asset path: ${assetPath}`);
  }
  
  return {
    packagePath: normalized.substring(0, lastSlash),
    assetName: normalized.substring(lastSlash + 1, lastDot),
  };
}

export function formatVector2D(x: number, y: number): string {
  return `(${x.toFixed(2)}, ${y.toFixed(2)})`;
}

export function formatVector3D(x: number, y: number, z: number): string {
  return `(${x.toFixed(2)}, ${y.toFixed(2)}, ${z.toFixed(2)})`;
}

export function formatRotator(pitch: number, yaw: number, roll: number): string {
  return `P=${pitch.toFixed(2)} Y=${yaw.toFixed(2)} R=${roll.toFixed(2)}`;
}

export function clamp(value: number, min: number, max: number): number {
  return Math.min(Math.max(value, min), max);
}

export function lerp(a: number, b: number, t: number): number {
  return a + (b - a) * t;
}

export function distance2D(
  x1: number, y1: number,
  x2: number, y2: number
): number {
  const dx = x2 - x1;
  const dy = y2 - y1;
  return Math.sqrt(dx * dx + dy * dy);
}

export function distance3D(
  x1: number, y1: number, z1: number,
  x2: number, y2: number, z2: number
): number {
  const dx = x2 - x1;
  const dy = y2 - y1;
  const dz = z2 - z1;
  return Math.sqrt(dx * dx + dy * dy + dz * dz);
}
