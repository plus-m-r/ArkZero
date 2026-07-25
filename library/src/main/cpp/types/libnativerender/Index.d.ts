declare module 'libnativerender.so' {

  // ===== Renderer API (核心渲染接口) =====

  export function create(
    surfaceId: string,
    width: number,
    height: number,
    format: number
  ): Promise<number>;

  export function renderFrame(
    handle: number,
    pixelData: ArrayBuffer,
    width: number,
    height: number
  ): Promise<void>;

  interface DirtyRect {
    x: number;
    y: number;
    w: number;
    h: number;
  }

  export function renderFrameRegions(
    handle: number,
    pixelData: ArrayBuffer,
    frameWidth: number,
    frameHeight: number,
    regions: Array<DirtyRect>,
    swap: boolean
  ): Promise<void>;

  export function updateDirtyRegions(
    handle: number,
    pixelData: ArrayBuffer,
    frameWidth: number,
    frameHeight: number,
    regions: Array<DirtyRect>
  ): Promise<void>;

  export function presentFrame(
    handle: number
  ): Promise<void>;

  export function resize(
    handle: number,
    width: number,
    height: number
  ): Promise<void>;

  export function destroy(handle: number): Promise<void>;

  // ===== RendererManager API =====

  export function managerCreateSurfaceRenderer(
    surfaceId: string,
    width: number,
    height: number,
    format: number
  ): number;

  export function managerDestroyRenderer(handle: number): boolean;

  // ===== SurfaceManager API =====

  export function surfaceManagerCreateNativeWindow(surfaceId: string): number;

  export function surfaceManagerDestroyNativeWindow(windowPtr: number): boolean;

  // ===== EGLContextManager API =====

  export function createEGLContext(width: number, height: number): number;

  export function destroyEGLContext(handle: number): void;

  export function eglMakeCurrent(handle: number): boolean;

  export function isEGLInitialized(handle: number): boolean;

  // ===== GLESBackend API =====

  export function createGLESBackend(): number;

  export function destroyGLESBackend(handle: number): void;

  export function glesBackendInitialize(
    handle: number,
    width: number,
    height: number,
    format: number
  ): boolean;

  export function glesBackendRenderFrame(
    handle: number,
    pixelData: ArrayBuffer,
    width: number,
    height: number
  ): boolean;

  export function isGLESBackendInitialized(handle: number): boolean;

  // ===== TextureManager API =====

  export function createTextureManager(): number;

  export function destroyTextureManager(handle: number): void;

  export function textureCreate(
    handle: number,
    width: number,
    height: number,
    internalFormat: number,
    format: number
  ): boolean;

  export function textureUpdate(
    handle: number,
    pixelData: ArrayBuffer,
    width: number,
    height: number,
    format: number
  ): boolean;

  export function textureDestroy(handle: number): void;

  export function getTextureId(handle: number): number;

  export function isTextureCreated(handle: number): boolean;

  // ===== TexturePool API =====

  export function createTexturePool(maxSize?: number): number;

  export function destroyTexturePool(handle: number): void;

  export function texturePoolAcquire(
    handle: number,
    width: number,
    height: number,
    internalFormat: number,
    format: number
  ): boolean;

  export function texturePoolRelease(handle: number): void;

  interface Resolution {
    width: number;
    height: number;
  }

  export function texturePoolPreallocate(
    handle: number,
    resolutions: Array<Resolution>,
    internalFormat: number,
    format: number
  ): void;

  export function texturePoolClear(handle: number): void;

  export function texturePoolSize(handle: number): number;

  interface TexturePoolStats {
    hitCount: number;
    missCount: number;
    hitRate: number;
  }

  export function texturePoolGetStats(handle: number): TexturePoolStats;

  // ===== PixelFormatConverter API =====

  export function getGLInternalFormat(format: number): number;

  export function getGLFormat(format: number): number;

  export function getBytesPerPixel(format: number): number;

  // ===== YUVShaderManager API =====

  export function createYUVShaderManager(): number;

  export function destroyYUVShaderManager(handle: number): void;

  export function yuvShaderInitialize(handle: number): boolean;

  export function yuvShaderRenderNV21(
    handle: number,
    yBuffer: ArrayBuffer,
    uvBuffer: ArrayBuffer,
    width: number,
    height: number
  ): boolean;

  export function yuvShaderRenderNV12(
    handle: number,
    yBuffer: ArrayBuffer,
    uvBuffer: ArrayBuffer,
    width: number,
    height: number
  ): boolean;

  export function isYUVShaderInitialized(handle: number): boolean;

  // ===== PerformanceMonitor API =====

  export function createPerformanceMonitor(): number;

  export function destroyPerformanceMonitor(handle: number): void;

  export function monitorBeginFrame(handle: number): void;

  export function monitorEndFrame(handle: number, dropped: boolean): void;

  export function monitorReset(handle: number): void;

  interface MonitorStats {
    totalFrames: number;
    droppedFrames: number;
    fps: number;
    dropRate: number;
    frameTimeMs: number;
  }

  export function getMonitorStats(handle: number): MonitorStats;

  // ===== 模块默认导出 =====

  const nativerender: {
    create: typeof create;
    renderFrame: typeof renderFrame;
    DirtyRect: DirtyRect;
    renderFrameRegions: typeof renderFrameRegions;
    updateDirtyRegions: typeof updateDirtyRegions;
    presentFrame: typeof presentFrame;
    resize: typeof resize;
    destroy: typeof destroy;
    managerCreateSurfaceRenderer: typeof managerCreateSurfaceRenderer;
    managerDestroyRenderer: typeof managerDestroyRenderer;
    surfaceManagerCreateNativeWindow: typeof surfaceManagerCreateNativeWindow;
    surfaceManagerDestroyNativeWindow: typeof surfaceManagerDestroyNativeWindow;
    createEGLContext: typeof createEGLContext;
    destroyEGLContext: typeof destroyEGLContext;
    eglMakeCurrent: typeof eglMakeCurrent;
    isEGLInitialized: typeof isEGLInitialized;
    createGLESBackend: typeof createGLESBackend;
    destroyGLESBackend: typeof destroyGLESBackend;
    glesBackendInitialize: typeof glesBackendInitialize;
    glesBackendRenderFrame: typeof glesBackendRenderFrame;
    isGLESBackendInitialized: typeof isGLESBackendInitialized;
    createTextureManager: typeof createTextureManager;
    destroyTextureManager: typeof destroyTextureManager;
    textureCreate: typeof textureCreate;
    textureUpdate: typeof textureUpdate;
    textureDestroy: typeof textureDestroy;
    getTextureId: typeof getTextureId;
    isTextureCreated: typeof isTextureCreated;
    createTexturePool: typeof createTexturePool;
    destroyTexturePool: typeof destroyTexturePool;
    texturePoolAcquire: typeof texturePoolAcquire;
    texturePoolRelease: typeof texturePoolRelease;
    texturePoolPreallocate: typeof texturePoolPreallocate;
    texturePoolClear: typeof texturePoolClear;
    texturePoolSize: typeof texturePoolSize;
    texturePoolGetStats: typeof texturePoolGetStats;
    getGLInternalFormat: typeof getGLInternalFormat;
    getGLFormat: typeof getGLFormat;
    getBytesPerPixel: typeof getBytesPerPixel;
    createYUVShaderManager: typeof createYUVShaderManager;
    destroyYUVShaderManager: typeof destroyYUVShaderManager;
    yuvShaderInitialize: typeof yuvShaderInitialize;
    yuvShaderRenderNV21: typeof yuvShaderRenderNV21;
    yuvShaderRenderNV12: typeof yuvShaderRenderNV12;
    isYUVShaderInitialized: typeof isYUVShaderInitialized;
    createPerformanceMonitor: typeof createPerformanceMonitor;
    destroyPerformanceMonitor: typeof destroyPerformanceMonitor;
    monitorBeginFrame: typeof monitorBeginFrame;
    monitorEndFrame: typeof monitorEndFrame;
    monitorReset: typeof monitorReset;
    getMonitorStats: typeof getMonitorStats;
  };

  export default nativerender;
}
