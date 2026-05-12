import hilog from "@ohos:hilog";
import UIAbility from "@ohos:app.ability.UIAbility";
import type AbilityConstant from "@ohos:app.ability.AbilityConstant";
import type Want from "@ohos:app.ability.Want";
import type window from "@ohos:window";
import type { BusinessError } from "@ohos:base";
class BjcCov {
    coverage: {
        [key: string]: string | number;
    };
    constructor(covData: object) {
        const gcv = "__coverage__";
        let coverage = globalThis[gcv] || (globalThis[gcv] = {});
        if (!coverage[covData.path] && true) {
            coverage[covData.path] = covData;
        }
        this.coverage = coverage[covData.path];
    }
    instrumentFunction(func: number) {
        this.coverage.functions[func].count++;
        this.coverage.functions[func].regions[0].count++;
    }
    instrumentRegion(func: number, region: number) {
        this.coverage.functions[func].regions[region].count++;
    }
    instrumentReturn(func: number, retIdx: number) {
        this.coverage.functions[func].returnes[retIdx].count++;
    }
    instrumentBranch(func: number, branch: number, trueOrFalse: boolean) {
        if (trueOrFalse) {
            this.coverage.functions[func].branches[branch].trueCount++;
            for (let r of Object.values(this.coverage.functions[func].branches[branch].group)) {
                if (r !== branch) {
                    this.coverage.functions[func].branches[r as number].falseCount++;
                }
            }
        }
        else {
            this.coverage.functions[func].branches[branch].falseCount++;
        }
    }
}
let bjccovmp2sjw5a = new BjcCov({ version: "bjc v1.0.0", versionCode: 10000, path: "entry/src/main/ets/entryability/EntryAbility.ets", hash: "8b11ea3f3a9e6220fb1e75a845c76232ec1f03807b3e68c10d23548d0d5e71f9", lineCnt: 67, count: 0, projectPath: "", functions: { 0: { name: "EntryAbility.onCreate", count: 0, regions: { 0: { startLoc: { line: 22, col: 3 }, endLoc: { line: 24, col: 4 }, count: 0, ignored: 0 }, 1: { startLoc: { line: 23, col: 5 }, endLoc: { line: 24, col: 4 }, count: 0, ignored: 0 } }, branches: {}, ignored: 0, index: 0 }, 1: { name: "EntryAbility.onDestroy", count: 0, regions: { 0: { startLoc: { line: 26, col: 3 }, endLoc: { line: 28, col: 4 }, count: 0, ignored: 0 }, 1: { startLoc: { line: 27, col: 5 }, endLoc: { line: 28, col: 4 }, count: 0, ignored: 0 } }, branches: {}, ignored: 0, index: 1 }, 2: { name: "EntryAbility.onWindowStageCreate", count: 0, regions: { 0: { startLoc: { line: 30, col: 3 }, endLoc: { line: 50, col: 4 }, count: 0, ignored: 0 }, 1: { startLoc: { line: 32, col: 5 }, endLoc: { line: 50, col: 4 }, count: 0, ignored: 0 }, 2: { startLoc: { line: 39, col: 7 }, endLoc: { line: 40, col: 6 }, count: 0, ignored: 0 }, 3: { startLoc: { line: 37, col: 9 }, endLoc: { line: 38, col: 8 }, count: 0, ignored: 0 }, 4: { startLoc: { line: 48, col: 7 }, endLoc: { line: 49, col: 6 }, count: 0, ignored: 0 }, 5: { startLoc: { line: 46, col: 9 }, endLoc: { line: 47, col: 8 }, count: 0, ignored: 0 } }, branches: {}, ignored: 0, index: 2 }, 3: { name: "anonymous_0", count: 0, regions: { 0: { startLoc: { line: 34, col: 31 }, endLoc: { line: 40, col: 6 }, count: 0, ignored: 0 }, 1: { startLoc: { line: 35, col: 21 }, endLoc: { line: 38, col: 8 }, count: 0, ignored: 0 } }, branches: { 0: { startLoc: { line: 35, col: 11 }, endLoc: { line: 35, col: 19 }, trueCount: 0, falseCount: 0, group: {}, ignored: 0 } }, ignored: 0, index: 3 }, 4: { name: "anonymous_1", count: 0, regions: { 0: { startLoc: { line: 43, col: 44 }, endLoc: { line: 49, col: 6 }, count: 0, ignored: 0 }, 1: { startLoc: { line: 44, col: 21 }, endLoc: { line: 47, col: 8 }, count: 0, ignored: 0 } }, branches: { 0: { startLoc: { line: 44, col: 11 }, endLoc: { line: 44, col: 19 }, trueCount: 0, falseCount: 0, group: {}, ignored: 0 } }, ignored: 0, index: 4 }, 5: { name: "EntryAbility.onWindowStageDestroy", count: 0, regions: { 0: { startLoc: { line: 52, col: 3 }, endLoc: { line: 55, col: 4 }, count: 0, ignored: 0 }, 1: { startLoc: { line: 54, col: 5 }, endLoc: { line: 55, col: 4 }, count: 0, ignored: 0 } }, branches: {}, ignored: 0, index: 5 }, 6: { name: "EntryAbility.onForeground", count: 0, regions: { 0: { startLoc: { line: 57, col: 3 }, endLoc: { line: 60, col: 4 }, count: 0, ignored: 0 }, 1: { startLoc: { line: 59, col: 5 }, endLoc: { line: 60, col: 4 }, count: 0, ignored: 0 } }, branches: {}, ignored: 0, index: 6 }, 7: { name: "EntryAbility.onBackground", count: 0, regions: { 0: { startLoc: { line: 62, col: 3 }, endLoc: { line: 65, col: 4 }, count: 0, ignored: 0 }, 1: { startLoc: { line: 64, col: 5 }, endLoc: { line: 65, col: 4 }, count: 0, ignored: 0 } }, branches: {}, ignored: 0, index: 7 } }, exeLine: { 0: 16, 1: 17, 2: 18, 3: 19, 4: 21, 5: 22, 6: 23, 7: 26, 8: 27, 9: 30, 10: 32, 11: 34, 12: 35, 13: 36, 14: 37, 15: 39, 16: 43, 17: 44, 18: 45, 19: 46, 20: 48, 21: 52, 22: 54, 23: 57, 24: 59, 25: 62, 26: 64, 27: 66 } });
export default class EntryAbility extends UIAbility {
    onCreate(want: Want, launchParam: AbilityConstant.LaunchParam) {
        bjccovmp2sjw5a.instrumentFunction(0);
        bjccovmp2sjw5a.instrumentRegion(0, 1);
        hilog.info(0x0000, 'testTag', '%{public}s', 'Ability onCreate');
    }
    onDestroy() {
        bjccovmp2sjw5a.instrumentFunction(1);
        bjccovmp2sjw5a.instrumentRegion(1, 1);
        hilog.info(0x0000, 'testTag', '%{public}s', 'Ability onDestroy');
    }
    onWindowStageCreate(windowStage: window.WindowStage) {
        bjccovmp2sjw5a.instrumentFunction(2);
        bjccovmp2sjw5a.instrumentRegion(2, 1);
        // Main window is created, set main page for this ability
        hilog.info(0x0000, 'testTag', '%{public}s', 'Ability onWindowStageCreate');
        windowStage.getMainWindow((err: BusinessError, data) => {
            bjccovmp2sjw5a.instrumentFunction(3);
            if (err.code) {
                bjccovmp2sjw5a.instrumentBranch(3, 0, true);
                bjccovmp2sjw5a.instrumentRegion(3, 1);
                hilog.error(0x0000, 'testTag', `Failed to obtain the main window. Cause:${err.message}`);
                bjccovmp2sjw5a.instrumentRegion(2, 3);
                return;
            }
            else {
                bjccovmp2sjw5a.instrumentBranch(3, 0, false);
            }
            bjccovmp2sjw5a.instrumentRegion(2, 2);
            data.setWindowLayoutFullScreen(true);
        });
        windowStage.loadContent('pages/Index', (err, data) => {
            bjccovmp2sjw5a.instrumentFunction(4);
            if (err.code) {
                bjccovmp2sjw5a.instrumentBranch(4, 0, true);
                bjccovmp2sjw5a.instrumentRegion(4, 1);
                hilog.error(0x0000, 'testTag', 'Failed to load the content. Cause: %{public}s', JSON.stringify(err) ?? '');
                bjccovmp2sjw5a.instrumentRegion(2, 5);
                return;
            }
            else {
                bjccovmp2sjw5a.instrumentBranch(4, 0, false);
            }
            bjccovmp2sjw5a.instrumentRegion(2, 4);
            hilog.info(0x0000, 'testTag', 'Succeeded in loading the content. Data: %{public}s', JSON.stringify(data) ?? '');
        });
    }
    onWindowStageDestroy() {
        bjccovmp2sjw5a.instrumentFunction(5);
        bjccovmp2sjw5a.instrumentRegion(5, 1);
        // Main window is destroyed, release UI related resources
        hilog.info(0x0000, 'testTag', '%{public}s', 'Ability onWindowStageDestroy');
    }
    onForeground() {
        bjccovmp2sjw5a.instrumentFunction(6);
        bjccovmp2sjw5a.instrumentRegion(6, 1);
        // Ability has brought to foreground
        hilog.info(0x0000, 'testTag', '%{public}s', 'Ability onForeground');
    }
    onBackground() {
        bjccovmp2sjw5a.instrumentFunction(7);
        bjccovmp2sjw5a.instrumentRegion(7, 1);
        // Ability has back to background
        hilog.info(0x0000, 'testTag', '%{public}s', 'Ability onBackground');
    }
}
;
