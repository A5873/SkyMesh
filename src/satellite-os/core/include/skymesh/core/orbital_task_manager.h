/**
 * @file orbital_task_manager.h
 * @brief Satellite Orbital Task Management System
 *
 * Defines the interface for managing satellite orbital tasks,
 * including scheduling, prioritization, radiation-tolerant execution,
 * and task lifecycle management.
 */

#ifndef SKYMESH_ORBITAL_TASK_MANAGER_H
#define SKYMESH_ORBITAL_TASK_MANAGER_H

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <map>
#include <optional>

namespace skymesh {
namespace core {

/**
 * @brief Task execution priority levels
 */
enum class TaskPriority {
    CRITICAL,     ///< Highest priority, mission-critical tasks
    HIGH,         ///< High-priority tasks, essential for operation
    NORMAL,       ///< Regular operational tasks
    LOW,          ///< Background tasks, can be delayed
    IDLE          ///< Lowest priority, run only when system is idle
};

/**
 * @brief Task execution status enumeration
 */
enum class TaskStatus {
    PENDING,      ///< Task scheduled but not yet executed
    RUNNING,      ///< Task currently executing
    COMPLETED,    ///< Task executed successfully
    FAILED,       ///< Task execution failed
    CANCELED,     ///< Task execution was canceled
    SUSPENDED     ///< Task execution temporarily suspended
};

/**
 * @brief Task types for orbital operations
 */
enum class TaskType {
    COMMUNICATION,      ///< RF communication tasks
    POWER_MANAGEMENT,   ///< Power system related tasks
    TELEMETRY,          ///< Collecting and processing telemetry data
    ATTITUDE_CONTROL,   ///< Satellite orientation adjustment
    ORBITAL_MANEUVER,   ///< Orbital position adjustment
    PAYLOAD_OPERATION,  ///< Mission-specific payload tasks
    HEALTH_CHECK,       ///< System health verification
    MAINTENANCE,        ///< System maintenance operations
    FIRMWARE_UPDATE     ///< Software/firmware updates
};

/**
 * @brief Error recovery strategies for radiation events
 */
enum class RecoveryStrategy {
    RETRY,              ///< Simply retry the task
    CHECKPOINT_RESTORE, ///< Restore from last saved checkpoint
    ALTERNATE_ROUTINE,  ///< Use an alternative implementation
    GROUND_ASSISTANCE,  ///< Request assistance from ground control
    SAFE_MODE           ///< Enter safe mode and await instructions
};

/**
 * @brief Task execution context
 */
struct TaskContext {
    uint64_t memory_limit_bytes;     ///< Maximum memory allocation
    uint32_t cpu_time_limit_ms;      ///< Maximum CPU time allocation
    bool allow_io_operations;        ///< Whether I/O operations are permitted
    bool allow_critical_subsystems;  ///< Whether task can access critical subsystems
    std::map<std::string, std::string> environment_vars; ///< Task environment variables
};

/**
 * @brief Orbital task definition
 */
struct OrbitalTask {
    std::string task_id;                         ///< Unique task identifier
    std::string name;                            ///< Human-readable name
    TaskType type;                               ///< Task type
    TaskPriority priority;                       ///< Task priority
    std::function<bool(const TaskContext&)> task_function;  ///< Function to execute
    std::chrono::system_clock::time_point scheduled_time;   ///< Scheduled execution time
    std::chrono::milliseconds timeout;           ///< Maximum execution time
    RecoveryStrategy recovery_strategy;          ///< Strategy for handling execution failures
    bool radiation_protected;                    ///< Whether task uses radiation protection
    uint32_t retry_count;                        ///< Number of retry attempts for failures
    std::map<std::string, std::string> metadata; ///< Additional task metadata
};

/**
 * @brief Task execution result
 */
struct TaskResult {
    std::string task_id;                         ///< Task identifier
    TaskStatus status;                           ///< Final task status
    std::chrono::system_clock::time_point start_time;  ///< Execution start time
    std::chrono::system_clock::time_point end_time;    ///< Execution end time
    std::string error_message;                   ///< Error details if failed
    std::map<std::string, std::string> output_data;    ///< Task output data
    uint32_t retry_attempts;                     ///< Number of retry attempts performed
    bool radiation_event_detected;               ///< Whether radiation event was detected
};

/**
 * @brief Orbit position information
 */
struct OrbitPosition {
    double altitude_km;                          ///< Altitude in kilometers
    double latitude;                             ///< Latitude in degrees
    double longitude;                            ///< Longitude in degrees
    double velocity_kmps;                        ///< Velocity in km/s
    std::chrono::system_clock::time_point timestamp;   ///< Position timestamp
};

/**
 * @brief Schedule trigger conditions
 */
struct TriggerCondition {
    std::optional<OrbitPosition> orbit_position; ///< Trigger at specific orbit position
    std::optional<std::string> event_name;       ///< Trigger on named event
    std::optional<std::chrono::system_clock::time_point> time_point; ///< Trigger at specific time
    std::optional<std::string> dependency_task_id; ///< Trigger after another task completes
};

/**
 * @brief Task completion notification callback
 */
using TaskCompletionCallback = std::function<void(const TaskResult&)>;

/**
 * @brief Interface for the satellite orbital task management system
 */
class OrbitalTaskManager {
public:
    virtual ~OrbitalTaskManager() = default;

    /**
     * @brief Initialize the task manager
     * @param config_path Path to configuration file
     * @return true if initialization successful
     */
    virtual bool initialize(const std::string& config_path = "") = 0;

    /**
     * @brief Start the task management system
     * @return true if successfully started
     */
    virtual bool start() = 0;

    /**
     * @brief Stop the task management system
     */
    virtual void stop() = 0;

    /**
     * @brief Schedule a task for one-time execution
     * @param task Task to schedule
     * @return task_id if scheduling successful, empty string otherwise
     */
    virtual std::string scheduleTask(const OrbitalTask& task) = 0;

    /**
     * @brief Schedule a task based on a trigger condition
     * @param task Task to schedule
     * @param trigger Condition that triggers task execution
     * @return task_id if scheduling successful, empty string otherwise
     */
    virtual std::string scheduleConditionalTask(const OrbitalTask& task, 
                                               const TriggerCondition& trigger) = 0;

    /**
     * @brief Schedule a recurring task
     * @param task Task to schedule
     * @param interval Time interval between executions
     * @return task_id if scheduling successful, empty string otherwise
     */
    virtual std::string scheduleRecurringTask(const OrbitalTask& task,
                                             std::chrono::milliseconds interval) = 0;

    /**
     * @brief Cancel a scheduled task
     * @param task_id ID of the task to cancel
     * @return true if cancellation successful
     */
    virtual bool cancelTask(const std::string& task_id) = 0;

    /**
     * @brief Suspend a running or scheduled task
     * @param task_id ID of the task to suspend
     * @return true if suspension successful
     */
    virtual bool suspendTask(const std::string& task_id) = 0;

    /**
     * @brief Resume a suspended task
     * @param task_id ID of the task to resume
     * @return true if resumption successful
     */
    virtual bool resumeTask(const std::string& task_id) = 0;

    /**
     * @brief Get the current status of a task
     * @param task_id ID of the task
     * @return Current task status
     */
    virtual TaskStatus getTaskStatus(const std::string& task_id) const = 0;

    /**
     * @brief Get the result of a completed task
     * @param task_id ID of the completed task
     * @return Task execution result if available
     */
    virtual std::optional<TaskResult> getTaskResult(const std::string& task_id) const = 0;

    /**
     * @brief Get all scheduled tasks
     * @return Vector of all scheduled tasks
     */
    virtual std::vector<OrbitalTask> getAllScheduledTasks() const = 0;

    /**
     * @brief Get all tasks with a specific status
     * @param status Status to filter by
     * @return Vector of tasks with the specified status
     */
    virtual std::vector<OrbitalTask> getTasksByStatus(TaskStatus status) const = 0;

    /**
     * @brief Register callback for task completion notification
     * @param callback Function to call when a task completes
     * @param task_type Optional task type to filter notifications
     * @return Callback ID for later removal
     */
    virtual int registerCompletionCallback(TaskCompletionCallback callback,
                                          TaskType task_type = TaskType::COMMUNICATION) = 0;

    /**
     * @brief Unregister a previously registered callback
     * @param callback_id ID returned from registerCompletionCallback
     */
    virtual void unregisterCompletionCallback(int callback_id) = 0;

    /**
     * @brief Update current orbital position
     * @param position New orbital position
     */
    virtual void updateOrbitalPosition(const OrbitPosition& position) = 0;

    /**
     * @brief Get current orbital position
     * @return Current orbital position
     */
    virtual OrbitPosition getCurrentOrbitalPosition() const = 0;

    /**
     * @brief Trigger recovery for a failed task
     * @param task_id ID of the failed task
     * @param strategy Recovery strategy to use
     * @return true if recovery initiated successfully
     */
    virtual bool recoverTask(const std::string& task_id, 
                            RecoveryStrategy strategy = RecoveryStrategy::RETRY) = 0;

    /**
     * @brief Report task execution metrics to ground station
     * @return true if report was queued successfully
     */
    virtual bool reportTaskMetrics() = 0;
};

/**
 * @brief Factory function to create orbital task manager instance
 * @param config_path Path to configuration file (optional)
 * @return Unique pointer to OrbitalTaskManager implementation
 */
std::unique_ptr<OrbitalTaskManager> createOrbitalTaskManager(const std::string& config_path = "");

} // namespace core
} // namespace skymesh

#endif // SKYMESH_ORBITAL_TASK_MANAGER_H

