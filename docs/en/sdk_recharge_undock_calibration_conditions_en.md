# Conditions of Use and Troubleshooting for Recharge, Undock, and Dock Calibration

## 1. Purpose

This document is intended for field operation, delivery, and after-sales personnel. It describes the conditions that must be met before the robot performs recharge, undock, or dock calibration tasks, as well as the common causes and recommended actions for different abnormal symptoms.

This document is organized according to the current software logic. Internal software communication interfaces and node names are not covered; only externally observable and actionable conditions are described.

## 2. Conditions Common to All Three Tasks

Recharge, undock, and dock calibration all depend on four areas working normally at the same time: vision, dock communication, robot motion, and calibration data. If any one of these areas does not meet the requirements, the task may fail to start or may stop during execution.

### 2.1 Site and Equipment Conditions

- The robot power supply is normal, and the robot is not in an emergency-stop, fallen, serious hardware fault, or motion-prohibited state.
- The robot chassis, legs, charging contacts, and dock mechanical structure have no obvious deformation, looseness, or jamming.
- The dock is placed securely. The robot can approach or leave the dock from its current direction, and the dock must not move during task execution.
- There are no people, pets, miscellaneous objects, wall corners, or other obstacles in front of the dock or along the undock direction that could affect motion.
- The camera lens is clean, with no dust, condensation, obstruction, protective-film residue, or obvious reflections.
- Site lighting is sufficient and stable. Insufficient light, backlighting, flicker, or an obstructed fill light may cause unstable recognition.

### 2.2 Dock Recognition Conditions

- The target dock marker must be visible in the camera field of view, and it must be sufficiently large, clear, and unobstructed.
- At any given time, only one target marker matching the current task may appear in the effective field of view.
- Multiple identical target markers must not be placed in the camera field of view at the same time. Even if they belong to different docks, identical IDs make the target position impossible to determine uniquely.
- Other markers with different IDs are normally not treated as the target, but in the field it is still recommended to keep only the target dock in view to avoid misjudgment and occlusion.
- There must be no reflective objects, glass, mirrors, or fast-moving objects passing between the robot and the dock.

The requirement that there be “only one QR code” is more accurately stated as: “Only one dock marker matching the current configuration may be present in the camera’s effective field of view.” Not every QR code in the environment needs to be removed, but during actual delivery, the target dock marker should ideally be the only dock marker visible.

### 2.3 Dock Communication Conditions

- The robot has selected and paired with the correct dock. It must not be connected to another dock or to a stale connection from a previous dock.
- The wireless connection between the dock and robot is stable, and dock status can be updated continuously.
- The dock is powered and can respond to operations such as turning on the light, stopping charging, and starting charging.
- The dock has no persistent hardware, communication, temperature, or charging fault.
- After replacing the dock, changing its address, or replacing its mainboard, confirm that the robot is using the configuration and calibration data corresponding to that dock.

Recharge and dock calibration require the dock to be paired and connected normally. Undock uses “dock connected” as the normal path, but the current version also supports a direct undock path: after receiving a fresh and explicit “dock disconnected” status, the system skips dock control and directly enters the stand-up and reverse-motion stages.

### 2.4 Calibration Data and Motion Conditions

- Camera intrinsic parameters, camera mounting parameters, and dock-position calibration data must exist, be readable, and be valid.
- Calibration data must match the current robot, camera, and dock. Directly copying parameters from another robot or dock may cause continuous deviation or a collision with the dock.
- At the start of recharge, the robot should be standing in a posture from which it can recognize the dock. It must not start while fallen, severely misaligned, or blocked by an obstacle.
- The motion-alignment module requires continuous, fresh dock-pose data. Once visual updates stop, the robot stops producing valid motion commands, and the task may time out or fail.
- Contact alignment does not mean that the robot has already touched the dock. The robot must also complete fine alignment, and the dock must confirm actual contact.

## 3. Recharge Task

### 3.1 Normal Execution Conditions

For recharge to complete normally, all of the following conditions should be met:

1. The target dock is paired and connected, and its status is stable.
2. The dock marker is clearly visible, with no duplicate target marker in the field of view.
3. The camera and dock calibration data used by the current robot are correct.
4. The robot is able to stand and move, and there are no obstacles in front of the dock.
5. The initial distance and angle between the robot and dock are within the range covered by visual alignment.
6. The dock contacts, charging power supply, and charging-status feedback are normal.
7. Battery status can be updated normally; the battery data must not be missing or obviously abnormal.

### 3.2 Normal Procedure

Recharge generally proceeds in the following order:

1. Check calibration data, visual capability, and motion-alignment capability.
2. Turn on the dock fill light according to the configuration, and adjust the robot to a standing posture.
3. Enable dock recognition, recognize the target marker, and calculate the relative position between the robot and dock.
4. Complete coarse alignment, fine alignment, and contact alignment in sequence.
5. Perform the final approach toward the dock and wait for physical contact confirmation.
6. Request the dock to start charging and wait for the dock to actually enter the charging state.
7. Stop visual tracking and the fill light when they are no longer needed, and enter charging monitoring.
8. Continuously monitor dock and battery status during charging. After the battery is full, remain on the dock and wait for the user to initiate an undock task or another valid state transition.

Acceptance of the command by the dock does not mean that charging has started. Recharge is successful only when the charging state has taken effect and continues to update.

The SDK can use `phase` from `OnTaskStateData()` to display this progress:
`recharge_preparing` for preparation, `recharge_docking` for alignment and dock
entry, `recharge_charging` for charging monitoring, and `recharge_completed` for
full-charge holding. `phase` may be empty or contain a future value and must not
replace `task_status` when determining the task result.

### 3.3 Recharge Symptoms, Causes, and Actions

| Symptom | Common causes | Recommended actions |
| --- | --- | --- |
| Vision or calibration is reported unavailable immediately | Camera calibration file is missing, unreadable, or corrupted; camera parameters do not match the current device | Check camera and dock calibration data and confirm that the files are from the current robot; recalibrate if necessary |
| Robot cannot find the dock at all | Marker is obstructed, lighting is insufficient, camera is dirty, image is not updating, or the robot is facing too far away | Clean the lens, improve lighting, remove obstructions, place the robot facing the target dock, and retry |
| Robot occasionally sees the dock, but the task still fails | Marker is too small, image is blurred, reflections are present, the target appears intermittently, or pose updates are discontinuous | Increase the dock’s size in the image, stabilize lighting and the dock, and prevent people from passing in front |
| Target is reported as non-unique or the position is unstable | Camera sees two identical target markers, or multiple docks use the same target ID | Keep only the target dock in view; check for duplicate markers and confirm that target IDs are planned correctly |
| Robot sees the dock but keeps correcting left/right or forward/backward and cannot approach | Dock-position calibration is inaccurate, dock was moved, ground is uneven, or the initial robot-pose error is too large | Secure the dock again and recalibrate; place the robot in a more appropriate initial position and retry |
| Robot does not move before or after entering the contact stage | Visual pose is stale, alignment error is still too large, target marker was temporarily lost, or motion control is in a protective state | Ensure continuous recognition, clear obstacles, confirm that the robot is not in emergency stop, and restart from a position facing the dock |
| Robot is close to the dock but contact is never confirmed | Charging-contact height or lateral position is unsuitable, contacts are dirty, dock is tilted, or robot posture is abnormal | Check and clean the contacts, check dock leveling and fixation, and confirm the robot’s final posture |
| Contact has occurred but charging does not start | Dock connection is lost, dock is not paired with the current robot, charging power is abnormal, contact is poor, or the dock reports an internal error | Confirm dock connection and status first, then check power, contacts, and dock fault messages; do not repeatedly send charging commands in rapid succession |
| Charging starts but fails shortly afterward | Dock connection or status feedback is interrupted, dock loses power, a charging fault occurs, or battery status stops updating | Check dock power and wireless connection, confirm that charging status persists, and undock before retrying recharge if necessary |
| Task shows that the battery is full but the robot remains on the dock | This is normal holding behavior; the robot does not automatically leave the dock after reaching full charge | Start a separate undock task when the robot needs to leave the dock |

## 4. Undock Task

### 4.1 Normal Execution Conditions

For undock to complete normally, all of the following conditions should be met:

1. The robot is actually on the target dock and is not currently executing another motion task.
2. During normal undock, the dock connection is stable, the fill light can be controlled, and the state change after charging stops can be confirmed. For direct undock after disconnection, the system must receive a fresh and explicit “dock disconnected” status; missing or stale status does not count as disconnected.
3. The target dock marker is visible and unique; two identical target markers must not appear in the camera field of view.
4. The undock direction and surrounding area are clear, with no obstacles along the robot’s reverse path.
5. Camera and dock-position calibration are correct, and the robot can continuously see the dock and calculate the relative pose.
6. The robot can recover from the charging posture to a standing posture, and the legs, chassis, and motion control are normal.
7. If undock collision detection is enabled in the product configuration, the relevant distance sensors must work normally and provide continuous data.

### 4.2 Normal Procedure

Undock generally proceeds in the following order:

1. Check calibration data and visual capability.
2. Start dock recognition and confirm that the target dock can be recognized continuously.
3. Determine the latest dock connection status and use one of the following paths:
   - Dock is explicitly disconnected: skip the fill light, stopping-charging, and post-stop stationary-wait stages, and directly enter standing and reverse motion.
   - Dock is connected, connection status is unknown, or status is stale: turn on the fill light, request charging to stop and confirm that it has stopped, remain stationary briefly, and then stand up.
4. Recover to a standing posture.
5. Reverse away based on the dock pose while continuously correcting direction and lateral deviation.
6. Stop motion after reaching the preset safe undock position, and release the recognition and fill-light resources used by this task.

For the normal connected path, success means that dock communication is complete, charging has stopped, the robot is standing, the preset undock position has been reached, and the robot has stopped moving. The direct-disconnect path does not wait for the fill-light or charging-stop result; instead, its conditions are that disconnection has been confirmed, the robot is standing, the preset undock position has been reached, and the robot has stopped moving.

The SDK can use `phase` from `OnTaskStateData()` to display this progress:
`undock_preparing` for preparation, `undock_undocking` for reverse motion, and
`undock_completed` for completion. Final success is still determined by
`task_status == SUCCESS`; clients must not require a non-empty `phase`.

### 4.3 Direct Undock When the Dock Is Disconnected

The current version supports “direct undock after explicit dock disconnection.” “Disconnected” has strict requirements: the system must have just received valid dock status that explicitly indicates disconnection. Missing status, stale status, or status still indicating a connection will not enter the direct-undock path.

The actual sequence for direct undock after disconnection is:

1. The system still checks calibration data, dock recognition, and undock capability.
2. After confirming explicit disconnection, it skips turning on the fill light, stopping charging, confirming the result, and the stationary wait after charging stops.
3. The robot stands up directly, then reverses to the safe undock position using continuous visual localization.
4. At task completion, the system still attempts to send cleanup notifications to stop charging and turn off the fill light, but it does not wait for a dock response or block the direct-undock result.

Therefore, direct undock after disconnection means “skip dock interactions in the main procedure that require result confirmation,” not “skip all prerequisite checks and immediately reverse blindly.” Visual recognition, standing confirmation, obstacle protection, and convergence to the undock position are still required.

If the dock is actually disconnected but the system has not received a fresh disconnection status, it conservatively attempts the original dock interactions; those interactions may fail and cause the task to fail. In the field, wait for the dock status to refresh, or restore the connection and use the normal undock path.

### 4.4 Undock Symptoms, Causes, and Actions

| Symptom | Common causes | Recommended actions |
| --- | --- | --- |
| Task fails immediately after starting | Calibration data is missing or unreadable; robot state does not permit motion | Check robot status and calibration data, clear emergency stop or faults, and retry |
| No dock marker is recognized | Robot is blocked by the dock, lens is too close, lighting is insufficient, or the marker is blocked by contacts or dock structure | Confirm that the camera can see the target marker, improve lighting, and check the lens and dock installation position |
| Task fails when multiple docks or identical markers are nearby | Target is non-unique and the system cannot determine which dock to undock from | Move non-target docks out of view or cover their markers; ensure target IDs are not duplicated across docks |
| Marker is visibly present but recognition still fails | Recognition results are not updated continuously, image is blurred, marker is too small, or reflections are present; the software version may also have an issue passing recognition results | Stabilize the robot and lighting and retry; if “visible but not accepted” repeats, record a field video and task time for after-sales analysis |
| Robot stands and reverses immediately after dock disconnection | System received a fresh and explicit dock-disconnected status and entered the direct-undock path | This is expected behavior in the current version; confirm that the undock direction is clear and visual recognition remains valid |
| Dock is actually disconnected, but task still tries to turn on the light or stop charging and then fails | Disconnection status was not updated in time, status is stale, or system still considers the dock connected | Wait for status to refresh; restore the dock connection and start a complete undock task if necessary |
| Robot remains on the dock and charging does not stop | Dock connection is lost, dock rejects the stop-charging request, dock status is not updated, or dock still reports a fault | Check dock connection, power, and fault messages; continue only after charging status changes to stopped |
| Robot cannot stand after charging stops | Leg or chassis fault, an external object is pressing against the robot, or robot is not in the expected charging posture | Clear the surrounding area, check mechanical and safety status, and perform maintenance before retrying if necessary |
| Robot suddenly stops during undock | Visual pose is lost, an obstacle is detected, sensor data is interrupted, dock was moved, or robot deviated from the path | Do not push the robot; clear people and obstacles, restore sensor and visual conditions, and retry |
| Obstacle is continuously reported and undock cannot complete | The undock area really contains an obstacle, or collision sensors are obstructed, dirty, or reporting abnormal data | Clear the undock area and clean the sensors; retry after confirming that the area is clear |
| Robot has left the dock but task has not completed | Preset undock position has not been reached, pose error has not converged stably, or recognition was lost during execution | Return the robot to a visible, unobstructed position and execute again; do not move the robot by force during motion |

### 4.5 Field Requirement: “Only One QR Code”

Undock is particularly sensitive to non-unique targets. The customer site should follow these rules:

- Keep only one target marker for the current dock within the camera’s visible range.
- Do not use the same target ID on adjacent docks.
- Do not allow mirrors, screens, printed materials, or other reflective objects to produce target-like images.
- If multiple docks are present, confirm before undocking that the robot is connected to the correct dock, and keep other docks out of the camera field of view.

Therefore, “only one dock may be nearby” is the safest field requirement. Algorithmically, what must actually be unique is the target marker and target dock in the camera field of view.

## 5. Dock Calibration Task

### 5.1 Normal Execution Conditions

For dock calibration to complete normally, all of the following conditions should be met:

1. The dock is powered and paired and connected to the current robot.
2. The system can identify the valid identity of the current dock. After replacing a dock, do not continue using an old dock identity that has not been verified.
3. Camera intrinsic parameters and camera mounting parameters are prepared; dock calibration cannot replace the camera’s basic calibration.
4. The robot and dock remain stationary. Do not push the robot, adjust the camera, or move the dock during calibration.
5. The camera can continuously and clearly see the unique target marker for a short period.
6. The target marker is not obstructed, reflective, severely tilted, or affected by motion blur.
7. Storage space and configuration-file permissions are normal, and the system can write the new dock-calibration result.

### 5.2 Normal Procedure

Dock calibration generally proceeds in the following order:

1. Start visual recognition.
2. Acquire multiple consecutive frames of stable dock poses within the specified time. The current logic requires five valid results in succession within a short period.
3. Trigger manual dock calibration and calculate the relative dock position using the currently recognized target.
4. Save the result in the calibration data corresponding to the current dock identity and load it for subsequent localization tasks.
5. Check that the file can be read, and immediately perform a recharge or undock verification to validate the actual result.

The necessary condition for calibration success is not “the QR code was seen once,” but rather “the unique target was recognized continuously and stably, the dock identity was valid, the calculated result was reasonable, and the result was saved successfully.”

### 5.3 Calibration Symptoms, Causes, and Actions

| Symptom | Common causes | Recommended actions |
| --- | --- | --- |
| Calibration task cannot start | Dock is not connected, dock identity is invalid, basic camera calibration is missing, or an existing calibration task has not ended | Confirm dock pairing and connection and basic camera calibration, then start calibration again |
| Stable recognition has not succeeded after about 5 seconds | Target marker cannot be seen, image is blurred, lighting is unstable, or the robot or dock is moving | Keep both stationary, clean the lens, improve lighting, and ensure the complete target marker is in view |
| Duplicate target or unstable pose is reported | Multiple identical target markers appear in one image, or similar patterns and reflections are nearby | Keep only one target marker, remove duplicates and reflective objects, and recalibrate |
| Recognition continues during calibration but the task ultimately fails | Recognition-pose quality is poor, camera mounting parameters are incorrect, or target distance or angle is unsuitable | Adjust the relative robot-dock position, verify camera mounting parameters, and retry |
| Calibration command is accepted but calibration ultimately fails | Acceptance only means processing has started; the task may later end because the target was lost, calculation failed, or file writing failed | Check the final task result; do not judge success solely from the “started” notification |
| File write or read failure is reported | Insufficient storage, abnormal file permissions, read-only file system, or corrupted original file | Check storage and the file system; keep a backup of the original file and have maintenance personnel handle it |
| Calibration reports success, but recharge direction is obviously wrong | Result belongs to another dock, dock was moved afterward, or the current task did not use the newly calibrated data | Confirm that the connected dock matches the calibration target, secure the dock, recalibrate, and verify with a recharge task |

### 5.4 Multi-Dock Requirements

A single robot may operate with different docks, but each dock must have its own calibration result, and each result must be bound to that dock’s identity. The site should follow these requirements:

- During calibration, keep only the current dock’s target marker in the camera’s effective field of view.
- Before calibration, confirm that the currently connected dock is the one to be calibrated.
- After replacing the dock, first confirm that the system has identified the new dock, then check or regenerate the calibration result for that dock.
- Do not use dock A’s calibration result to verify dock B, and do not simply copy one generic file over all docks.
