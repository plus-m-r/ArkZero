/**
 * Native屏幕渲染器模块
 * 
 * 🎯 **设计原则**：
 * - 只暴露基本类型接口（number、ArrayBuffer）
 * - 不暴露复杂对象（如Capture、Tile等）
 * - 保持接口极简，只有5个核心方法
 */
declare module 'nativerender' {
  
  /**
   * 像素格式枚举
   */
  enum PixelFormat {
    RGBA = 0,   // RGBA8888 (4字节/像素)
    RGB = 1,    // RGB888 (3字节/像素)
    BGRA = 2,   // BGRA8888 (4字节/像素)
    NV21 = 3,   // YUV420半平面 (1.5字节/像素)
    NV12 = 4    // YUV420半平面 (1.5字节/像素)
  }

  /**
   * 创建渲染器
   * 
   * @param width 宽度
   * @param height 高度
   * @param format 像素格式（必选）
   * @returns 渲染器句柄（number）
   * 
   * ⚠️ 注意：此方法使用离屏渲染模式，已废弃。
   * 请使用 createWithSurface 获得最佳性能。
   */
  export function create(width: number, height: number, format: PixelFormat): Promise<number>;

  /**
   * 创建渲染器（使用 XComponent Surface）
   * 
   * ⭐ **推荐方式**：Direct Surface Rendering
   * 
   * @param surfaceId XComponent 的 surface ID
   * @param width 宽度
   * @param height 高度
   * @param format 像素格式
   * @returns 渲染器句柄（number）
   * 
   * 🎯 **优势**：
   * - 直接渲染到屏幕，无中间合成步骤
   * - 延迟 <10ms（相比离屏模式的 20-30ms）
   * - 支持 VSync 同步，消除画面撕裂
   * 
   * 示例：
   * ```typescript
   * const handle = await nativerender.createWithSurface(
   *   this.surfaceId,
   *   1920,
   *   1080,
   *   PixelFormat.RGBA
   * );
   * ```
   */
  export function createWithSurface(
    surfaceId: string,
    width: number,
    height: number,
    format: PixelFormat
  ): Promise<number>;

  /**
   * 注册 XComponent Surface
   * 
   * ⚠️ **已废弃**：NativeWindow 现在由 C++ 层自动创建
   * 此接口保留仅为向后兼容，调用无效
   * 
   * @deprecated 使用 createWithSurface 即可，无需手动注册
   */
  export function registerSurface(surfaceId: string): void;

  /**
   * 注销 XComponent Surface
   * 
   * ⚠️ **已废弃**：NativeWindow 现在由 C++ 层自动管理
   * 此接口保留仅为向后兼容，调用无效
   * 
   * @deprecated 渲染器销毁时会自动清理资源
   */
  export function unregisterSurface(surfaceId: string): void;

  /**
   * 渲染帧
   * 
   * @param handle 渲染器句柄
   * @param pixelData 像素数据（ArrayBuffer，RGBA格式）
   * @param width 宽度
   * @param height 高度
   * @returns Promise，渲染完成后resolve
   * 
   * ⭐ 零拷贝优化：
   * - NAPI通过napi_get_arraybuffer_info获取ArrayBuffer指针
   * - Native层直接使用指针进行glTexSubImage2D
   * - 无中间拷贝，无内存分配
   */
  export function renderFrame(
    handle: number,
    pixelData: ArrayBuffer,
    width: number,
    height: number
  ): Promise<void>;

  /**
   * 调整渲染尺寸
   * 
   * @param handle 渲染器句柄
   * @param width 新宽度
   * @param height 新高度
   * @returns Promise，调整完成后resolve
   */
  export function resize(
    handle: number,
    width: number,
    height: number
  ): Promise<void>;

  /**
   * 销毁渲染器
   * 
   * @param handle 渲染器句柄
   * @returns Promise，销毁完成后resolve
   */
  export function destroy(handle: number): Promise<void>;
}
