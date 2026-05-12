import { describe, it, expect } from "@package:pkg_modules/.ohpm/@ohos+hypium@1.0.24/pkg_modules/@ohos/hypium/index";
/**
 * Local Test 入口文件（占位）
 *
 * 说明：
 * - Local Test 运行在本地 JVM，不支持 Native C++ 测试
 * - PerformanceMonitor 等 NAPI 测试请使用 Instrument Test (ohosTest)
 * - 此文件仅为满足构建系统要求
 */
export default function testsuite() {
    describe('Local Test Placeholder', () => {
        it('Placeholder test - add real tests here if needed', 0, () => {
            // 占位测试
            expect(true).assertTrue();
        });
    });
}
