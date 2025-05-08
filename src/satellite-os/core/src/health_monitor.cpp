/**
 * @file health_monitor.cpp
 * @brief Implementation of the Health Monitor system
 */

#include "skymesh/core/health_monitor.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace skymesh {
namespace core {

namespace {
    // Helper functions for logging
    void log_debug(const std::string& message) {
        std::cout << "[DEBUG] " << message << std::endl;
    }
    
    void log_info(const std::string& message) {
        std::cout << "[INFO] " << message << std::endl;
    }
    
    void log_warning(const std::string& message) {
        std::cout << "[WARNING] " << message << std::endl;
    }
    
    void log_error(const std::string& message) {
        std::cerr << "[ERROR] " << message << std::endl;
    }
}

class HealthMonitorImpl : public HealthMonitor {
private:
    // Thread synchronization
    mutable std::mutex mutex_;
    std::thread monitor_thread_;
    bool running_;
    uint32_t polling_interval_ms_;

    // Component health data
    std::map<std::string, ComponentHealth> component_health_;
    std::map<ComponentType, std::vector<HealthAlertConfig>> alert_configs_;
    
    // Radiation and temperature monitoring
    RadiationData current_radiation_;
    std::map<std::string, TemperatureData> temperature_data_;
    
    // Callbacks
    struct CallbackEntry {
        int id;
        HealthStatusCallback callback;
        ComponentType filter_type;
    };
    std::vector<CallbackEntry> callbacks_;
    int next_callback_id_;

public:
    HealthMonitorImpl()
        : running_(false)
        , polling_interval_ms_(1000)
        , next_callback_id_(0) {
    }

    ~HealthMonitorImpl() {
        stop();
    }

    bool initialize(uint32_t polling_interval_ms) override {
        std::lock_guard<std::mutex> lock(mutex_);
        polling_interval_ms_ = polling_interval_ms;
        return true;
    }

    bool start() override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (running_) {
            return false; // Already running
        }
        
        running_ = true;
        monitor_thread_ = std::thread(&HealthMonitorImpl::monitoringLoop, this);
        return true;
    }

    void stop() override {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!running_) {
                return;
            }
            running_ = false;
        }

        if (monitor_thread_.joinable()) {
            monitor_thread_.join();
        }
    }

    ComponentHealth getComponentHealth(const std::string& component_id) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = component_health_.find(component_id);
        if (it != component_health_.end()) {
            return it->second;
        }
        
        // Create default component health for unknown components
        ComponentHealth health;
        health.component_id = component_id;
        health.status = HealthStatus::UNKNOWN;
        health.health_percentage = 0.0f;
        health.last_updated = std::chrono::system_clock::now();
        return health;
    }

    std::vector<ComponentHealth> getAllComponentHealth() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<ComponentHealth> result;
        result.reserve(component_health_.size());
        
        for (const auto& pair : component_health_) {
            result.push_back(pair.second);
        }
        return result;
    }

    int registerStatusCallback(HealthStatusCallback callback, 
                             ComponentType component_type) override {
        std::lock_guard<std::mutex> lock(mutex_);
        int id = next_callback_id_++;
        callbacks_.push_back({id, callback, component_type});
        return id;
    }

    void unregisterStatusCallback(int callback_id) override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = std::find_if(callbacks_.begin(), callbacks_.end(),
            [callback_id](const CallbackEntry& entry) {
                return entry.id == callback_id;
            });
        
        if (it != callbacks_.end()) {
            callbacks_.erase(it);
        }
    }

    void configureAlert(const HealthAlertConfig& config) override {
        std::lock_guard<std::mutex> lock(mutex_);
        alert_configs_[config.component].push_back(config);
    }

    RadiationData getRadiationData() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return current_radiation_;
    }

    TemperatureData getTemperature(ComponentType component,
                                  const std::string& sensor_id) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto key = sensor_id.empty() ? 
            std::to_string(static_cast<int>(component)) : sensor_id;
        
        auto it = temperature_data_.find(key);
        if (it != temperature_data_.end()) {
            return it->second;
        }
        
        // Return empty data if not found
        TemperatureData data;
        data.component = component;
        data.sensor_id = sensor_id;
        data.temperature_celsius = 0.0f;
        data.timestamp = std::chrono::system_clock::now();
        return data;
    }

    bool initiateRecovery(const std::string& component_id) override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = component_health_.find(component_id);
        if (it == component_health_.end()) {
            return false;
        }

        // Log recovery attempt
        log_info("Initiating recovery for component: " + component_id);
        
        // Implement recovery logic here
        // For now, just mark as degraded and requiring attention
        it->second.status = HealthStatus::DEGRADED;
        it->second.diagnostic_info = "Recovery procedure initiated";
        it->second.last_updated = std::chrono::system_clock::now();
        
        notifyStatusChange(it->second);
        return true;
    }

    bool reportToGround(bool full_report) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Prepare report
        std::stringstream report;
        report << "Health Status Report - " 
               << (full_report ? "Full" : "Summary") << "\n";
        
        // Add radiation data
        report << "Radiation - Total Dose: " << current_radiation_.total_dose
               << " rads, Rate: " << current_radiation_.dose_rate << " rads/hour\n";
        
        // Add component health
        for (const auto& pair : component_health_) {
            const auto& health = pair.second;
            report << "Component " << health.component_id 
                   << " - Status: " << static_cast<int>(health.status)
                   << ", Health: " << health.health_percentage << "%\n";
            
            if (full_report && !health.diagnostic_info.empty()) {
                report << "  Info: " << health.diagnostic_info << "\n";
            }
        }
        
        // In a real implementation, this would send the report to ground
        log_info("Sending health report to ground:\n" + report.str());
        return true;
    }

private:
    void monitoringLoop() {
        log_info("Health monitoring loop started");
        
        while (running_) {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                updateRadiationData();
                updateTemperatureData();
                checkComponentHealth();
            }
            
            std::this_thread::sleep_for(
                std::chrono::milliseconds(polling_interval_ms_));
        }
        
        log_info("Health monitoring loop stopped");
    }

    void updateRadiationData() {
        // Simulate radiation monitoring
        // In a real implementation, this would read from radiation sensors
        current_radiation_.timestamp = std::chrono::system_clock::now();
        // Add some random variation to simulate real measurements
        current_radiation_.dose_rate += 
            (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.1f;
        current_radiation_.total_dose += 
            current_radiation_.dose_rate * 
            (polling_interval_ms_ / 3600000.0f);  // Convert to hours
    }

    void updateTemperatureData() {
        // Simulate temperature monitoring
        // In a real implementation, this would read from temperature sensors
        for (auto& pair : temperature_data_) {
            pair.second.timestamp = std::chrono::system_clock::now();
            // Add some random variation
            pair.second.temperature_celsius += 
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.5f;
        }
    }

    void checkComponentHealth() {
        for (auto& pair : component_health_) {
            auto& health = pair.second;
            
            // Check temperature thresholds
            auto temp_data = getTemperature(health.type, "");
            if (temp_data.temperature_celsius > 80.0f) {
                health.status = HealthStatus::CRITICAL;
                health.diagnostic_info = "Temperature critically high";
                notifyStatusChange(health);
            }
            else if (temp_data.temperature_celsius > 60.0f) {
                health.status = HealthStatus::WARNING;
                health.diagnostic_info = "Temperature elevated";
                notifyStatusChange(health);
            }
            
            // Check radiation effects
            if (current_radiation_.dose_rate > 1000.0f) {
                health.status = HealthStatus::WARNING;
                health.diagnostic_info = "High radiation exposure";
                notifyStatusChange(health);
            }
            
            // Update health percentage based on various factors
            updateHealthPercentage(health);
        }
    }

    void updateHealthPercentage(ComponentHealth& health) {
        // Calculate health percentage based on multiple factors
        float temp_factor = 1.0f;
        float radiation_factor = 1.0f;
        float time_factor = 1.0f;
        
        // Temperature impact
        auto temp_data = getTemperature(health.type, "");
        if (temp_data.temperature_celsius > 60.0f) {
            temp_factor = 1.0f - ((temp_data.temperature_celsius - 60.0f) / 40.0f);
        }
        
        // Radiation impact
        if (current_radiation_.dose_rate > 100.0f) {
            radiation_factor = 1.0f - (current_radiation_.dose_rate / 2000.0f);
        }
        
        // Time-based degradation
        auto age = std::chrono::duration_cast<std::chrono::hours>(
            std::chrono::system_clock::now() - health.last_updated).count();
        time_factor = 1.0f - (age / 8760.0f);  // Approximate 1-year degradation
        
        // Calculate overall health percentage
        health.health_percentage = 100.0f * 
            std::min({temp_factor, radiation_factor, time_factor});
        health.health_percentage = std::max(0.0f, 
            std::min(100.0f, health.health_percentage));
    }

    void notifyStatusChange(const ComponentHealth& health) {
        // Notify all registered callbacks that match the component type
        for (const auto& entry : callbacks_) {
            if (entry.filter_type == health.type) {
                try {
                    entry.callback(health);
                }
                catch (const std::exception& e) {
                    log_error("Exception in health status callback: " + 
                             std::string(e.what()));
                }
                catch (...) {
                    log_error("Unknown exception in health status callback");
                }
            }
        }
        
        // Check alert configurations
        auto it = alert_configs_.find(health.type);
        if (it != alert_configs_.end()) {
            for (const auto& config : it->second) {
                if (health.status == config.trigger_status) {
                    if (config.notify_ground) {
                        reportToGround(true);
                    }
                    if (config.auto_recovery) {
                        initiateRecovery(health.component_id);
                    }
                }
            }
        }
    }
};

// Factory function implementation
std::unique_ptr<HealthMonitor> createHealthMonitor(const std::string& config_path) {
    auto monitor = std::make_unique<HealthMonitorImpl>();
    
    // Initialize with default polling interval
    if (!monitor->initialize(1000)) {
        return nullptr;
    }
    
    // Load configuration if path provided
    if (!config_path.empty()) {
        // TODO: Load configuration from file
        // For now, just log that we received a config path
        log_info("Health monitor created with config path: " + config_path);
    }
    
    return monitor;
}

} // namespace core
} // namespace skymesh

