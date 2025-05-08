/**
 * @file power_manager.h
 * @brief Power Management System for SkyMesh satellites
 *
 * Defines the interface for managing satellite power systems, including:
 * - Solar panel power generation
 * - Battery management
 * - Power distribution
 * - Power-saving modes
 * - Subsystem power control
 */

#ifndef SKYMESH_POWER_MANAGER_H
#define SKYMESH_POWER_MANAGER_H

#include <cstdint>
#include <vector>
#include <array>
#include <functional>
#include <memory>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <cstring> // for strcmp

namespace skymesh {
namespace core {
// Forward declarations
class RFController;
/**
 * @enum PowerMode
 * @brief Power modes for the satellite
 */
enum class PowerMode {
    NORMAL,         ///< Normal operation mode
    LOW_POWER,      ///< Low power mode, non-critical systems disabled
    CRITICAL,       ///< Critical power mode, minimal systems running
    EMERGENCY,      ///< Emergency power mode, only core survival systems active
    HIBERNATION     ///< Hibernation mode, most systems powered down
};

/**
 * @enum PowerSource
 * @brief Available power sources for the satellite
 */
enum class PowerSource {
    SOLAR_PANEL,    ///< Solar panel power generation
    BATTERY,        ///< Battery power
    BACKUP_BATTERY  ///< Backup battery power
};

/**
 * @enum SubsystemID
 * @brief Identifiers for satellite subsystems
 */
enum class SubsystemID {
    RF_SYSTEM,      ///< RF communication system
    OBC,            ///< On-board computer
    ADCS,           ///< Attitude determination and control
    THERMAL,        ///< Thermal control system
    PAYLOAD,        ///< Mission payload
    SENSORS         ///< Sensors array
};

/**
 * @struct PowerSourceStatus
 * @brief Status information for a power source
 */
struct PowerSourceStatus {
    PowerSource source;                  ///< Type of power source
    float currentVoltage;                ///< Current voltage in volts
    float currentCurrent;                ///< Current in amperes
    float temperature;                   ///< Temperature in degrees Celsius
    float stateOfCharge;                 ///< State of charge (for batteries, 0.0-1.0)
    std::chrono::system_clock::time_point lastUpdated; ///< Timestamp of last update
};

/**
 * @struct PowerConsumption
 * @brief Power consumption information for a subsystem
 */
struct PowerConsumption {
    SubsystemID subsystem;               ///< Subsystem identifier
    float averagePower;                  ///< Average power consumption in watts
    float peakPower;                     ///< Peak power consumption in watts
    float currentPower;                  ///< Current power consumption in watts
    bool isActive;                       ///< Whether the subsystem is currently active
};

/**
 * @struct PowerBudget
 * @brief Overall power budget for the satellite
 */
struct PowerBudget {
    float totalAvailable;                ///< Total available power in watts
    float totalConsumption;              ///< Total current consumption in watts
    float projectedAvailable;            ///< Projected available power (next orbit) in watts
    std::vector<PowerConsumption> subsystems; ///< Power consumption by subsystem
    PowerMode currentMode;               ///< Current power mode
    float batteryReserve;                ///< Battery reserve in watt-hours
    float solarInputRate;                ///< Current solar input rate in watts
};

/**
 * @class PowerManager
 * @brief Manages the satellite power system
 * 
 * Responsible for controlling power distribution, monitoring power sources,
 * managing subsystem power states, and implementing power-saving strategies.
 * Radiation-hardened design with redundancy and error detection/correction.
 */
class PowerManager {
    // Allow radiation testing class full access to test hardening features
    friend class RadiationHardeningTest;

protected:
    // Protected interface for radiation testing
    struct RadiationTestInterface {
        static void* getInternalStatePtr(PowerManager* pm, const char* memberName) {
            if (strcmp(memberName, "currentMode") == 0) return &pm->currentMode;
            if (strcmp(memberName, "subsystemStates") == 0) return &pm->subsystemStates;
            if (strcmp(memberName, "subsystemPowerLevels") == 0) return &pm->subsystemPowerLevels;
            return nullptr;
        }
    };

public:
    /**
     * @brief Constructor
     */
    PowerManager();
    
    /**
     * @brief Destructor
     */
    virtual ~PowerManager();

    /**
     * @brief Initialize the power management system
     * @param subsystems Vector of subsystem IDs to manage
     * @return True if initialization successful, false otherwise
     */
    bool initialize(const std::vector<SubsystemID>& subsystems);

    /**
     * @brief Set the power mode for the satellite
     * @param mode The desired power mode
     * @return True if mode change successful, false otherwise
     */
    bool setPowerMode(PowerMode mode);
    
    /**
     * @brief Get the current power mode
     * @return Current power mode
     */
    PowerMode getCurrentPowerMode() const;
    
    /**
     * @brief Enable a specific subsystem
     * @param subsystem The subsystem to enable
     * @param powerLevel Power level allocation (0.0-1.0)
     * @return True if subsystem enabled successfully, false otherwise
     */
    bool enableSubsystem(SubsystemID subsystem, float powerLevel = 1.0);
    
    /**
     * @brief Disable a specific subsystem
     * @param subsystem The subsystem to disable
     * @return True if subsystem disabled successfully, false otherwise
     */
    bool disableSubsystem(SubsystemID subsystem);
    
    /**
     * @brief Check if a subsystem is currently enabled
     * @param subsystem The subsystem to check
     * @return True if subsystem is enabled, false otherwise
     */
    bool isSubsystemEnabled(SubsystemID subsystem) const;
    
    /**
     * @brief Get the current power budget
     * @return Current power budget information
     */
    PowerBudget getPowerBudget() const;
    
    /**
     * @brief Get the status of a specific power source
     * @param source The power source to check
     * @return Status information for the specified power source
     */
    PowerSourceStatus getPowerSourceStatus(PowerSource source) const;
    
    /**
     * @brief Set a power level for a specific subsystem
     * @param subsystem The subsystem to set power for
     * @param level Power level (0.0-1.0)
     * @return True if power level set successfully, false otherwise
     */
    bool setSubsystemPowerLevel(SubsystemID subsystem, float level);
    
    /**
     * @brief Register a callback for power warning events
     * @param callback Function to call when power warning occurs
     * @return Identifier for the registered callback
     */
    uint32_t registerPowerWarningCallback(std::function<void(PowerMode)> callback);
    
    /**
     * @brief Unregister a previously registered callback
     * @param callbackId The identifier of the callback to unregister
     */
    void unregisterPowerWarningCallback(uint32_t callbackId);
    
    /**
     * @brief Update system with orbit information
     * @param timeInSunlight Predicted time in sunlight (seconds)
     * @param timeInEclipse Predicted time in eclipse (seconds)
     */
    void updateOrbitPowerProfile(uint32_t timeInSunlight, uint32_t timeInEclipse);
    
    /**
     * @brief Perform a health check on the power system
     * @return True if all systems are healthy, false otherwise
     */
    bool performHealthCheck();
    
    /**
     * @brief Reset the power management system
     * @param hardReset If true, perform a hard reset including hardware
     * @return True if reset successful, false otherwise
     */
    bool reset(bool hardReset = false);
    
    /**
     * @brief Handle radiation-induced errors
     * @return True if errors were corrected, false if uncorrectable errors
     */
    bool handleRadiationErrors();
    
    /**
     * @brief Set RF system power allocation for different communication modes
     * @param standardMode Power allocation for standard communications (0.0-1.0)
     * @param burstMode Power allocation for burst transmissions (0.0-1.0)
     * @param emergencyMode Power allocation for emergency comms (0.0-1.0)
     * @return True if allocations set successfully, false otherwise
     */
    bool setRFPowerAllocations(float standardMode, float burstMode, float emergencyMode);
    
    /**
     * @brief Prepare for RF transmission burst
     * @param durationMs Expected duration in milliseconds
     * @param powerLevel Required power level (0.0-1.0)
     * @return True if power allocation successful, false otherwise
     */
    bool prepareForRFBurst(uint32_t durationMs, float powerLevel);
    
    /**
     * @brief Update handler to process periodic power system updates
     * @param deltaTimeMs Time since last update in milliseconds
     */
    void update(uint32_t deltaTimeMs);

private:
    // Current power mode
    std::atomic<PowerMode> currentMode;
    
    // Subsystem power states (enabled/disabled)
    std::unordered_map<SubsystemID, bool> subsystemStates;
    
    // Subsystem power levels (0.0-1.0)
    std::unordered_map<SubsystemID, float> subsystemPowerLevels;
    
    // Registered callbacks for power warnings
    std::unordered_map<uint32_t, std::function<void(PowerMode)>> powerWarningCallbacks;
    
    // Next callback ID for registration
    uint32_t nextCallbackId;
    
    // Solar panel efficiency factors
    std::array<float, 6> solarPanelEfficiencies;
    
    // Battery health indicators
    float mainBatteryHealth;
    float backupBatteryHealth;
    
    // RF power allocations for different modes
    float rfStandardPowerAllocation;
    float rfBurstPowerAllocation;
    float rfEmergencyPowerAllocation;
    
    /**
     * @brief Calculate available power
     * @return Total available power in watts
     */
    float calculateAvailablePower() const;
    
    /**
     * @brief Calculate current power consumption
     * @return Total current consumption in watts
     */
    float calculateCurrentConsumption() const;
    
    /**
     * @brief Implement power-saving algorithm based on current status
     * @return Suggested power mode based on conditions
     */
    PowerMode determineSuggestedPowerMode() const;
    
    /**
     * @brief Apply triple modular redundancy to critical measurements
     * @param measurements Array of redundant measurements
     * @return Error-corrected measurement value
     */
    template<typename T>
    T applyTMR(const std::array<T, 3>& measurements) const;
    
    /**
     * @brief Apply error detection and correction to power controls
     */
    void applyScrubbing();
    
    /**
     * @brief Handle transition between power modes
     * @param fromMode Previous power mode
     * @param toMode New power mode
     */
    void handleModeTransition(PowerMode fromMode, PowerMode toMode);
};

} // namespace core
} // namespace skymesh

#endif // SKYMESH_POWER_MANAGER_H

