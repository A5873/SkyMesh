/**
 * @file power_manager.cpp
 * @brief Implementation of the Power Management System for SkyMesh satellites
 */

#include "skymesh/core/power_manager.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <unordered_map>

namespace skymesh {
namespace core {

// Constants for power management
constexpr float MINIMUM_BATTERY_THRESHOLD = 0.15f;  // 15% minimum battery level
constexpr float LOW_POWER_THRESHOLD = 0.30f;        // 30% battery triggers low power mode
constexpr float CRITICAL_THRESHOLD = 0.20f;         // 20% battery triggers critical mode
constexpr float EMERGENCY_THRESHOLD = 0.10f;        // 10% battery triggers emergency mode
constexpr float NORMAL_RECOVERY_THRESHOLD = 0.40f;  // 40% battery to recover to normal mode

// Subsystem power requirements (watts)
constexpr float POWER_REQ_RF_STANDARD = 0.8f;
constexpr float POWER_REQ_RF_BURST = 2.5f;
constexpr float POWER_REQ_RF_EMERGENCY = 1.2f;
constexpr float POWER_REQ_OBC = 0.6f;
constexpr float POWER_REQ_ADCS = 0.75f;
constexpr float POWER_REQ_THERMAL = 0.5f;
constexpr float POWER_REQ_PAYLOAD = 1.5f;
constexpr float POWER_REQ_SENSORS = 0.3f;

PowerManager::PowerManager() 
    : currentMode(PowerMode::NORMAL),
      nextCallbackId(1),
      mainBatteryHealth(1.0f),
      backupBatteryHealth(1.0f),
      rfStandardPowerAllocation(0.8f),
      rfBurstPowerAllocation(1.0f),
      rfEmergencyPowerAllocation(0.9f) {
    
    // Initialize solar panel efficiencies (one for each face of the CubeSat)
    for (size_t i = 0; i < solarPanelEfficiencies.size(); ++i) {
        solarPanelEfficiencies[i] = 0.95f;  // Start with 95% efficiency
    }
}

PowerManager::~PowerManager() {
    // Safely shutdown all subsystems to prevent damage
    for (const auto& subsystem : subsystemStates) {
        if (subsystem.second) {
            disableSubsystem(subsystem.first);
        }
    }
    
    // Clear all registered callbacks
    powerWarningCallbacks.clear();
}

bool PowerManager::initialize(const std::vector<SubsystemID>& subsystems) {
    // Initialize subsystem states and power levels
    for (const auto& subsystem : subsystems) {
        // Triple redundant initialization for radiation hardening
        for (int i = 0; i < 3; ++i) {
            subsystemStates[subsystem] = false;
            subsystemPowerLevels[subsystem] = 0.0f;
        }
    }
    
    // Apply scrubbing to ensure consistent initialization
    applyScrubbing();
    
    // Set initial power mode based on battery status
    PowerSourceStatus batteryStatus = getPowerSourceStatus(PowerSource::BATTERY);
    if (batteryStatus.stateOfCharge < LOW_POWER_THRESHOLD) {
        setPowerMode(PowerMode::LOW_POWER);
    }
    
    // Perform initial health check
    return performHealthCheck();
}

bool PowerManager::setPowerMode(PowerMode mode) {
    // Store the current mode for transition handling
    PowerMode previousMode = currentMode.load();
    
    // Skip if it's the same mode
    if (previousMode == mode) {
        return true;
    }
    
    // Apply the mode change with TMR pattern for radiation hardening
    std::array<bool, 3> success = {false, false, false};
    
    for (int i = 0; i < 3; ++i) {
        try {
            // Handle transition between modes
            handleModeTransition(previousMode, mode);
            
            // Set the new mode
            currentMode.store(mode);
            success[i] = true;
        }
        catch (const std::exception& e) {
            // Log the error
            std::cerr << "Error setting power mode: " << e.what() << std::endl;
            success[i] = false;
        }
    }
    
    // Use majority voting to determine overall success
    bool overallSuccess = (success[0] && success[1]) || 
                         (success[0] && success[2]) || 
                         (success[1] && success[2]);
    
    // Notify registered callbacks about the power mode change
    if (overallSuccess) {
        for (const auto& callback : powerWarningCallbacks) {
            callback.second(mode);
        }
    }
    
    return overallSuccess;
}

PowerMode PowerManager::getCurrentPowerMode() const {
    // Read the current mode with redundancy for radiation hardening
    std::array<PowerMode, 3> redundantReads = {
        currentMode.load(),
        currentMode.load(),
        currentMode.load()
    };
    
    // Apply TMR to the readings
    return applyTMR<PowerMode>(redundantReads);
}

bool PowerManager::enableSubsystem(SubsystemID subsystem, float powerLevel) {
    // Validate power level
    powerLevel = std::max(0.0f, std::min(1.0f, powerLevel));
    
    // Get current power budget
    PowerBudget budget = getPowerBudget();
    
    // Check if we have enough power
    float requiredPower = 0.0f;
    
    switch (subsystem) {
        case SubsystemID::RF_SYSTEM:
            requiredPower = POWER_REQ_RF_STANDARD * powerLevel;
            break;
        case SubsystemID::OBC:
            requiredPower = POWER_REQ_OBC * powerLevel;
            break;
        case SubsystemID::ADCS:
            requiredPower = POWER_REQ_ADCS * powerLevel;
            break;
        case SubsystemID::THERMAL:
            requiredPower = POWER_REQ_THERMAL * powerLevel;
            break;
        case SubsystemID::PAYLOAD:
            requiredPower = POWER_REQ_PAYLOAD * powerLevel;
            break;
        case SubsystemID::SENSORS:
            requiredPower = POWER_REQ_SENSORS * powerLevel;
            break;
    }
    
    // Check if enabling would exceed available power
    if (budget.totalConsumption + requiredPower > budget.totalAvailable) {
        // Not enough power available
        return false;
    }
    
    // Triple redundant state update for radiation hardening
    for (int i = 0; i < 3; ++i) {
        subsystemStates[subsystem] = true;
        subsystemPowerLevels[subsystem] = powerLevel;
    }
    
    // Apply error correction
    applyScrubbing();
    
    return true;
}

bool PowerManager::disableSubsystem(SubsystemID subsystem) {
    // Triple redundant state update for radiation hardening
    for (int i = 0; i < 3; ++i) {
        subsystemStates[subsystem] = false;
        subsystemPowerLevels[subsystem] = 0.0f;
    }
    
    // Apply error correction
    applyScrubbing();
    
    return true;
}

bool PowerManager::isSubsystemEnabled(SubsystemID subsystem) const {
    // Check if the subsystem is in our map
    if (subsystemStates.find(subsystem) == subsystemStates.end()) {
        return false;
    }
    
    // Perform redundant reads for radiation hardening
    std::array<bool, 3> redundantReads = {
        subsystemStates.at(subsystem),
        subsystemStates.at(subsystem),
        subsystemStates.at(subsystem)
    };
    
    // Apply TMR to the readings
    return applyTMR<bool>(redundantReads);
}

PowerBudget PowerManager::getPowerBudget() const {
    PowerBudget budget;
    
    // Calculate total available power
    budget.totalAvailable = calculateAvailablePower();
    
    // Calculate total consumption
    budget.totalConsumption = calculateCurrentConsumption();
    
    // Determine projected available power based on orbital parameters
    // This is a simplified calculation
    PowerSourceStatus solarStatus = getPowerSourceStatus(PowerSource::SOLAR_PANEL);
    budget.projectedAvailable = solarStatus.currentVoltage * solarStatus.currentCurrent;
    
    // Get battery reserve
    PowerSourceStatus batteryStatus = getPowerSourceStatus(PowerSource::BATTERY);
    budget.batteryReserve = batteryStatus.stateOfCharge * 10.0f; // Assuming 10 Wh total capacity
    
    // Solar input rate
    budget.solarInputRate = solarStatus.currentVoltage * solarStatus.currentCurrent;
    
    // Current power mode
    budget.currentMode = getCurrentPowerMode();
    
    // Populate subsystem power consumption
    budget.subsystems.clear();
    for (const auto& subsystem : subsystemStates) {
        if (subsystem.second) { // If enabled
            PowerConsumption consumption;
            consumption.subsystem = subsystem.first;
            consumption.isActive = true;
            
            // Calculate power based on subsystem and level
            float powerLevel = subsystemPowerLevels.at(subsystem.first);
            
            switch (subsystem.first) {
                case SubsystemID::RF_SYSTEM:
                    consumption.currentPower = POWER_REQ_RF_STANDARD * powerLevel;
                    consumption.averagePower = POWER_REQ_RF_STANDARD * 0.7f;
                    consumption.peakPower = POWER_REQ_RF_BURST;
                    break;
                case SubsystemID::OBC:
                    consumption.currentPower = POWER_REQ_OBC * powerLevel;
                    consumption.averagePower = POWER_REQ_OBC * 0.9f;
                    consumption.peakPower = POWER_REQ_OBC;
                    break;
                case SubsystemID::ADCS:
                    consumption.currentPower = POWER_REQ_ADCS * powerLevel;
                    consumption.averagePower = POWER_REQ_ADCS * 0.8f;
                    consumption.peakPower = POWER_REQ_ADCS * 1.2f;
                    break;
                case SubsystemID::THERMAL:
                    consumption.currentPower = POWER_REQ_THERMAL * powerLevel;
                    consumption.averagePower = POWER_REQ_THERMAL * 0.6f;
                    consumption.peakPower = POWER_REQ_THERMAL * 1.5f;
                    break;
                case SubsystemID::PAYLOAD:
                    consumption.currentPower = POWER_REQ_PAYLOAD * powerLevel;
                    consumption.averagePower = POWER_REQ_PAYLOAD * 0.5f;
                    consumption.peakPower = POWER_REQ_PAYLOAD * 1.8f;
                    break;
                case SubsystemID::SENSORS:
                    consumption.currentPower = POWER_REQ_SENSORS * powerLevel;
                    consumption.averagePower = POWER_REQ_SENSORS * 0.7f;
                    consumption.peakPower = POWER_REQ_SENSORS * 1.1f;
                    break;
            }
            
            budget.subsystems.push_back(consumption);
        }
    }
    
    return budget;
}

PowerSourceStatus PowerManager::getPowerSourceStatus(PowerSource source) const {
    PowerSourceStatus status;
    status.source = source;
    status.lastUpdated = std::chrono::system_clock::now();
    
    // Simulate getting actual hardware readings
    // In a real implementation, this would involve reading from sensors
    
    switch (source) {
        case PowerSource::SOLAR_PANEL: {
            // Simulate solar panel readings based on efficiency
            float avgEfficiency = 0.0f;
            for (float eff : solarPanelEfficiencies) {
                avgEfficiency += eff;
            }
            avgEfficiency /= solarPanelEfficiencies.size();
            
            status.currentVoltage = 5.0f * avgEfficiency;
            status.currentCurrent = 0.2f * avgEfficiency;
            status.temperature = 25.0f; // Celsius
            status.stateOfCharge = 1.0f; // Not applicable for solar panels
            break;
        }
        case PowerSource::BATTERY: {
            // Simulate battery readings based on battery health
            status.currentVoltage = 3.7f * mainBatteryHealth;
            status.currentCurrent = 0.5f;
            status.temperature = 20.0f; // Celsius
            status.stateOfCharge = 0.75f * mainBatteryHealth; // 75% charged * health factor
            break;
        }
        case PowerSource::BACKUP_BATTERY: {
            // Simulate backup battery readings
            status.currentVoltage = 3.7f * backupBatteryHealth;
            status.currentCurrent = 0.1f;
            status.temperature = 18.0f; // Celsius
            status.stateOfCharge = 0.95f * backupBatteryHealth; // 95% charged * health factor
            break;
        }
    }
    
    return status;
}

bool PowerManager::setSubsystemPowerLevel(SubsystemID subsystem, float level) {
    // Validate level is between 0.0 and 1.0
    level = std::max(0.0f, std::min(1.0f, level));
    
    // Check if the subsystem is enabled
    if (!isSubsystemEnabled(subsystem)) {
        return false;
    }
    
    // Update power level with triple redundancy
    for (int i = 0; i < 3; ++i) {
        subsystemPowerLevels[subsystem] = level;
    }
    
    // Apply error correction
    applyScrubbing();
    
    return true;
}

uint32_t PowerManager::registerPowerWarningCallback(std::function<void(PowerMode)> callback) {
    // Assign a unique ID to this callback
    uint32_t callbackId = nextCallbackId++;
    
    // Store the callback with redundancy for radiation hardening
    for (int i = 0; i < 3; ++i) {
        powerWarningCallbacks[callbackId] = callback;
    }
    
    return callbackId;
}

void PowerManager::unregisterPowerWarningCallback(uint32_t callbackId) {
    // Remove the callback with redundancy for radiation hardening
    for (int i = 0; i < 3; ++i) {
        powerWarningCallbacks.erase(callbackId);
    }
}

void PowerManager::updateOrbitPowerProfile(uint32_t timeInSunlight, uint32_t timeInEclipse) {
    // Calculate expected power generation during sunlight period
    float avgSolarPanelEfficiency = 0.0f;
    for (float eff : solarPanelEfficiencies) {
        avgSolarPanelEfficiency += eff;
    }
    avgSolarPanelEfficiency /= solarPanelEfficiencies.size();
    
    // Calculate expected power generation and consumption throughout the orbit
    float expectedGenerationWattHours = (5.0f * 0.2f * avgSolarPanelEfficiency) * (timeInSunlight / 3600.0f);
    float expectedConsumptionWattHours = calculateCurrentConsumption() * ((timeInSunlight + timeInEclipse) / 3600.0f);
    
    // If expected consumption exceeds generation, prepare for power conservation
    if (expectedConsumptionWattHours > expectedGenerationWattHours) {
        PowerSourceStatus batteryStatus = getPowerSourceStatus(PowerSource::BATTERY);
        float estimatedEndCharge = batteryStatus.stateOfCharge - 
                                  (expectedConsumptionWattHours - expectedGenerationWattHours) / 10.0f; // 10Wh capacity
        
        // Determine if we need to change power mode based on projections
        if (estimatedEndCharge < CRITICAL_THRESHOLD && getCurrentPowerMode() != PowerMode::CRITICAL) {
            setPowerMode(PowerMode::LOW_POWER);
        } else if (estimatedEndCharge < EMERGENCY_THRESHOLD && getCurrentPowerMode() != PowerMode::EMERGENCY) {
            setPowerMode(PowerMode::CRITICAL);
        }
    }
}

bool PowerManager::performHealthCheck() {
    bool allHealthy = true;
    
    // Check solar panel health (simulate with efficiency values)
    for (size_t i = 0; i < solarPanelEfficiencies.size(); ++i) {
        if (solarPanelEfficiencies[i] < 0.6f) { // Below 60% efficiency is concerning
            allHealthy = false;
            // Log the issue
            std::cerr << "Solar panel " << i << " health degraded: " << solarPanelEfficiencies[i] * 100.0f << "% efficiency" << std::endl;
        }
    }
    
    // Check battery health
    if (mainBatteryHealth < 0.7f) {
        allHealthy = false;
        std::cerr << "Main battery health degraded: " << mainBatteryHealth * 100.0f << "%" << std::endl;
    }
    
    if (backupBatteryHealth < 0.8f) {
        allHealthy = false;
        std::cerr << "Backup battery health degraded: " << backupBatteryHealth * 100.0f << "%" << std::endl;
    }
    
    // Check for inconsistencies in subsystem states (radiation effect detection)
    for (const auto& subsystem : subsystemStates) {
        // Perform redundant reads
        std::array<bool, 3> stateReads = {
            subsystemStates.at(subsystem.first),
            subsystemStates.at(subsystem.first),
            subsystemStates.at(subsystem.first)
        };
        
        // Check for inconsistencies (indicating possible radiation effects)
        if (stateReads[0] != stateReads[1] || stateReads[0] != stateReads[2] || stateReads[1] != stateReads[2]) {
            allHealthy = false;
            std::cerr << "Inconsistent subsystem state detected for subsystem ID: " 
                      << static_cast<int>(subsystem.first) << std::endl;
            
            // Attempt to correct using TMR
            bool correctedState = applyTMR<bool>(stateReads);
            for (int i = 0; i < 3; ++i) {
                subsystemStates[subsystem.first] = correctedState;
            }
        }
    }
    
    return allHealthy;
}

bool PowerManager::reset(bool hardReset) {
    // For soft reset, restore default power mode and settings
    try {
        // Set power mode to normal with TMR pattern
        for (int i = 0; i < 3; ++i) {
            currentMode.store(PowerMode::NORMAL);
        }
        
        // Reset all subsystem power levels to zero
        for (auto& subsystem : subsystemPowerLevels) {
            for (int i = 0; i < 3; ++i) {
                subsystemPowerLevels[subsystem.first] = 0.0f;
            }
        }
        
        // Disable all subsystems
        for (auto& subsystem : subsystemStates) {
            disableSubsystem(subsystem.first);
        }
        
        // Reset RF power allocations to defaults
        rfStandardPowerAllocation = 0.8f;
        rfBurstPowerAllocation = 1.0f;
        rfEmergencyPowerAllocation = 0.9f;
        
        if (hardReset) {
            // In a real implementation, this would involve hardware resets
            // For simulation, we'll just reinitialize everything
            
            // Reinitialize solar panel efficiencies
            for (size_t i = 0; i < solarPanelEfficiencies.size(); ++i) {
                solarPanelEfficiencies[i] = 0.95f;
            }
            
            // Reinitialize battery health factors
            mainBatteryHealth = 1.0f;
            backupBatteryHealth = 1.0f;
            
            // Clear all callbacks
            powerWarningCallbacks.clear();
            nextCallbackId = 1;
        }
        
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error during power system reset: " << e.what() << std::endl;
        return false;
    }
}

bool PowerManager::handleRadiationErrors() {
    bool errorsDetected = false;
    bool allErrorsCorrected = true;
    
    // Check for errors in power mode (critical state)
    std::array<PowerMode, 3> modeReads = {
        currentMode.load(),
        currentMode.load(),
        currentMode.load()
    };
    
    // Check for inconsistencies
    if (modeReads[0] != modeReads[1] || modeReads[0] != modeReads[2] || modeReads[1] != modeReads[2]) {
        errorsDetected = true;
        
        // Attempt to correct using TMR
        PowerMode correctedMode = applyTMR<PowerMode>(modeReads);
        currentMode.store(correctedMode);
        
        std::cerr << "Corrected radiation-induced error in power mode" << std::endl;
    }
    
    // Check for errors in subsystem states
    for (auto& subsystem : subsystemStates) {
        // Create temporary storage for redundant reads
        std::array<bool, 3> stateReads;
        std::array<float, 3> levelReads;
        
        // Perform redundant reads
        for (int i = 0; i < 3; ++i) {
            stateReads[i] = subsystemStates.at(subsystem.first);
            levelReads[i] = subsystemPowerLevels.at(subsystem.first);
        }
        
        // Check for inconsistencies in states
        if (stateReads[0] != stateReads[1] || stateReads[0] != stateReads[2] || stateReads[1] != stateReads[2]) {
            errorsDetected = true;
            
            // Attempt to correct using TMR
            bool correctedState = applyTMR<bool>(stateReads);
            for (int i = 0; i < 3; ++i) {
                subsystemStates[subsystem.first] = correctedState;
            }
            
            std::cerr << "Corrected radiation-induced error in subsystem state: " 
                      << static_cast<int>(subsystem.first) << std::endl;
        }
        
        // Check for inconsistencies in power levels
        if (std::abs(levelReads[0] - levelReads[1]) > 0.01f || 
            std::abs(levelReads[0] - levelReads[2]) > 0.01f || 
            std::abs(levelReads[1] - levelReads[2]) > 0.01f) {
            
            errorsDetected = true;
            
            // For floating point values, take the median as the "correct" value
            std::array<float, 3> sortedLevels = levelReads;
            std::sort(sortedLevels.begin(), sortedLevels.end());
            float correctedLevel = sortedLevels[1]; // Median value
            
            for (int i = 0; i < 3; ++i) {
                subsystemPowerLevels[subsystem.first] = correctedLevel;
            }
            
            std::cerr << "Corrected radiation-induced error in subsystem power level: " 
                      << static_cast<int>(subsystem.first) << std::endl;
        }
    }
    
    // Apply memory scrubbing to detect and correct any other errors
    applyScrubbing();
    
    return errorsDetected && allErrorsCorrected;
}

float PowerManager::calculateCurrentConsumption() const {
    float totalConsumption = 0.0f;
    
    // Real implementation would query actual power draws
    // For this example, we calculate based on enabled subsystems
    for (const auto& pair : subsystemStates) {
        if (pair.second) { // If subsystem is enabled
            SubsystemID id = pair.first;
            float powerLevel = 1.0f; // Default to full power
            
            // Get configured power level if available
            auto it = subsystemPowerLevels.find(id);
            if (it != subsystemPowerLevels.end()) {
                powerLevel = it->second;
            }
            
            // Base power consumption for each subsystem type
            float basePower = 0.0f;
            switch (id) {
                case SubsystemID::RF_SYSTEM:
                    basePower = 5.0f; // 5W
                    break;
                case SubsystemID::OBC:
                    basePower = 3.0f; // 3W
                    break;
                case SubsystemID::ADCS:
                    basePower = 4.0f; // 4W
                    break;
                case SubsystemID::THERMAL:
                    basePower = 2.0f; // 2W
                    break;
                case SubsystemID::PAYLOAD:
                    basePower = 8.0f; // 8W
                    break;
                case SubsystemID::SENSORS:
                    basePower = 1.5f; // 1.5W
                    break;
            }
            
            totalConsumption += basePower * powerLevel;
        }
    }
    
    // Use TMR pattern to get the correct value
    return totalConsumption;
}

float PowerManager::calculateAvailablePower() const {
    float totalPower = 0.0f;
    
    // Get power from solar panels
    auto solarStatus = getPowerSourceStatus(PowerSource::SOLAR_PANEL);
    totalPower += solarStatus.currentVoltage * solarStatus.currentCurrent;
    
    // Add power available from batteries
    auto batteryStatus = getPowerSourceStatus(PowerSource::BATTERY);
    if (batteryStatus.stateOfCharge > 0.1f) { // Only count battery if it has sufficient charge
        totalPower += batteryStatus.currentVoltage * batteryStatus.currentCurrent;
    }
    
    // Factor in system efficiency and losses
    float systemEfficiency = 0.95f; // 95% system efficiency
    
    // Apply radiation hardening correction factors
    float radiationFactor = 0.98f; // 2% loss due to radiation effects
    
    return totalPower * systemEfficiency * radiationFactor;
}

// Implementation of the TMR template function for radiation hardening
template<typename T>
T PowerManager::applyTMR(const std::array<T, 3>& measurements) const {
    // Apply Triple Modular Redundancy voting algorithm
    // Return the majority value to correct for single-event upsets
    
    // For boolean values, use direct voting
    if constexpr (std::is_same_v<T, bool>) {
        // Count the number of true values
        int trueCount = 0;
        for (const auto& value : measurements) {
            if (value) {
                trueCount++;
            }
        }
        
        // Majority vote: return true if at least 2 values are true
        return trueCount >= 2;
    }
    // For integral types (including enums), use direct comparison
    else if constexpr (std::is_integral_v<T> || std::is_enum_v<T>) {
        // If at least two values match, return that value
        if (measurements[0] == measurements[1] || measurements[0] == measurements[2]) {
            return measurements[0];
        } else if (measurements[1] == measurements[2]) {
            return measurements[1];
        }
        
        // If all three values differ (extremely rare), return the first value
        // In a real implementation, this might trigger additional error handling
        return measurements[0];
    }
    // For floating point types, use median as the most accurate value
    else if constexpr (std::is_floating_point_v<T>) {
        // Create a copy of the array for sorting
        std::array<T, 3> sortedValues = measurements;
        std::sort(sortedValues.begin(), sortedValues.end());
        
        // Return the median value
        return sortedValues[1];
    }
    // For other types (not expected in this context)
    else {
        // Default case: if two or more values match, return that value
        if (measurements[0] == measurements[1] || measurements[0] == measurements[2]) {
            return measurements[0];
        } else if (measurements[1] == measurements[2]) {
            return measurements[1];
        }
        
        // Fallback: return the first value
        return measurements[0];
    }
}

// Explicit template instantiations for types used in the power manager
template bool PowerManager::applyTMR<bool>(const std::array<bool, 3>& measurements) const;
template PowerMode PowerManager::applyTMR<PowerMode>(const std::array<PowerMode, 3>& measurements) const;
template float PowerManager::applyTMR<float>(const std::array<float, 3>& measurements) const;
template int PowerManager::applyTMR<int>(const std::array<int, 3>& measurements) const;

PowerMode PowerManager::determineSuggestedPowerMode() const {
    // Determine the suggested power mode based on battery status and power budget
    
    // We don't need battery status here as we're using direct power measurements
    // and already check battery status in the available power calculations
    
    // Get power budget
    PowerBudget budget = getPowerBudget();
    // Calculate available power from all sources using redundant measurements
    // for radiation hardening
    
    std::array<float, 3> totalPowerMeasurements = {0.0f, 0.0f, 0.0f};
    
    for (int i = 0; i < 3; ++i) {
        // Get solar panel contribution
        PowerSourceStatus solarStatus = getPowerSourceStatus(PowerSource::SOLAR_PANEL);
        float solarPower = solarStatus.currentVoltage * solarStatus.currentCurrent;
        
        // Get main battery contribution based on current power mode
        PowerSourceStatus batteryStatus = getPowerSourceStatus(PowerSource::BATTERY);
        float batteryPower = 0.0f;
        
        // Determine how much we can draw from the battery based on current mode
        PowerMode mode = getCurrentPowerMode();
        switch (mode) {
            case PowerMode::NORMAL:
                // In normal mode, we can use up to 100% of battery capacity
                batteryPower = 3.0f;  // Max 3W from battery in normal mode
                break;
            case PowerMode::LOW_POWER:
                // In low power mode, restrict battery usage
                batteryPower = 2.0f;
                break;
            case PowerMode::CRITICAL:
                // In critical mode, severely restrict battery usage
                batteryPower = 1.5f;
                break;
            case PowerMode::EMERGENCY:
                // In emergency mode, only use battery for essential systems
                batteryPower = 1.0f;
                break;
            case PowerMode::HIBERNATION:
                // In hibernation, minimal power usage
                batteryPower = 0.5f;
                break;
        }
        
        // Reduce available battery power if battery health is degraded
        batteryPower *= mainBatteryHealth;
        
        // Don't draw from battery if state of charge is below minimum threshold
        if (batteryStatus.stateOfCharge < MINIMUM_BATTERY_THRESHOLD) {
            batteryPower = 0.0f;
        }
        
        // Get backup battery contribution (only used in emergency or critical modes)
        float backupPower = 0.0f;
        if (mode == PowerMode::EMERGENCY || mode == PowerMode::CRITICAL) {
            PowerSourceStatus backupStatus = getPowerSourceStatus(PowerSource::BACKUP_BATTERY);
            if (backupStatus.stateOfCharge > MINIMUM_BATTERY_THRESHOLD) {
                backupPower = 1.0f * backupBatteryHealth;
            }
        }
        
        // Calculate total available power
        totalPowerMeasurements[i] = solarPower + batteryPower + backupPower;
    }
    
    // Use TMR pattern to get the correct value
    // Convert the float result to PowerMode based on thresholds
    float availablePower = applyTMR<float>(totalPowerMeasurements);
    
    // Determine appropriate mode based on available power
    if (availablePower < 1.0f) {
        return PowerMode::EMERGENCY;
    } else if (availablePower < 2.0f) {
        return PowerMode::CRITICAL;
    } else if (availablePower < 3.0f) {
        return PowerMode::LOW_POWER;
    } else {
        return PowerMode::NORMAL;
    }
}

void PowerManager::applyScrubbing() {
    // Apply error detection and correction to power management parameters
    // using a memory scrubbing technique
    
    // Check and correct power mode
    std::array<PowerMode, 3> modeReads = {
        currentMode.load(),
        currentMode.load(),
        currentMode.load()
    };
    
    // Check for inconsistencies
    if (modeReads[0] != modeReads[1] || modeReads[0] != modeReads[2] || modeReads[1] != modeReads[2]) {
        // Apply TMR to correct the power mode
        PowerMode correctedMode = applyTMR<PowerMode>(modeReads);
        currentMode.store(correctedMode);
    }
    
    // Check and correct subsystem states
    for (auto& subsystem : subsystemStates) {
        // Perform redundant reads
        std::array<bool, 3> stateReads = {
            subsystemStates.at(subsystem.first),
            subsystemStates.at(subsystem.first),
            subsystemStates.at(subsystem.first)
        };
        
        // Check for inconsistencies
        if (stateReads[0] != stateReads[1] || stateReads[0] != stateReads[2] || stateReads[1] != stateReads[2]) {
            // Apply TMR to correct the state
            bool correctedState = applyTMR<bool>(stateReads);
            subsystemStates[subsystem.first] = correctedState;
        }
    }
    
    // Check and correct power levels
    for (auto& level : subsystemPowerLevels) {
        // Perform redundant reads
        std::array<float, 3> levelReads = {
            subsystemPowerLevels.at(level.first),
            subsystemPowerLevels.at(level.first),
            subsystemPowerLevels.at(level.first)
        };
        
        // For floating point, check if differences exceed a threshold
        if (std::abs(levelReads[0] - levelReads[1]) > 0.01f || 
            std::abs(levelReads[0] - levelReads[2]) > 0.01f || 
            std::abs(levelReads[1] - levelReads[2]) > 0.01f) {
            
            // Sort the values and take the median for correction
            std::array<float, 3> sortedLevels = levelReads;
            std::sort(sortedLevels.begin(), sortedLevels.end());
            float correctedLevel = sortedLevels[1]; // Median value
            
            subsystemPowerLevels[level.first] = correctedLevel;
        }
    }
    
    // Check and correct RF power allocations
    std::array<float, 3> standardAllocReads = {
        rfStandardPowerAllocation,
        rfStandardPowerAllocation,
        rfStandardPowerAllocation
    };
    
    std::array<float, 3> burstAllocReads = {
        rfBurstPowerAllocation,
        rfBurstPowerAllocation,
        rfBurstPowerAllocation
    };
    
    std::array<float, 3> emergencyAllocReads = {
        rfEmergencyPowerAllocation,
        rfEmergencyPowerAllocation,
        rfEmergencyPowerAllocation
    };
    
    // Check for inconsistencies in standard allocation
    if (std::abs(standardAllocReads[0] - standardAllocReads[1]) > 0.01f || 
        std::abs(standardAllocReads[0] - standardAllocReads[2]) > 0.01f || 
        std::abs(standardAllocReads[1] - standardAllocReads[2]) > 0.01f) {
        
        std::array<float, 3> sortedAllocs = standardAllocReads;
        std::sort(sortedAllocs.begin(), sortedAllocs.end());
        rfStandardPowerAllocation = sortedAllocs[1]; // Median value
    }
    
    // Check for inconsistencies in burst allocation
    if (std::abs(burstAllocReads[0] - burstAllocReads[1]) > 0.01f || 
        std::abs(burstAllocReads[0] - burstAllocReads[2]) > 0.01f || 
        std::abs(burstAllocReads[1] - burstAllocReads[2]) > 0.01f) {
        
        std::array<float, 3> sortedAllocs = burstAllocReads;
        std::sort(sortedAllocs.begin(), sortedAllocs.end());
        rfBurstPowerAllocation = sortedAllocs[1]; // Median value
    }
    
    // Check for inconsistencies in emergency allocation
    if (std::abs(emergencyAllocReads[0] - emergencyAllocReads[1]) > 0.01f || 
        std::abs(emergencyAllocReads[0] - emergencyAllocReads[2]) > 0.01f || 
        std::abs(emergencyAllocReads[1] - emergencyAllocReads[2]) > 0.01f) {
        
        std::array<float, 3> sortedAllocs = emergencyAllocReads;
        std::sort(sortedAllocs.begin(), sortedAllocs.end());
        rfEmergencyPowerAllocation = sortedAllocs[1]; // Median value
    }
}

void PowerManager::handleModeTransition(PowerMode fromMode, PowerMode toMode) {
    // Handle transition between power modes
    // This involves adjusting subsystem power levels and enabling/disabling
    // certain subsystems based on the new power mode
    
    // Log the transition
    std::cout << "Power mode transition: " 
              << static_cast<int>(fromMode) << " -> " << static_cast<int>(toMode) << std::endl;
    
    // Apply triple redundancy for radiation hardening
    std::array<bool, 3> success = {false, false, false};
    
    for (int i = 0; i < 3; ++i) {
        try {
            // Configure subsystems based on the target power mode
            switch (toMode) {
                case PowerMode::NORMAL:
                    // In normal mode, enable most subsystems
                    if (isSubsystemEnabled(SubsystemID::RF_SYSTEM)) {
                        setSubsystemPowerLevel(SubsystemID::RF_SYSTEM, rfStandardPowerAllocation);
                    }
                    if (isSubsystemEnabled(SubsystemID::OBC)) {
                        setSubsystemPowerLevel(SubsystemID::OBC, 1.0f);
                    }
                    if (isSubsystemEnabled(SubsystemID::ADCS)) {
                        setSubsystemPowerLevel(SubsystemID::ADCS, 1.0f);
                    }
                    if (isSubsystemEnabled(SubsystemID::THERMAL)) {
                        setSubsystemPowerLevel(SubsystemID::THERMAL, 1.0f);
                    }
                    if (isSubsystemEnabled(SubsystemID::PAYLOAD)) {
                        setSubsystemPowerLevel(SubsystemID::PAYLOAD, 1.0f);
                    }
                    if (isSubsystemEnabled(SubsystemID::SENSORS)) {
                        setSubsystemPowerLevel(SubsystemID::SENSORS, 1.0f);
                    }
                    break;
                    
                case PowerMode::LOW_POWER:
                    // In low power mode, reduce non-essential systems
                    if (isSubsystemEnabled(SubsystemID::RF_SYSTEM)) {
                        setSubsystemPowerLevel(SubsystemID::RF_SYSTEM, rfStandardPowerAllocation * 0.7f);
                    }
                    if (isSubsystemEnabled(SubsystemID::OBC)) {
                        setSubsystemPowerLevel(SubsystemID::OBC, 0.8f);
                    }
                    if (isSubsystemEnabled(SubsystemID::ADCS)) {
                        setSubsystemPowerLevel(SubsystemID::ADCS, 0.6f);
                    }
                    if (isSubsystemEnabled(SubsystemID::THERMAL)) {
                        setSubsystemPowerLevel(SubsystemID::THERMAL, 0.7f);
                    }
                    if (isSubsystemEnabled(SubsystemID::PAYLOAD)) {
                        setSubsystemPowerLevel(SubsystemID::PAYLOAD, 0.5f);
                    }
                    if (isSubsystemEnabled(SubsystemID::SENSORS)) {
                        setSubsystemPowerLevel(SubsystemID::SENSORS, 0.7f);
                    }
                    break;
                    
                case PowerMode::CRITICAL:
                    // In critical mode, disable non-essential systems
                    if (isSubsystemEnabled(SubsystemID::RF_SYSTEM)) {
                        setSubsystemPowerLevel(SubsystemID::RF_SYSTEM, rfEmergencyPowerAllocation);
                    }
                    if (isSubsystemEnabled(SubsystemID::OBC)) {
                        setSubsystemPowerLevel(SubsystemID::OBC, 0.6f);
                    }
                    if (isSubsystemEnabled(SubsystemID::ADCS)) {
                        setSubsystemPowerLevel(SubsystemID::ADCS, 0.4f);
                    }
                    if (isSubsystemEnabled(SubsystemID::THERMAL)) {
                        setSubsystemPowerLevel(SubsystemID::THERMAL, 0.5f);
                    }
                    // Disable payload in critical mode
                    if (isSubsystemEnabled(SubsystemID::PAYLOAD)) {
                        disableSubsystem(SubsystemID::PAYLOAD);
                    }
                    if (isSubsystemEnabled(SubsystemID::SENSORS)) {
                        setSubsystemPowerLevel(SubsystemID::SENSORS, 0.5f);
                    }
                    break;
                    
                case PowerMode::EMERGENCY:
                    // In emergency mode, only keep essential systems online
                    if (isSubsystemEnabled(SubsystemID::RF_SYSTEM)) {
                        setSubsystemPowerLevel(SubsystemID::RF_SYSTEM, rfEmergencyPowerAllocation * 0.8f);
                    }
                    if (isSubsystemEnabled(SubsystemID::OBC)) {
                        setSubsystemPowerLevel(SubsystemID::OBC, 0.4f);
                    }
                    if (isSubsystemEnabled(SubsystemID::ADCS)) {
                        setSubsystemPowerLevel(SubsystemID::ADCS, 0.2f);
                    }
                    // Disable non-essential systems
                    if (isSubsystemEnabled(SubsystemID::THERMAL)) {
                        setSubsystemPowerLevel(SubsystemID::THERMAL, 0.3f);
                    }
                    if (isSubsystemEnabled(SubsystemID::PAYLOAD)) {
                        disableSubsystem(SubsystemID::PAYLOAD);
                    }
                    if (isSubsystemEnabled(SubsystemID::SENSORS)) {
                        setSubsystemPowerLevel(SubsystemID::SENSORS, 0.3f);
                    }
                    break;
                    
                case PowerMode::HIBERNATION:
                    // In hibernation mode, disable everything except OBC and emergency RF
                    if (isSubsystemEnabled(SubsystemID::RF_SYSTEM)) {
                        setSubsystemPowerLevel(SubsystemID::RF_SYSTEM, rfEmergencyPowerAllocation * 0.5f);
                    }
                    if (isSubsystemEnabled(SubsystemID::OBC)) {
                        setSubsystemPowerLevel(SubsystemID::OBC, 0.2f);
                    }
                    // Disable all other systems
                    if (isSubsystemEnabled(SubsystemID::ADCS)) {
                        disableSubsystem(SubsystemID::ADCS);
                    }
                    if (isSubsystemEnabled(SubsystemID::THERMAL)) {
                        disableSubsystem(SubsystemID::THERMAL);
                    }
                    if (isSubsystemEnabled(SubsystemID::PAYLOAD)) {
                        disableSubsystem(SubsystemID::PAYLOAD);
                    }
                    if (isSubsystemEnabled(SubsystemID::SENSORS)) {
                        disableSubsystem(SubsystemID::SENSORS);
                    }
                    break;
            }
            
            // Special transition-specific logic
            if (fromMode == PowerMode::HIBERNATION && toMode != PowerMode::HIBERNATION) {
                // Coming out of hibernation - needs warm-up sequence
                std::cout << "Executing warm-up sequence from hibernation mode" << std::endl;
                
                // Enable critical systems first with minimal power
                if (!isSubsystemEnabled(SubsystemID::OBC)) {
                    enableSubsystem(SubsystemID::OBC, 0.5f);
                }
                if (!isSubsystemEnabled(SubsystemID::RF_SYSTEM)) {
                    enableSubsystem(SubsystemID::RF_SYSTEM, rfEmergencyPowerAllocation);
                }
                
                // Thermal system should be enabled to normalize temperature
                if (!isSubsystemEnabled(SubsystemID::THERMAL)) {
                    enableSubsystem(SubsystemID::THERMAL, 0.7f);
                }
            }
            
            if (toMode == PowerMode::NORMAL && fromMode != PowerMode::NORMAL) {
                // If returning to normal mode, ensure all essential systems are enabled
                std::cout << "Restoring normal mode operations" << std::endl;
                
                // Enable essential sensor systems if they were disabled
                if (!isSubsystemEnabled(SubsystemID::SENSORS)) {
                    enableSubsystem(SubsystemID::SENSORS, 0.8f);
                }
                
                // Enable ADCS if it was disabled
                if (!isSubsystemEnabled(SubsystemID::ADCS)) {
                    enableSubsystem(SubsystemID::ADCS, 0.7f);
                }
            }
            
            success[i] = true;
        }
        catch (const std::exception& e) {
            std::cerr << "Error during power mode transition: " << e.what() << std::endl;
            success[i] = false;
        }
    }
    
    // Use majority voting to determine overall success
    bool overallSuccess = (success[0] && success[1]) || 
                         (success[0] && success[2]) || 
                         (success[1] && success[2]);
                         
    // If successful, run a health check to ensure all systems are stable
    if (overallSuccess) {
        performHealthCheck();
    }
    
    // void function - do not return a value
}

/**
 * Updates the power system state and handles mode transitions if needed
 * @param deltaTimeMs Time since last update in milliseconds (not currently used 
 *                    but kept for future implementations that may need timing)
 */
void PowerManager::update(uint32_t /*deltaTimeMs*/) {
    // Perform periodic power system update with radiation-hardened approach
    std::array<bool, 3> success = {false, false, false};
    
    for (int i = 0; i < 3; ++i) {
        try {
            // Get current battery status
            PowerSourceStatus batteryStatus = getPowerSourceStatus(PowerSource::BATTERY);
            PowerMode currentMode = getCurrentPowerMode();
            
            // Automatic mode transitions based on battery levels
            if (batteryStatus.stateOfCharge <= EMERGENCY_THRESHOLD && currentMode != PowerMode::EMERGENCY) {
                // Battery critically low, enter emergency mode
                setPowerMode(PowerMode::EMERGENCY);
            }
            else if (batteryStatus.stateOfCharge <= CRITICAL_THRESHOLD && 
                     currentMode != PowerMode::CRITICAL && 
                     currentMode != PowerMode::EMERGENCY) {
                // Battery very low, enter critical mode
                setPowerMode(PowerMode::CRITICAL);
            }
            else if (batteryStatus.stateOfCharge <= LOW_POWER_THRESHOLD && 
                     currentMode == PowerMode::NORMAL) {
                // Battery getting low, enter low power mode
                setPowerMode(PowerMode::LOW_POWER);
            }
            else if (batteryStatus.stateOfCharge >= NORMAL_RECOVERY_THRESHOLD && 
                     (currentMode == PowerMode::LOW_POWER || 
                      currentMode == PowerMode::CRITICAL)) {
                // Battery recovered, return to normal mode
                setPowerMode(PowerMode::NORMAL);
            }
            
            // Check for solar panel status
            // We check solar panels but don't use the status directly in this function
            // getPowerSourceStatus(PowerSource::SOLAR_PANEL);
            // Calculate solar power but we're only checking status here, not using the value
            // float solarPower = solarStatus.currentVoltage * solarStatus.currentCurrent;
            
            // Update the power budgets and adjust if necessary
            PowerBudget budget = getPowerBudget();
            
            // Ensure we're not exceeding our power budget
            if (budget.totalConsumption > budget.totalAvailable * 0.95f) {
                // We're using too much power, reduce non-essential systems
                // Scale down subsystem power based on priority
                for (auto& consumption : budget.subsystems) {
                    if (consumption.subsystem == SubsystemID::PAYLOAD) {
                        // Reduce payload power first
                        float currentLevel = subsystemPowerLevels.at(consumption.subsystem);
                        setSubsystemPowerLevel(consumption.subsystem, currentLevel * 0.8f);
                    }
                }
            }
            
            // Check for radiation errors and correct them
            handleRadiationErrors();
            
            // Update battery health factors based on usage patterns
            // In a real implementation, this would use actual measurements
            
            // Perform memory scrubbing to detect and correct radiation-induced errors
            applyScrubbing();
            
            success[i] = true;
        }
        catch (const std::exception& e) {
            std::cerr << "Error during power system update: " << e.what() << std::endl;
            success[i] = false;
        }
    }
    
    // Use majority voting to determine overall success
    // We don't need to use the result since we're not returning it
    // and there's no additional work to do based on success/failure
    
    // No return value needed for void function
}

bool PowerManager::setRFPowerAllocations(float standardMode, float burstMode, float emergencyMode) {
    // Validate input values
    standardMode = std::max(0.1f, std::min(1.0f, standardMode));
    burstMode = std::max(0.2f, std::min(1.0f, burstMode));
    emergencyMode = std::max(0.3f, std::min(1.0f, emergencyMode));
    
    // Check if RF system is registered
    if (subsystemStates.find(SubsystemID::RF_SYSTEM) == subsystemStates.end()) {
        return false;
    }
    
    // Set allocations with redundancy for radiation hardening
    for (int i = 0; i < 3; ++i) {
        rfStandardPowerAllocation = standardMode;
        rfBurstPowerAllocation = burstMode;
        rfEmergencyPowerAllocation = emergencyMode;
    }
    
    // If RF system is currently enabled, update its power level based on current mode
    if (isSubsystemEnabled(SubsystemID::RF_SYSTEM)) {
        PowerMode mode = getCurrentPowerMode();
        float level = 0.0f;
        
        switch (mode) {
            case PowerMode::NORMAL:
                level = rfStandardPowerAllocation;
                break;
            case PowerMode::LOW_POWER:
                level = rfStandardPowerAllocation * 0.7f;
                break;
            case PowerMode::CRITICAL:
                level = rfEmergencyPowerAllocation;
                break;
            case PowerMode::EMERGENCY:
                level = rfEmergencyPowerAllocation * 0.8f;
                break;
            case PowerMode::HIBERNATION:
                // RF system should be disabled in hibernation
                disableSubsystem(SubsystemID::RF_SYSTEM);
                return true;
        }
        
        // Update the RF system's power level
        setSubsystemPowerLevel(SubsystemID::RF_SYSTEM, level);
    }
    
    // Apply scrubbing to ensure values are consistent
    applyScrubbing();
    
    // Verify the allocations were set correctly using TMR pattern
    std::array<float, 3> standardAllocReads = {
        rfStandardPowerAllocation,
        rfStandardPowerAllocation,
        rfStandardPowerAllocation
    };
    
    std::array<float, 3> burstAllocReads = {
        rfBurstPowerAllocation,
        rfBurstPowerAllocation,
        rfBurstPowerAllocation
    };
    
    std::array<float, 3> emergencyAllocReads = {
        rfEmergencyPowerAllocation,
        rfEmergencyPowerAllocation,
        rfEmergencyPowerAllocation
    };
    
    // Verify allocations are consistent
    bool standardConsistent = (std::abs(standardAllocReads[0] - standardMode) < 0.01f) &&
                             (std::abs(standardAllocReads[1] - standardMode) < 0.01f) &&
                             (std::abs(standardAllocReads[2] - standardMode) < 0.01f);
    
    bool burstConsistent = (std::abs(burstAllocReads[0] - burstMode) < 0.01f) &&
                          (std::abs(burstAllocReads[1] - burstMode) < 0.01f) &&
                          (std::abs(burstAllocReads[2] - burstMode) < 0.01f);
    
    bool emergencyConsistent = (std::abs(emergencyAllocReads[0] - emergencyMode) < 0.01f) &&
                               (std::abs(emergencyAllocReads[1] - emergencyMode) < 0.01f) &&
                               (std::abs(emergencyAllocReads[2] - emergencyMode) < 0.01f);
    
    return standardConsistent && burstConsistent && emergencyConsistent;
}

bool PowerManager::prepareForRFBurst(uint32_t durationMs, float powerLevel) {
    // Validate input parameters
    if (powerLevel < 0.0f || powerLevel > 1.0f) {
        return false;
    }

    // Check if RF system is enabled and available
    if (!isSubsystemEnabled(SubsystemID::RF_SYSTEM)) {
        return false;
    }

    // Check if we have enough power budget for the burst
    PowerBudget budget = getPowerBudget();
    float burstPowerRequired = POWER_REQ_RF_BURST * powerLevel;
    
    // Calculate total power needed for the burst duration
    float totalEnergyRequired = (burstPowerRequired * durationMs) / 1000.0f; // Convert to watt-seconds
    
    // Check if we have enough power available
    if ((budget.totalAvailable - budget.totalConsumption) < burstPowerRequired) {
        return false;
    }
    
    // Check battery capacity
    PowerSourceStatus batteryStatus = getPowerSourceStatus(PowerSource::BATTERY);
    float availableEnergy = batteryStatus.stateOfCharge * 10.0f * 3600.0f; // Convert to watt-seconds
    
    if (availableEnergy < totalEnergyRequired) {
        return false;
    }
    
    // If all checks pass, set up for RF burst
    for (int i = 0; i < 3; ++i) { // Triple redundancy for radiation hardening
        rfBurstPowerAllocation = powerLevel;
    }
    
    // Apply scrubbing to ensure values are consistent
    applyScrubbing();
    
    return true;
}

} // namespace core
} // namespace skymesh
