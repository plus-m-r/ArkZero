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
  export enum PixelFormat {
    RGBA = 0,   // RGBA8888 (4字节/像素)
    RGB = 1,    // RGB888 (3字节/像素)
    BGRA = 2,   // BGRA8888 (4字节/像素)
    NV21 = 3,   // YUV420半平面 (1.5字节/像素)
    NV12 = 4    // YUV420半平面 (1.5字节/像素)
  }

  /**
   * 创建渲染器
   * 
   * @param surfaceId XComponent 的 surface ID
   * @param width 宽度
   * @param height 高度
   * @param format 像素格式
   * @returns 渲染器句柄（number）
   * 
   * 🎯 **优势**：
   * - 直接渲染到屏幕，无中间合成步骤
   * - 延迟 <10ms
   * - 支持 VSync 同步，消除画面撕裂
   * 
   * 示例：
   * ```typescript
   * const handle = await nativerender.create(
   *   this.surfaceId,
   *   1920,
   *   1080,
   *   PixelFormat.RGBA
   * );
   * ```
   */
  export function create(
    surfaceId: string,
    width: number,
    height: number,
    format: PixelFormat
  ): Promise<number>;

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
   * 获取性能统计
   * 
   * @param handle 渲染器句柄
   * @returns JSON格式的性能数据字符串
   * 
   * 📊 **返回字段**：
   * - fps: 当前帧率（帧/秒）
   * - frameTime: 单帧平均耗时（毫秒）
   * - dropRate: 丢帧率（百分比）
   * - totalFrames: 总渲染帧数
   * - droppedFrames: 丢弃的帧数
   * 
   * 示例：
   * ```typescript
   * const stats = nativerender.getPerformanceStats(handle);
   * console.log('FPS:', JSON.parse(stats).fps);
   * ```
   */
  export function getPerformanceStats(handle: number): string;

  /**
   * 销毁渲染器
   * 
   * @param handle 渲染器句柄
   * @returns Promise，销毁完成后resolve
   */
  export function destroy(handle: number): Promise<void>;

  const nativerender: {
    create: typeof create;
    renderFrame: typeof renderFrame;
    resize: typeof resize;
    getPerformanceStats: typeof getPerformanceStats;
    destroy: typeof destroy;
    PixelFormat: typeof PixelFormat;
  };

  export default nativerender;
}
