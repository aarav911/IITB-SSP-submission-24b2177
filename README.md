Here’s a formal and polished version of your README file, formatted for a GitHub repository:

---

# IITB-SSP-Submission-24b2177

This repository contains the boilerplate code for the **Attitude Determination and Control System (ADCS)** subsystem of the **IotaSat CubeSat**. The ADCS is implemented as a modular and robust state machine in embedded C++, designed to operate on an ARM Cortex series microprocessor. While the system is compatible with FreeRTOS, this implementation does not utilize multitasking features.

---

## Overview

The ADCS subsystem is responsible for determining and controlling the satellite's orientation in space. It achieves this by managing various operational modes, handling faults, and ensuring safe state transitions based on real-time sensor data.

Key features include:
- A robust state machine for ADCS control.
- Persistent state storage and retrieval using Non-Volatile Memory (NVM).
- Fault detection and recovery mechanisms.
- Integration with a watchdog timer (WDT) for system safety.
- Three types of resets, to be safe from a wide variety of problems.
- Modular design with hardware abstraction for easy extensibility.

---

## Modes of Operation

The ADCS operates in five distinct modes:

| **Mode**             | **Primary Action**                              | **Trigger Condition**                  | **Exit Criteria**                        |
|-----------------------|------------------------------------------------|----------------------------------------|------------------------------------------|
| **DETUMBLING**        | Reduces angular velocity using magnetorquers.  | Initial state after reset.             | Angular velocity < 5°/s.                 |
| **SUN_ACQUISITION**   | Aligns the satellite with the Sun.             | Stable rotation achieved.              | Sun vector within 2° tolerance.          |
| **NOMINAL_POINTING**  | Maintains a stable pointing mode.              | payoad operations initiated.          | Power < 20% or fault detected.           |
| **SAFE_MODE**         | Reduces power consumption and ensures safety.  | Power or thermal limits exceeded.      | Normal parameters restored.              |
| **FAULT_RECOVERY**    | Handles detected faults before resuming ops.   | Watchdog timeout or sensor anomalies.  | Subsystem checks passed.                 |

---

## System Workflow

### Startup Sequence
1. Retrieve the last saved state from NVM and validate its integrity using a checksum.
2. If the saved state is corrupt or unsafe, default to the DETUMBLING mode.
3. Initialize all sensors and perform diagnostics.

### Main Loop (`run_cycle()`)
The `run_cycle()` function executes continuously and serves as the heart of the ADCS logic:
1. Acquire real-time sensor data (e.g., IMU readings, power levels).
2. Evaluate current conditions and determine necessary state transitions.
3. Refresh the watchdog timer to ensure system safety.
4. Generate telemetry packets for communication with ground stations.

---

## Reset Mechanisms

The system implements a hierarchical reset strategy to ensure fault recovery:

1. **Software Reset**:
   - Reinitializes all sensor drivers in case of anomalies.
   - Maintains power to critical subsystems.
   - Triggered by sensor CRC failures or minor faults.

2. **Hardware Reset**:
   - Executes a full power cycle via an external microcontroller.
   - Turns off the main circuit for a few seconds before restarting.
   - Triggered after three consecutive software resets (worst-case scenario).

3. **Watchdog Timer Reset**:
   - Uses a hardware watchdog timer (WDT) with a 1.6-second timeout.
   - Triggers a reset if `run_cycle()` fails to execute within the timeout period.

---

## Code Structure

The codebase is designed to be modular, object-oriented, and easy to extend:

- **Core Components**:
  - `StateMachine.cpp`: Implements state transition logic.
  - `FaultHandler.cpp`: Manages fault detection and recovery mechanisms.

### Key Functions
- `void run_cycle();`  
  Main control loop executed continuously (basically the `while(true)` in `main` runs it continuously, with a delay).
- `void update_sensor_data();`  
  Updates sensor readings.
- `void check_state_transition();`  
  Evaluates and transitions between states.
- `void manage_faults();`  
  Handles detected faults.


- **Design Patterns Used**:
  - Hardware Abstraction Layer (HAL) for sensor I/O operations.(NonVolatileMemory class)
  - Strategy pattern for mode-specific control algorithms.
  - Singleton pattern for managing NVM access.

---

## Features

- Implements a robust state machine for ADCS control.
- Stores and retrieves the last known state from NVM with integrity checks.
- Detects and handles various fault conditions, such as:
  - High angular velocity
  - Low power levels
  - Sensor anomalies
- Includes watchdog timer integration for enhanced system safety.
- Modular design ensures clean separation of concerns.

---

## Future Improvements

The following enhancements are planned for future iterations:

1. **FreeRTOS Integration**:
   - Introduce multitasking to improve modularity and scalability.
   - Separate tasks for sensor polling, control logic, and telemetry generation.

2. **Power Optimization**:
   - Implement dynamic clock scaling based on operational mode.
   - Enable sleep states for sensors during SAFE_MODE.

3. **Enhanced Fault Recovery**:
   - Add redundancy through sensor fusion techniques.
   - Use Kalman filters for anomaly detection and smoother transitions.

4. **Simulation Framework**:
   - Develop a hardware-independent testing framework using mock objects.

---

## Assumptions

1. The WDT reset directly restarts execution at `int main()`.
2. Hardware reset is managed by an external circuit/microcontroller that powers down the main board temporarily before restarting it.

---

## Contributing

Contributions are welcome! If you have suggestions or improvements, feel free to submit a pull request or open an issue in this repository.

---

## License

This project is licensed under [MIT License](LICENSE).

---

## Acknowledgments

Special thanks to [IIT Bombay Student Satellite Program](https://www.aero.iitb.ac.in/satlab/) for providing guidance and resources for this project.
This README was formatted by Perplexity AI, but i did recheck it several times. 

--- 

This README provides a professional and comprehensive overview of your project while maintaining clarity and accessibility for collaborators or contributors on GitHub!
