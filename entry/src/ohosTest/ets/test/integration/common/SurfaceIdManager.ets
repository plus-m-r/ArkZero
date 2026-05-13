/**
 * 全局 Surface ID 管理器
 * 
 * ⭐ 用于在 IntegrationTestPage 和集成测试之间共享真实的 XComponent Surface ID
 */

class SurfaceIdManager {
  private static instance: SurfaceIdManager;
  private surfaceId: string = '';
  private isReady: boolean = false;
  private waitQueue: Array<(surfaceId: string) => void> = [];
  
  private constructor() {}
  
  static getInstance(): SurfaceIdManager {
    if (!SurfaceIdManager.instance) {
      SurfaceIdManager.instance = new SurfaceIdManager();
    }
    return SurfaceIdManager.instance;
  }
  
  /**
   * 设置 Surface ID（由 IntegrationTestPage 调用）
   */
  setSurfaceId(surfaceId: string): void {
    console.info(`[SurfaceIdManager] Surface ID set: ${surfaceId}`);
    this.surfaceId = surfaceId;
    this.isReady = true;
    
    // 通知所有等待者
    while (this.waitQueue.length > 0) {
      const callback = this.waitQueue.shift();
      if (callback) {
        callback(surfaceId);
      }
    }
  }
  
  /**
   * 获取 Surface ID（同步）
   */
  getSurfaceId(): string {
    return this.surfaceId;
  }
  
  /**
   * 检查 Surface 是否就绪
   */
  isSurfaceReady(): boolean {
    return this.isReady;
  }
  
  /**
   * 等待 Surface ID 就绪（异步）
   */
  async waitForSurface(timeout: number = 5000): Promise<string> {
    if (this.isReady) {
      return this.surfaceId;
    }
    
    return new Promise<string>((resolve, reject) => {
      // 添加到等待队列
      this.waitQueue.push((surfaceId: string) => {
        resolve(surfaceId);
      });
      
      // 超时处理
      setTimeout(() => {
        if (!this.isReady) {
          // 从队列中移除
          const index = this.waitQueue.findIndex(cb => cb === this.waitQueue[this.waitQueue.length - 1]);
          if (index !== -1) {
            this.waitQueue.splice(index, 1);
          }
          reject(new Error(`Timeout waiting for Surface ID (${timeout}ms)`));
        }
      }, timeout);
    });
  }
  
  /**
   * 重置状态（用于清理）
   */
  reset(): void {
    this.surfaceId = '';
    this.isReady = false;
    this.waitQueue = [];
  }
}

export default SurfaceIdManager;
