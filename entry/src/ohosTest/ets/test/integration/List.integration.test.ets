import { describe } from '@ohos/hypium';

// 集成测试套件
import RenderPipelineTest from './pipeline/RenderPipeline.test';
import EGLContextLifecycleTest from './surface/EGLContextLifecycle.test';
import FullRenderFlowTest from './e2e/FullRenderFlow.test';

export default function integrationTests() {
  // Level 2: 渲染管线集成测试
  RenderPipelineTest();
  
  // Level 3: Surface 管理集成测试
  EGLContextLifecycleTest();
  
  // Level 4: 端到端集成测试
  FullRenderFlowTest();
}
