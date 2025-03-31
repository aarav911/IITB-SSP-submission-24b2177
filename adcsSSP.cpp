#include <cstdint>

#include <cmath>

#include <array>
//here i am assuming that we will be using freeRTOS(though i am not using multitasking features of RTOS)
//and i am assuming that we are using ARM cortex series microprocessor(and not an arduino type processor, thus i am not using setup() and loop() functions typically found in arduino code) this is pure embedded c++ implementation.
//Hardware Abstraction layer
class NonVolatileMemory {
    public: struct ADCSState {
        ADCSMode current_mode;
        uint32_t mode_entry_time; //time at which that state was saved(these special data types are used to be more memory efficient)
        std::array < float, 3 > angular_velocity; //in all 3 directions
        float power_level;
        float timestamp; //this is store the time the state was saved/written in memory
        float checksum;
        static ADCSState read_persistent_state() {
            /* NVM read implementation */ }
    };

    public: static ADCSState read_persistent_state() {
        /* NVM read implementation */ }
    static void write(const ADCSState & state) {
        /* NVM write implementation */ }
    void save_persistent_state(ADCSState state) {
        NonVolatileMemory::write((ADCSState) state);
    }
};

enum class ADCSMode: uint8_t { //this stores the mode the ADCS currently is in
    DETUMBLING,
    SUN_ACQUISITION,
    NOMINAL_POINTING,
    SAFE_MODE,
    FAULT_RECOVERY
};

struct ADCSState {
    ADCSMode current_mode;
    uint32_t mode_entry_time; //time at which that state was saved
    std::array < float, 3 > angular_velocity; //in all 3 directions
    float power_level;
    static ADCSState read_persistent_state() {
        /* NVM read implementation */ }
};

class FaultManager {
    public: enum class FaultType {
        NONE,
        HIGH_ANGULAR_RATE,
        LOW_POWER,
        SENSOR_ANOMALY,
        CRITICAL,
        SOFTWARE_RESET_REQUIRED
    };

    FaultType check_faults(const ADCSState & state) {
        if (check_angular_rate(state)) return FaultType::HIGH_ANGULAR_RATE;
        if (check_power_level(state)) return FaultType::LOW_POWER;
        if (check_sensors()) return FaultType::SENSOR_ANOMALY;
        return FaultType::NONE;
    }

    private: bool check_angular_rate(const ADCSState & state) {
        constexpr float MAX_ANGULAR_RATE = 0.1f; // rad/s
        return (std::abs(state.angular_velocity[0]) > MAX_ANGULAR_RATE) ||
            (std::abs(state.angular_velocity[1]) > MAX_ANGULAR_RATE) ||
            (std::abs(state.angular_velocity[2]) > MAX_ANGULAR_RATE);
    }

    bool check_power_level(const ADCSState & state) {
        constexpr float LOW_POWER_THRESHOLD = 4.0f; // Watts
        return state.power_level < LOW_POWER_THRESHOLD;
    }

    bool check_sensors() {
        /* Sensor consistency check implementation */
        return false;
    }
};

class StateMachine {
    public:
    ADCSState current_state;
    FaultManager fault_checker;
    WatchdogTimer watchdog;

    StateMachine() { //default constructor to Loads the last saved state from non-volatile memory (so the satellite resumes from its last mode after a reset).
        //Initializes the watchdog timer to prevent system failures.
        current_state.read_persistent_state();
        //check if the current state is corrupted of not
        if (is_corrupt(current_state.current_mode)) {
            //using the checksum logic we check if the state data is corrupt or not
            current_state.current_mode = ADCSMode::SAFE_MODE;
        } else if (!is_state_safe(current_state.current_mode)) {
            current_state.current_mode = ADCSMode::SAFE_MODE;
        } else {
            current_state.current_mode = ADCSMode::DETUMBLING; //as we start from detumbling.
        }
        watchdog.initialize();
    }

    void run_cycle() {
        //this function is run continuously by the main's while(1) loop
        update_sensor_data();
        check_state_transition();
        execute_mode_entry(current_state.current_mode);
        check_for_software_reset();
        check_for_hardware_reset();
        manage_faults();
        watchdog.refresh_watchdog(); //if we get stuck in any of the 4 functions we get a reset. 
    }

    void update_sensor_data() {
        current_state.angular_velocity = read_imu();
        current_state.power_level = read_power_system();
    }

    void check_state_transition() {
        const ADCSMode new_mode = evaluate_transition_conditions();

        if (new_mode != current_state.current_mode) {
            execute_mode_exit(current_state.current_mode);
            current_state.current_mode = new_mode;
            current_state.mode_entry_time = get_current_time();
            execute_mode_entry(new_mode);
            save_persistent_state(); //save the state after every mode change
        }
    }

    ADCSMode evaluate_transition_conditions() {
        ADCSMode previous_operational_mode = current_state.current_mode;
        // Simplified transition logic
        switch (current_state.current_mode) {
        case ADCSMode::DETUMBLING:
            if (is_angular_rate_stable()) return ADCSMode::SUN_ACQUISITION;
            break;

        case ADCSMode::SUN_ACQUISITION:
            if (sun_vectors_aligned()) return ADCSMode::NOMINAL_POINTING;
            break;

        case ADCSMode::NOMINAL_POINTING:
            // Normal operation check
            break;

        case ADCSMode::SAFE_MODE:
            if (power_restored()) return previous_operational_mode;
            break;

        case ADCSMode::FAULT_RECOVERY:
            if (fault_recovery_complete()) return previous_operational_mode;
            break;
        }
        return current_state.current_mode;
    }

    void manage_faults() {
        const auto fault = fault_checker.check_faults(current_state); //auto allows it to automatically infer the datatype
        if (fault != FaultManager::FaultType::NONE) {
            handle_fault(fault);
        }
    }

    void handle_fault(FaultManager::FaultType fault) {
        ADCSMode previous_operational_mode = current_state.current_mode;

        previous_operational_mode = current_state.current_mode;

        switch (fault) {
        case FaultManager::FaultType::HIGH_ANGULAR_RATE:
            engage_magnetorquers();
            current_state.current_mode = ADCSMode::DETUMBLING;
            break;

        case FaultManager::FaultType::LOW_POWER:
            power_system_slowdown();
            current_state.current_mode = ADCSMode::SAFE_MODE;
            break;

        case FaultManager::FaultType::SENSOR_ANOMALY:
            reset_sensor_array();
            break;
        case FaultManager::FaultType::SOFTWARE_RESET_REQUIRED:
            execute_software_reset();
            break;
        case FaultManager::FaultType::CRITICAL:
            execute_hardware_reset();
            break;

        }
    }

    void execute_mode_entry(ADCSMode mode) {
        execute_state_behavior(mode);
    }

    void execute_mode_exit(ADCSMode mode) {
        /*mode exit logic depending on the hardware(turns off the mode specific actions)*/
    }

    void execute_state_behavior(ADCSMode mode) {
        switch (mode) {
        case ADCSMode::DETUMBLING:
            run_detumbling();
            break;
        case ADCSMode::SUN_ACQUISITION:
            run_sun_acquisition();
            break;
        case ADCSMode::NOMINAL_POINTING:
            run_nominal_pointing();
            break;
        case ADCSMode::SAFE_MODE:
            run_safe_mode();
            break;
        default:
            break;
        }
    }

    bool is_corrupt(ADCSMode mode) {
        /*the logic is to store the sum of all state parameters in checksum when we are writing the state in the memory, and then when we read it, we calculate checksum again, and check if the value of checksum has changed. if it has changes that means that the state was corrupted and thus we must start again*/
    }

    bool is_state_safe(ADCSMode mode) {
        //maybe the current state of the satellite is such that the angular velocity is very high, but the last saved state was Nominal pointing... clearly we cant run nominal pointing mode with high angular velocity thus we much check if the last state is safe to be implemented, or else start from the beginning. 
    }

    void save_persistent_state() {
        NonVolatileMemory::ADCSState nvm_state {
            .current_mode = (current_state.current_mode),
                .timestamp = current_state.mode_entry_time
        };
        NonVolatileMemory::write(nvm_state);
    }

    // Hardware interaction placeholders
    std::array < float,3 > read_imu() {
        /* IMU read implementation */
        return {};
    }
    float read_power_system() {
        /* EPS read implementation */
        return 0.0f;
    }
    void engage_magnetorquers() {
        /* Actuator control */ }
    uint16_t get_current_time() {
        /*code to fetch time*/ }
    void angular_rate_stable() {
        /*check current_state.angular_velocity according to appropriate data*/ }
    bool is_angular_rate_stable() {}
    bool sun_vectors_aligned() {}
    bool power_restored() {}
    bool fault_recovery_complete() {} //returns true if fault recovery is complete
    void reset_sensor_array() {
        /*software reset implementation*/ }
    void power_system_slowdown() {
        /*The EPS will be adjusted. to power only the most important things*/ }
    void execute_software_reset() {
        /*the implementation is complicated, but we will be reseting the sensors by rebooting their drivers*/ }
    void execute_hardware_reset() {
        /*the implementation is complicated. but my idea is to have another external circuit or microcontroller to whom we give a command to turn off the power to the main circuit, and then turn it on again after a delay*/ }
        public:
    void check_for_software_reset(){}//carefully decide the conditions for this reset and run execute_software_reset
    void check_for_hardware_reset(){}//carefully decide the conditions for this reset and run execute_hardware_reset



    //the functions that execute the specific modes...
    void run_detumbling() {
        //detumbling logic
        return;//once done
    }
    void run_safe_mode() {
        //one thing to note is that before entering this mode we will save the state memory
        save_persistent_state();

        //safe mode logic

        
        if(fault_recovery_complete()){
            return;//return back to fault checking and state transition mechanisms
        }
    }
    void run_nominal_pointing() {
        //nominal pointing logic
        return;//once done
    }
    void run_sun_acquisition() {
        //sun acquisition logic
        return;//once done
    }
    //these functions have all the implementation, and when that implementation is done, we simply go back to the run_cycle() function, and then we check the fault... so the faults are checked after we implement the whole mode.

};

class WatchdogTimer {
    public:
        //implementation of the WDT(depending on which type of WDT we use)
        void initialize() {
            /*watchdog implementation*/
        }
    void refresh_watchdog() {
        /*watchdog implementation*/
    }
};

int main() {
    int main_loop_delay_period;
    StateMachine adcs;

    while (true) {
        adcs.run_cycle(); //we are running this function continuously 
        //here i am assuming that run_cycle will not take a substantial amount of time to execute
        //atleast it should take more that the difference between the main loop delay and WDT duration
        delay(main_loop_delay_period); // RTOS-compatible delay
    }

    return 0;
}

void delay(int a) {
    //delay mechanism implementation
}
