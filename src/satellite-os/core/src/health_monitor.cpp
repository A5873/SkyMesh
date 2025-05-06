/**
 * @file health_monitor.cpp
 * @brief Implementation of the Satellite Health Monitoring System
 */

#include "skymesh/core/health_monitor.h"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <atomic>

namespace skymesh {
namespace core {

/**
 * @brief Concrete implementation of the HealthMonitor interface
 */
class HealthMonitorImpl : public HealthMonitor {
public:
    HealthMonitorImpl(const std::string& config_path) 
        : m_polling_interval_ms(1000),
          m_running(false),
          m_next_callback_id(0) {
        if (!config_path.empty()) {
            loadConfiguration(config_path);
        }
        initializeDefaultComponents();
    }

    ~HealthMonitorImpl() override {
        stop();
    }

    bool initialize(uint32_t polling_interval_ms = 1000) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_polling_interval_ms = polling_interval_ms;
        return true;
    }

    bool start() override {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_running) {
            return true; // Already running
        }
        
        m_running = true;
        m_monitor_thread = std::thread(&HealthMonitorImpl::monitoringLoop, this);
        return true;
    }

    void stop() override {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_running) {
                return; // Already stopped
            }
            m_running = false;
        }
        
        if (m_monitor_thread.joinable()) {
            m_monitor_thread.join();
        }
    }

    ComponentHealth getComponentHealth(const std::string& component_id) const override {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_component_health.find(component_id);
        if (it != m_component_health.end()) {
            return it->second;
        }
        
        // Return unknown status if component not found
        ComponentHealth unknown;
        unknown.component_id = component_id;
        unknown.status = HealthStatus::UNKNOWN;
        unknown.health_percentage = 0.0f;
        unknown.diagnostic_info = "Component not registered with health monitor";
        unknown.last_updated = std::chrono::system_clock::now();
        return unknown;
    }

    std::vector<ComponentHealth> getAllComponentHealth() const override {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<ComponentHealth> result;
        result.reserve(m_component_health.size());
        
        for (const auto& pair : m_component_health) {
            result.push_back(pair.second);
        }
        
        return result;
    }

    int registerStatusCallback(HealthStatusCallback callback,
                               ComponentType component_type = ComponentType::PROCESSOR) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        int callback_id = m_next_callback_id++;
        
        m_callbacks[callback_id] = {callback, component_type};
        return callback_id;
    }

    void unregisterStatusCallback(int callback_id) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_callbacks.erase(callback_id);
    }

    void configureAlert(const HealthAlertConfig& config) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Use component type as key for alert configuration
        std::string key = std::to_string(static_cast<int>(config.component));
        m_alert_configs[key] = config;
    }

    RadiationData getRadiationData() const override {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_radiation_data;
    }

    TemperatureData getTemperature(ComponentType component,
                                   const std::string& sensor_id = "") const override {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Look for specific sensor if provided
        if (!sensor_id.empty()) {
            auto key = makeTemperatureKey(component, sensor_id);
            auto it = m_temperature_data.find(key);
            if (it != m_temperature_data.end()) {
                return it->second;
            }
        }
        
        // Return first sensor for the component type if specific sensor not found
        for (const auto& pair : m_temperature_data) {
            if (pair.second.component == component) {
                return pair.second;
            }
        }
        
        // Return empty data if not found
        TemperatureData empty;
        empty.component = component;
        empty.sensor_id = sensor_id;
        empty.temperature_celsius = -273.15f; // Absolute zero indicates invalid reading
        empty.timestamp = std::chrono::system_clock::now();
        return empty;
    }

    bool initiateRecovery(const std::string& component_id) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_component_health.find(component_id);
        if (it == m_component_health.end()) {
            return false;
        }
        
        // Attempt recovery based on component type
        ComponentHealth& health = it->second;
        
        // Log recovery attempt
        std::cout << "Initiating recovery for component: " << component_id 
                  << " (Type: " << static_cast<int>(health.type) << ")" << std::endl;
        
        // Simulate recovery process
        health.status = HealthStatus::DEGRADED;
        health.health_percentage = std::min(health.health_percentage + 20.0f, 80.0f);
        health.diagnostic_info = "Recovery initiated at " + 
            std::to_string(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
        health.last_updated = std::chrono::system_clock::now();
        
        // Notify callbacks
        notifyStatusChange(health);
        
        return true;
    }

    bool reportToGround(bool full_report = false) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // In a real implementation, this would format and queue telemetry data
        // for transmission to ground station via the communication subsystem
        
        // For demonstration, log the report
        std::cout << "Health report generated at: " 
                  << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) << std::endl;
        
        if (full_report) {
            std::cout << "Full health report - Components: " << m_component_health.size() << std::endl;
            for (const auto& pair : m_component_health) {
                const auto& health = pair.second;
                std::cout << "  Component: " << health.component_id 
                          << ", Status: " << static_cast<int>(health.status)
                          << ", Health: " << health.health_percentage << "%" << std::endl;
            }
        } else {
            // Report only critical or failing components
            int critical_count = 0;
            for (const auto& pair : m_component_health) {
                const auto& health = pair.second;
                if (health.status == HealthStatus::CRITICAL || health.status == HealthStatus::FAILED) {
                    critical_count++;
                    std::cout << "  Critical Component: " << health.component_id 
                              << ", Status: " << static_cast<int>(health.status)
                              << ", Health: " << health.health_percentage << "%" << std::endl;
                }
            }
            std::cout << "Health summary - Critical components: " << critical_count << std::endl;
        }
        
        return true;
    }

private:
    /**
     * @brief Load configuration from file
     * @param config_path Path to configuration file
     */
    void loadConfiguration(const std::string& config_path) {
        std::ifstream config_file(config_path);
        if (!config_file.is_open()) {
            std::cerr << "Failed to open health monitor configuration file: " << config_path << std::endl;
            return;
        }
        
        // In a real implementation, parse the configuration file
        // For this example, we'll use default values
    }
    
    /**
     * @brief Initialize default components for monitoring
     */
    void initializeDefaultComponents() {
        // Add some default components for monitoring
        auto now = std::chrono::system_clock::now();
        
        // Power system
        ComponentHealth power;
        power.type = ComponentType::POWER_SYSTEM;
        power.component_id = "power_main";
        power.status = HealthStatus::NOMINAL;
        power.health_percentage = 98.5f;
        power.diagnostic_info = "Nominal operation";
        power.last_updated = now;
        m_component_health["power_main"] = power;
        
        // Communication system
        ComponentHealth comm;
        comm.type = ComponentType::COMMUNICATION_SYSTEM;
        comm.component_id = "uhf_transceiver";
        comm.status = HealthStatus::NOMINAL;
        comm.health_percentage = 95.0f;
        comm.diagnostic_info = "Signal strength normal";
        comm.last_updated = now;
        m_component_health["uhf_transceiver"] = comm;
        
        // Processor
        ComponentHealth cpu;
        cpu.type = ComponentType::PROCESSOR;
        cpu.component_id = "main_processor";
        cpu.status = HealthStatus::NOMINAL;
        cpu.health_percentage = 99.0f;
        cpu.diagnostic_info = "Operating within thermal limits";
        cpu.last_updated = now;
        m_component_health["main_processor"] = cpu;
        
        // Initialize temperature sensors
        TemperatureData temp_cpu;
        temp_cpu.component = ComponentType::PROCESSOR;
        temp_cpu.sensor_id = "cpu_temp";
        temp_cpu.temperature_celsius = 45.2f;
        temp_cpu.timestamp = now;
        m_temperature_data[makeTemperatureKey(temp_cpu.component, temp_cpu.sensor_id)] = temp_cpu;
        
        TemperatureData temp_comm;
        temp_comm.component = ComponentType::COMMUNICATION_SYSTEM;
        temp_comm.sensor_id = "radio_temp";
        temp_comm.temperature_celsius = 32.7f;
        temp_comm.timestamp = now;
        m_temperature_data[makeTemperatureKey(temp_comm.component, temp_comm.sensor_id)] = temp_comm;
        
        // Initialize radiation data
        m_radiation_data.total_dose = 0.125f;
        m_radiation_data.dose_rate = 0.00021f;
        m_radiation_data.single_event_upsets = 0;
        m_radiation_data.timestamp = now;
    }
    
    /**
     * @brief Main monitoring thread loop
     */
    void monitoringLoop() {
        while (m_running) {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                updateHealthStatus();
            }
            
            // Sleep for the configured interval
            std::this_thread::sleep_for(std::chrono::milliseconds(m_polling_interval_ms));
        }
    }
    
    /**
     * @brief Update health status of all components
     */
    void updateHealthStatus() {
        auto now = std::chrono::system_clock::now();
        
        // Update radiation data
        m_radiation_data.total_dose += m_radiation_data.dose_rate * 
            (m_polling_interval_ms / 3600000.0f); // Convert ms to hours
        
        // Simulate small random fluctuations in dose rate
        float random_factor = 0.9f + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.2f);
        m_radiation_data.dose_rate *= random_factor;
        m_radiation_data.timestamp = now;
        
        // Update temperature readings
        for (auto& pair : m_temperature_data) {
            TemperatureData& temp = pair.second;
            
            // Simulate small temperature fluctuations
            float delta = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.6f) - 0.3f;
            temp.temperature_celsius += delta;
            temp.timestamp = now;
        }
        
        // Update component health
        for (auto& pair : m_component_health) {
            ComponentHealth& health = pair.second;
            ComponentHealth old_health = health;
            
            // For demonstration, occasionally degrade some components
            if (rand() % 100 == 0) {
                health.health_percentage -= 1.0f;
                if (health.health_percentage < 70.0f) {
                    health.status = HealthStatus::DEGRADED;
                    health.diagnostic_info = "Performance degradation detected";
                }
                if (health.health_percentage < 40.0f) {
                    health.status = HealthStatus::WARNING;
                    health.diagnostic_info = "Component showing signs of failure";
                }
                if (health.health_percentage < 20.0f) {
                    health.status = HealthStatus::CRITICAL;
                    health.diagnostic_info = "Component critically degraded";
                }
                if (health.health_percentage < 5.0f) {
                    health.status = HealthStatus::FAILED;
                    health.diagnostic_info = "Component has failed";
                }
                
                health.last_updated = now;
                
                // If status changed, notify callbacks
                if (health.status != old_health.status) {
                    notifyStatusChange(health);
                    
                    // Check if this should trigger an alert
                    checkAndTriggerAlert(health);
                }
            }
        }
    }
    
    /**
     * @brief Notify registered callbacks of a status change
     * @param health The updated component health
     */
    void notifyStatusChange(const ComponentHealth& health) {
        for (const auto& pair : m_callbacks) {
            const auto& callback_info = pair.second;
            
            // Call if callback is registered for all components or this specific type
            if (callback_info.component_type == health

