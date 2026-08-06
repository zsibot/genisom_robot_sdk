## SDK Internal Motion State Flow

**Note: Switching from the locked state to any other state is considered an unlock operation.**

### SDK Posture Capability Matrix

> All postures support direct switching.

#### Posture List

| Posture ID | Posture Name | Description | Can Switch To | Notes |
|---------|---------|---------|---------|---------|
| StandUp | Stand Up | Robot stands and can move | Any posture | Supports movement via `Move` |
| BalanceStandUp | Balance Stand Up | Robot stands in place | Any posture | Supports head control via `ControlHead`<br>Supports body rolling via `Turn`<br>Supports body height adjustment via `HighLowStance` |
| LieDown | Lie Down | Prone posture | Any posture | - |
| Stair | Stair Mode | Standing posture for stair traversal | Any posture | Supports movement via `Move` |
| Crawl | Crawl | Crawling posture, stationary | Any posture | - |
| CrawlWalk | Crawl Walk | Crawling posture with movement | Any posture | Supports movement via `Move` |
| Climb | Climb | Climbing posture | Any posture | Supports movement via `Move` |
| Gait | Gait | Fine gait posture | Any posture | Supports movement via `Move` |
| Slim | Slim | Narrow passage posture | Any posture | Supports movement via `Move` |
| DSB | DSB | Door sill / barrier crossing posture | Any posture | Supports movement via `Move` |
| PosControl | Position Control | Position control posture | Any posture | Supports position control via `PosMove` |
| Locked | Locked | Locked posture | Any posture | Switching to any other posture performs unlock |
| SkWalk | SkWalk | Same-knee posture | Only `StandUp` | Supports movement via `Move` |

## SDK Command Delivery

The figure below shows two command delivery designs.

![Image](images/SDK-SendCmd.png)