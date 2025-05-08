/**
 * @file health_monitor.h
 * @brief Satellite Health Monitoring System
 *
 * Defines the interface for monitoring satellite system health,
 * including radiation monitoring, temperature tracking, and 
 * component status management.
 */

#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <map>
#include <mutex>
#include <thread>

namespace skymesh {
namespace core {

/**
 * @brief Component health status enumeration
 */
enum class HealthStatus {
    NOMINAL,       ///< Component functioning within normal parameters
    DEGRADED,      ///< Component functioning with reduced capabilities
    WARNING,       ///< Component showing signs of potential failure
    CRITICAL,      ///< Component in critical state, immediate action required
    FAILED,        ///< Component has failed
    UNKNOWN        ///< Component status cannot be determined
};

/**
 * @brief Satellite component types
 */
enum class ComponentType {
    POWER_SYSTEM,          ///< Power generation and distribution
    COMMUNICATION_SYSTEM,  ///< Communication hardware
    THERMAL_CONTROL,       ///< Thermal regulation systems
    ATTITUDE_CONTROL,      ///< Orientation and stabilization
    PROPULSION,            ///< Propulsion systems
    PAYLOAD,               ///< Mission-specific payload
    PROCESSOR,             ///< Main onboard computer
    MEMORY,                ///< Storage systems
    SENSOR                 ///< Various sensors
};

/**
 * @brief Radiation measurement data
 */
struct RadiationData {
    float total_dose;              ///< Cumulative radiation dose in rads
    float dose_rate;               ///< Current radiation dose rate in rads/hour
    int single_event_upsets;       ///< Count of detected bit flips
    std::chrono::system_clock::time_point timestamp;  ///< Measurement timestamp
};

/**
 * @brief Temperature measurement data
 */
struct TemperatureData {
    float temperature_celsius;     ///< Temperature in Celsius
    ComponentType component;       ///< Component being measured
    std::string sensor_id;         ///< Unique sensor identifier
    std::chrono::system_clock::time_point timestamp;  ///< Measurement timestamp
};

/**
 * @brief Component health report
 */
struct ComponentHealth {
    ComponentType type;            ///< Component type
    std::string component_id;      ///< Unique component identifier
    HealthStatus status;           ///< Current health status
    float health_percentage;       ///< Estimated health as percentage (0-100)
    std::string diagnostic_info;   ///< Additional diagnostic information
    std::chrono::system_clock::time_point last_updated;  ///< Last update timestamp
};

/**
 * @brief Health alert configuration
 */
struct HealthAlertConfig {
    ComponentType component;       ///< Component to monitor
    HealthStatus trigger_status;   ///< Status level that triggers alert
    bool notify_ground;            ///< Whether to notify ground control
    bool auto_recovery;            ///< Whether to attempt automatic recovery
    uint8_t priority;              ///< Alert priority (0-255, 0 highest)
};

/**
 * @brief Callback function type for health status change notifications
 */
using HealthStatusCallback = std::function<void(const ComponentHealth&)>;

/**
 * @brief Interface for the satellite health monitoring system
 */
class HealthMonitor {
public:
    virtual ~HealthMonitor() = default;

    /**
     * @brief Initialize the health monitoring system
     * @param polling_interval_ms Interval for regular health checks in milliseconds
     * @return true if initialization successful, false otherwise
     */
    virtual bool initialize(uint32_t polling_interval_ms = 1000) = 0;

    /**
     * @brief Start health monitoring operations
     * @return true if successfully started
     */
    virtual bool start() = 0;

    /**
     * @brief Stop health monitoring operations
     */
    virtual void stop() = 0;

    /**
     * @brief Get the current health status of a component
     * @param component_id Unique identifier for the component
     * @return ComponentHealth structure with current health data
     */
    virtual ComponentHealth getComponentHealth(const std::string& component_id) const = 0;

    /**
     * @brief Get health status for all components
     * @return Vector of ComponentHealth structures
     */
    virtual std::vector<ComponentHealth> getAllComponentHealth() const = 0;

    /**
     * @brief Register a callback for health status changes
     * @param callback Function to call when component health changes
     * @param component_type Optional component type to filter notifications
     * @return Callback ID for later removal
     */
    virtual int registerStatusCallback(HealthStatusCallback callback,
                                       ComponentType component_type = ComponentType::PROCESSOR) = 0;

    /**
     * @brief Unregister a previously registered callback
     * @param callback_id ID returned from registerStatusCallback
     */
    virtual void unregisterStatusCallback(int callback_id) = 0;

    /**
     * @brief Configure health monitoring alerts
     * @param config Alert configuration
     */
    virtual void configureAlert(const HealthAlertConfig& config) = 0;

    /**
     * @brief Get the latest radiation data
     * @return Current radiation measurements
     */
    virtual RadiationData getRadiationData() const = 0;

    /**
     * @brief Get temperature data for a specific component
     * @param component Component type
     * @param sensor_id Optional specific sensor ID
     * @return Temperature data for the specified component
     */
    virtual TemperatureData getTemperature(ComponentType component,
                                           const std::string& sensor_id = "") const = 0;

    /**
     * @brief Initiate recovery procedure for a component
     * @param component_id Identifier for the component to recover
     * @return true if recovery initiated successfully
     */
    virtual bool initiateRecovery(const std::string& component_id) = 0;

    /**
     * @brief Report health information to ground station
     * @param full_report If true, send comprehensive health data
     * @return true if report was queued successfully
     */
    virtual bool reportToGround(bool full_report = false) = 0;
};

/**
 * @brief Factory function to create health monitor instance
 * @param config_path Path to configuration file (optional)
 * @return Unique pointer to HealthMonitor implementation
 */
std::unique_ptr<HealthMonitor> createHealthMonitor(const std::string& config_path = "");

} // namespace core
} // namespace skymesh


