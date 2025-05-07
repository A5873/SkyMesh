/**
 * @file orbital_task_manager.cpp
 * @brief Implementation of the Orbital Task Manager for satellite operations
 * 
 * This file implements the OrbitalTaskManager interface defined in orbital_task_manager.h.
 * It provides functionality for scheduling, executing, and managing tasks in a radiation-hardened
 * satellite environment.
 */

#include "skymesh/core/orbital_task_manager.h"

#include <condition_variable>
#include <functional>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <queue>
#include <random>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <utility>
#include <ctime>
#include <cmath>
#include <ctime>
#include <sstream>
#include <iomanip>

// Forward declarations for internal logging
namespace {
    void log_debug(const std::string& message);
    void log_info(const std::string& message);
    void log_warning(const std::string& message);
    void log_error(const std::string& message);
    std::string generate_task_id();
    std::string timestamp_to_string(const std::chrono::system_clock::time_point& time);
}

namespace skymesh {
namespace core {

/**
 * @brief Implementation of the OrbitalTaskManager interface.
 * 
 * This class manages satellite tasks with radiation protection, prioritization,
 * and task lifecycle management in the orbital environment.
 */
class OrbitalTaskManagerImpl : public OrbitalTaskManager {
public:
    OrbitalTaskManagerImpl();
    ~OrbitalTaskManagerImpl() override;

    // Interface implementation
    bool initialize(const std::string& config_path) override;
    bool start() override;
    void stop() override;
    std::string scheduleTask(const OrbitalTask& task) override;
    std::string scheduleConditionalTask(const OrbitalTask& task, const TriggerCondition& trigger) override;
    std::string scheduleRecurringTask(const OrbitalTask& task, std::chrono::milliseconds interval) override;
    bool cancelTask(const std::string& task_id) override;
    bool suspendTask(const std::string& task_id) override;
    bool resumeTask(const std::string& task_id) override;
    TaskStatus getTaskStatus(const std::string& task_id) const override;
    std::optional<TaskResult> getTaskResult(const std::string& task_id) const override;
    std::vector<OrbitalTask> getAllScheduledTasks() const override;
    std::vector<OrbitalTask> getTasksByStatus(TaskStatus status) const override;
    int registerCompletionCallback(TaskCompletionCallback callback, TaskType task_type) override;
    void unregisterCompletionCallback(int callback_id) override;
    void updateOrbitalPosition(const OrbitPosition& position) override;
    OrbitPosition getCurrentOrbitalPosition() const override;
    bool recoverTask(const std::string& task_id, RecoveryStrategy strategy) override;
    bool reportTaskMetrics() override;

private:
    // Internal task structure with additional metadata
    struct TaskEntry {
        OrbitalTask task;
        TaskStatus status;
        std::chrono::system_clock::time_point actual_start_time;
        std::chrono::system_clock::time_point actual_end_time;
        std::string error_message;
        uint32_t actual_retry_count;
        std::map<std::string, std::string> result_data;
        bool is_recurring;
        std::chrono::milliseconds recurring_interval;
        TriggerCondition trigger_condition;
        bool radiation_event_detected;
        
        // Comparison operator for priority queue
        bool operator<(const TaskEntry& other) const {
            // If priorities differ, higher priority tasks come first
            if (task.priority != other.task.priority) {
                return task.priority > other.task.priority;
            }
            
            // Otherwise, earlier scheduled tasks come first
            return task.scheduled_time < other.task.scheduled_time;
        }
    };

    // Task comparison for priority queue
    struct TaskComparator {
        bool operator()(const std::shared_ptr<TaskEntry>& a, const std::shared_ptr<TaskEntry>& b) const {
            return *b < *a; // Reverse comparison for min-heap (we want max priority at top)
        }
    };

    // Callback entry structure
    struct CallbackEntry {
        int id;
        TaskCompletionCallback callback;
        TaskType filter_type;
    };

    // Thread for task execution
    void taskExecutionThread();
    
    // Thread for task scheduling and condition evaluation
    void taskSchedulingThread();
    
    // Execute a single task with radiation protection if needed
    TaskResult executeTask(const std::shared_ptr<TaskEntry>& task_entry);
    
    // Evaluate if a task with trigger conditions should be executed
    bool shouldExecuteConditionalTask(const TriggerCondition& condition) const;
    
    // Execute a function with Triple Modular Redundancy
    bool executeWithTMR(const std::function<bool(const TaskContext&)>& func, const TaskContext& context);
    
    // Create a task result structure from a task entry
    TaskResult createTaskResult(const std::shared_ptr<TaskEntry>& task_entry) const;
    
    // Notify all registered callbacks about task completion
    void notifyTaskCompletion(const TaskResult& result);
    
    // Check if a position matches the orbit trigger condition
    bool matchesOrbitPosition(const OrbitPosition& current, const OrbitPosition& trigger) const;
    
    // Thread-safe task storage
    mutable std::mutex tasks_mutex_;
    std::unordered_map<std::string, std::shared_ptr<TaskEntry>> task_map_;
    
    // Priority queue for scheduled tasks
    std::mutex queue_mutex_;
    std::priority_queue<std::shared_ptr<TaskEntry>, 
                        std::vector<std::shared_ptr<TaskEntry>>, 
                        TaskComparator> task_queue_;
    
    // Conditional tasks waiting for trigger conditions
    std::vector<std::shared_ptr<TaskEntry>> conditional_tasks_;
    
    // Completed task results
    mutable std::mutex results_mutex_;
    std::unordered_map<std::string, TaskResult> task_results_;
    
    // Thread synchronization
    std::condition_variable queue_condition_;
    std::mutex execution_mutex_;
    std::condition_variable execution_condition_;
    
    // Execution thread
    std::thread execution_thread_;
    std::thread scheduling_thread_;
    std::atomic<bool> running_{false};
    
    // Current orbital position
    mutable std::mutex position_mutex_;
    OrbitPosition current_position_;
    
    // Task completion callbacks
    mutable std::mutex callback_mutex_;
    std::vector<CallbackEntry> callbacks_;
    int next_callback_id_{0};
    
    // Task execution metrics
    std::atomic<uint64_t> tasks_executed_{0};
    std::atomic<uint64_t> tasks_failed_{0};
    std::atomic<uint64_t> radiation_events_{0};
    
    // Default task context when none provided
    TaskContext default_context_;
};

OrbitalTaskManagerImpl::OrbitalTaskManagerImpl() {
    // Initialize default task context
    default_context_.memory_limit_bytes = 1024 * 1024; // 1MB
    default_context_.cpu_time_limit_ms = 5000; // 5 seconds
    default_context_.allow_io_operations = true;
    default_context_.allow_critical_subsystems = false;
    
    // Initialize default orbital position
    current_position_.altitude_km = 550.0;
    current_position_.latitude = 0.0;
    current_position_.longitude = 0.0;
    current_position_.velocity_kmps = 7.6; // Typical LEO velocity
    current_position_.timestamp = std::chrono::system_clock::now();
}

OrbitalTaskManagerImpl::~OrbitalTaskManagerImpl() {
    stop();
}

bool OrbitalTaskManagerImpl::initialize(const std::string& config_path) {
    log_info("Initializing OrbitalTaskManager" + 
             (config_path.empty() ? "" : " with config: " + config_path));
    
    // Load configuration if provided
    if (!config_path.empty()) {
        // Config file processing would go here
        // For now, we'll just use default settings
    }
    
    return true;
}

bool OrbitalTaskManagerImpl::start() {
    std::lock_guard<std::mutex> lock(execution_mutex_);
    
    if (running_) {
        log_warning("OrbitalTaskManager already running");
        return false;
    }
    
    log_info("Starting OrbitalTaskManager");
    running_ = true;
    
    // Start execution thread
    execution_thread_ = std::thread(&OrbitalTaskManagerImpl::taskExecutionThread, this);
    
    // Start scheduling thread
    scheduling_thread_ = std::thread(&OrbitalTaskManagerImpl::taskSchedulingThread, this);
    
    return true;
}

void OrbitalTaskManagerImpl::stop() {
    {
        std::lock_guard<std::mutex> lock(execution_mutex_);
        
        if (!running_) {
            return;
        }
        
        log_info("Stopping OrbitalTaskManager");
        running_ = false;
    }
    
    // Notify all waiting threads
    queue_condition_.notify_all();
    execution_condition_.notify_all();
    
    // Join threads if they are joinable
    if (execution_thread_.joinable()) {
        execution_thread_.join();
    }
    
    if (scheduling_thread_.joinable()) {
        scheduling_thread_.join();
    }
    
    log_info("OrbitalTaskManager stopped");
}

std::string OrbitalTaskManagerImpl::scheduleTask(const OrbitalTask& task) {
    if (!running_) {
        log_error("Cannot schedule task: OrbitalTaskManager not running");
        return "";
    }
    
    auto task_entry = std::make_shared<TaskEntry>();
    task_entry->task = task;
    
    // Generate task ID if not provided
    if (task_entry->task.task_id.empty()) {
        task_entry->task.task_id = generate_task_id();
    }
    
    task_entry->status = TaskStatus::PENDING;
    task_entry->actual_retry_count = 0;
    task_entry->is_recurring = false;
    task_entry->radiation_event_detected = false;
    
    log_info("Scheduling task: " + task_entry->task.name + " (ID: " + task_entry->task.task_id + ")");
    
    // Add to task map and priority queue
    {
        std::lock_guard<std::mutex> map_lock(tasks_mutex_);
        task_map_[task_entry->task.task_id] = task_entry;
    }
    
    {
        std::lock_guard<std::mutex> queue_lock(queue_mutex_);
        task_queue_.push(task_entry);
    }
    
    // Notify execution thread
    queue_condition_.notify_one();
    
    return task_entry->task.task_id;
}

std::string OrbitalTaskManagerImpl::scheduleConditionalTask(
    const OrbitalTask& task, const TriggerCondition& trigger) {
    
    if (!running_) {
        log_error("Cannot schedule conditional task: OrbitalTaskManager not running");
        return "";
    }
    
    auto task_entry = std::make_shared<TaskEntry>();
    task_entry->task = task;
    
    // Generate task ID if not provided
    if (task_entry->task.task_id.empty()) {
        task_entry->task.task_id = generate_task_id();
    }
    
    task_entry->status = TaskStatus::PENDING;
    task_entry->actual_retry_count = 0;
    task_entry->is_recurring = false;
    task_entry->trigger_condition = trigger;
    task_entry->radiation_event_detected = false;
    
    log_info("Scheduling conditional task: " + task_entry->task.name + 
             " (ID: " + task_entry->task.task_id + ")");
    
    // Add to task map
    {
        std::lock_guard<std::mutex> map_lock(tasks_mutex_);
        task_map_[task_entry->task.task_id] = task_entry;
    }
    
    // Add to conditional tasks list
    {
        std::lock_guard<std::mutex> queue_lock(queue_mutex_);
        conditional_tasks_.push_back(task_entry);
    }
    
    return task_entry->task.task_id;
}

std::string OrbitalTaskManagerImpl::scheduleRecurringTask(
    const OrbitalTask& task, std::chrono::milliseconds interval) {
    
    if (!running_) {
        log_error("Cannot schedule recurring task: OrbitalTaskManager not running");
        return "";
    }
    
    auto task_entry = std::make_shared<TaskEntry>();
    task_entry->task = task;
    
    // Generate task ID if not provided
    if (task_entry->task.task_id.empty()) {
        task_entry->task.task_id = generate_task_id();
    }
    
    task_entry->status = TaskStatus::PENDING;
    task_entry->actual_retry_count = 0;
    task_entry->is_recurring = true;
    task_entry->recurring_interval = interval;
    task_entry->radiation_event_detected = false;
    
    log_info("Scheduling recurring task: " + task_entry->task.name + 
             " (ID: " + task_entry->task.task_id + ") with interval " + 
             std::to_string(interval.count()) + "ms");
    
    // Add to task map and priority queue
    {
        std::lock_guard<std::mutex> map_lock(tasks_mutex_);
        task_map_[task_entry->task.task_id] = task_entry;
    }
    
    {
        std::lock_guard<std::mutex> queue_lock(queue_mutex_);
        task_queue_.push(task_entry);
    }
    
    // Notify execution thread
    queue_condition_.notify_one();
    
    return task_entry->task.task_id;
}

bool OrbitalTaskManagerImpl::cancelTask(const std::string& task_id) {
    std::lock_guard<std::mutex> map_lock(tasks_mutex_);
    
    auto it = task_map_.find(task_id);
    if (it == task_map_.end()) {
        log_warning("Cannot cancel task: Task ID not found: " + task_id);
        return false;
    }
    
    auto task_entry = it->second;
    
    if (task_entry->status == TaskStatus::RUNNING) {
        log_warning("Cannot cancel running task: " + task_id);
        return false;
    }
    
    task_entry->status = TaskStatus::CANCELED;
    log_info("Task canceled: " + task_id);
    
    return true;
}

bool OrbitalTaskManagerImpl::suspendTask(const std::string& task_id) {
    std::lock_guard<std::mutex> map_lock(tasks_mutex_);
    
    auto it = task_map_.find(task_id);
    if (it == task_map_.end()) {
        log_warning("Cannot suspend task: Task ID not found: " + task_id);
        return false;
    }
    
    auto task_entry = it->second;
    
    if (task_entry->status != TaskStatus::RUNNING && 
        task_entry->status != TaskStatus::PENDING) {
        log_warning("Cannot suspend task with status: " + 
                   std::to_string(static_cast<int>(task_entry->status)));
        return false;
    }
    
    task_entry->status = TaskStatus::SUSPENDED;
    log_info("Task suspended: " + task_id);
    
    return true;
}

bool OrbitalTaskManagerImpl::resumeTask(const std::string& task_id) {
    std::lock_guard<std::mutex> map_lock(tasks_mutex_);
    
    auto it = task_map_.find(task_id);
    if (it == task_map_.end()) {
        log_warning("Cannot resume task: Task ID not found: " + task_id);
        return false;
    }
    
    auto task_entry = it->second;
    
    if (task_entry->status != TaskStatus::SUSPENDED) {
        log_warning("Cannot resume task with status: " + 
                   std::to_string(static_cast<int>(task_entry->status)));
        return false;
    }
    
    task_entry->status = TaskStatus::PENDING;
    log_info("Task resumed: " + task_id);
    
    // Re-add to priority queue
    {
        std::lock_guard<std::mutex> queue_lock(queue_mutex_);
        task_queue_.push(task_entry);
    }
    
    // Notify execution thread
    queue_condition_.notify_one();
    
    return true;
}

TaskStatus OrbitalTaskManagerImpl::getTaskStatus(const std::string& task_id) const {
    std::lock_guard<std::mutex> map_lock(tasks_mutex_);
    
    auto it = task_map_.find(task_id);
    if (it == task_map_.end()) {
        log_warning("Task not found for status check: " + task_id);
        return TaskStatus::FAILED; // Default to failed if not found
    }
    
    return it->second->status;
}

std::optional<TaskResult> OrbitalTaskManagerImpl::getTaskResult(const std::string& task_id) const {
    // First check if the task exists
    {
        std::lock_guard<std::mutex> map_lock(tasks_mutex_);
        auto it = task_map_.find(task_id);
        if (it == task_map_.end()) {
            log_warning("Task not found for result retrieval: " + task_id);
            return std::nullopt;
        }
        
        // If task exists but is not completed, return nullopt
        if (it->second->status != TaskStatus::COMPLETED && 
            it->second->status != TaskStatus::FAILED) {
            return std::nullopt;
        }
    }
    
    // Then check for results
    std::lock_guard<std::mutex> results_lock(results_mutex_);
    auto it = task_results_.find(task_id);
    if (it == task_results_.end()) {
        return std::nullopt;
    }
    
    return it->second;
}

std::vector<OrbitalTask> OrbitalTaskManagerImpl::getAllScheduledTasks() const {
    std::lock_guard<std::mutex> map_lock(tasks_mutex_);
    
    std::vector<OrbitalTask> tasks;
    tasks.reserve(task_map_.size());
    
    for (const auto& pair : task_map_) {
        tasks.push_back(pair.second->task);
    }
    
    return tasks;
}

std::vector<OrbitalTask> OrbitalTaskManagerImpl::getTasksByStatus(TaskStatus status) const {
    std::lock_guard<std::mutex> map_lock(tasks_mutex_);
    
    std::vector<OrbitalTask> tasks;
    
    for (const auto& pair : task_map_) {
        if (pair.second->status == status) {
            tasks.push_back(pair.second->task);
        }
    }
    
    return tasks;
}

int OrbitalTaskManagerImpl::registerCompletionCallback(
    TaskCompletionCallback callback, TaskType task_type) {
    
    std::lock_guard<std::mutex> callback_lock(callback_mutex_);
    
    int id = next_callback_id_++;
    callbacks_.push_back({id, callback, task_type});
    
    log_info("Registered completion callback with ID: " + std::to_string(id) + 
             " for task type: " + std::to_string(static_cast<int>(task_type)));
    
    return id;
}

void OrbitalTaskManagerImpl::unregisterCompletionCallback(int callback_id) {
    std::lock_guard<std::mutex> callback_lock(callback_mutex_);
    
    auto it = std::find_if(callbacks_.begin(), callbacks_.end(),
                          [callback_id](const CallbackEntry& entry) {
                              return entry.id == callback_id;
                          });
    
    if (it != callbacks_.end()) {
        callbacks_.erase(it);
        log_info("Unregistered completion callback with ID: " + std::to_string(callback_id));
    } else {
        log_warning("Callback ID not found for unregistration: " + std::to_string(callback_id));
    }
}

void OrbitalTaskManagerImpl::updateOrbitalPosition(const OrbitPosition& position) {
    std::lock_guard<std::mutex> position_lock(position_mutex_);
    
    current_position_ = position;
    
    // Log position update at debug level
    log_debug("Updated orbital position: (" + 
             std::to_string(position.latitude) + ", " + 
             std::to_string(position.longitude) + ") at " + 
             std::to_string(position.altitude_km) + " km");
    
    // Notify scheduling thread to check for position-triggered tasks
    execution_condition_.notify_one();
}

OrbitPosition OrbitalTaskManagerImpl::getCurrentOrbitalPosition() const {
    std::lock_guard<std::mutex> position_lock(position_mutex_);
    return current_position_;
}

bool OrbitalTaskManagerImpl::recoverTask(const std::string& task_id, RecoveryStrategy strategy) {
    std::lock_guard<std::mutex> map_lock(tasks_mutex_);
    
    auto it = task_map_.find(task_id);
    if (it == task_map_.end()) {
        log_warning("Cannot recover task: Task ID not found: " + task_id);
        return false;
    }
    
    auto task_entry = it->second;
    
    if (task_entry->status != TaskStatus::FAILED) {
        log_warning("Cannot recover task with status: " + 
                   std::to_string(static_cast<int>(task_entry->status)));
        return false;
    }
    
    log_info("Recovering task: " + task_id + " with strategy: " + 
            std::to_string(static_cast<int>(strategy)));
    
    // Apply the recovery strategy
    switch (strategy) {
        case RecoveryStrategy::RETRY:
            // Simply retry the task
            task_entry->status = TaskStatus::PENDING;
            task_entry->actual_retry_count = 0; // Reset retry count
            
            // Re-add to priority queue
            {
                std::lock_guard<std::mutex> queue_lock(queue_mutex_);
                task_queue_.push(task_entry);
            }
            
            // Notify execution thread
            queue_condition_.notify_one();
            break;
            
        case RecoveryStrategy::CHECKPOINT_RESTORE:
            // Restore from last checkpoint, if available
            task_entry->status = TaskStatus::PENDING;
            task_entry->actual_retry_count = 0;
            
            // Set task metadata to indicate checkpoint restore
            task_entry->task.metadata["recovery_type"] = "checkpoint";
            
            // Re-add to priority queue
            {
                std::lock_guard<std::mutex> queue_lock(queue_mutex_);
                task_queue_.push(task_entry);
            }
            
            queue_condition_.notify_one();
            break;
            
        case RecoveryStrategy::ALTERNATE_ROUTINE:
            // Use an alternative implementation
            task_entry->status = TaskStatus::PENDING;
            task_entry->actual_retry_count = 0;
            
            // Set task metadata to indicate alternate routine
            task_entry->task.metadata["recovery_type"] = "alternate";
            
            // Re-add to priority queue
            {
                std::lock_guard<std::mutex> queue_lock(queue_mutex_);
                task_queue_.push(task_entry);
            }
            
            queue_condition_.notify_one();
            break;
            
        case RecoveryStrategy::GROUND_ASSISTANCE:
            // Request assistance from ground control
            // In a real system, this would queue a message to ground
            task_entry->status = TaskStatus::SUSPENDED;
            task_entry->task.metadata["recovery_type"] = "ground_assist";
            task_entry->task.metadata["ground_assist_requested"] = timestamp_to_string(
                std::chrono::system_clock::now());
            
            log_info("Ground assistance requested for task: " + task_id);
            break;
            
        case RecoveryStrategy::SAFE_MODE:
            // Enter safe mode and await instructions
            task_entry->status = TaskStatus::SUSPENDED;
            task_entry->task.metadata["recovery_type"] = "safe_mode";
            
            log_warning("Task " + task_id + " triggered SAFE_MODE recovery strategy");
            // In a real system, this would trigger satellite-wide safe mode
            break;
            
        default:
            log_error("Unknown recovery strategy: " + 
                     std::to_string(static_cast<int>(strategy)));
            return false;
    }
    
    return true;
}

bool OrbitalTaskManagerImpl::reportTaskMetrics() {
    // Collect metrics
    uint64_t executed = tasks_executed_.load();
    uint64_t failed = tasks_failed_.load();
    uint64_t radiation = radiation_events_.load();
    
    // Get task counts by status
    std::unordered_map<TaskStatus, int> status_counts;
    
    {
        std::lock_guard<std::mutex> map_lock(tasks_mutex_);
        for (const auto& pair : task_map_) {
            status_counts[pair.second->status]++;
        }
    }
    
    // Log metrics
    std::stringstream ss;
    ss << "Task Metrics Report:" << std::endl
       << "  Tasks Executed: " << executed << std::endl
       << "  Tasks Failed: " << failed << std::endl
       << "  Radiation Events: " << radiation << std::endl
       << "  Tasks by Status:" << std::endl
       << "    Pending: " << status_counts[TaskStatus::PENDING] << std::endl
       << "    Running: " << status_counts[TaskStatus::RUNNING] << std::endl
       << "    Completed: " << status_counts[TaskStatus::COMPLETED] << std::endl
       << "    Failed: " << status_counts[TaskStatus::FAILED] << std::endl
       << "    Canceled: " << status_counts[TaskStatus::CANCELED] << std::endl
       << "    Suspended: " << status_counts[TaskStatus::SUSPENDED] << std::endl;
    
    log_info(ss.str());
    
    // In a real implementation, this would also send the metrics to ground control
    return true;
}

// Implementation of thread methods

void OrbitalTaskManagerImpl::taskExecutionThread() {
    log_info("Task execution thread started");
    
    while (running_) {
        std::shared_ptr<TaskEntry> task_entry;
        
        // Wait for a task to be available in the queue
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_condition_.wait(lock, [this] {
                return !running_ || !task_queue_.empty();
            });
            
            // Check if we should exit
            if (!running_) {
                break;
            }
            
            // Get the highest priority task
            if (!task_queue_.empty()) {
                task_entry = task_queue_.top();
                task_queue_.pop();
            }
        }
        
        // Process the task if we got one
        if (task_entry) {
            // Check if scheduled time has arrived
            auto now = std::chrono::system_clock::now();
            if (task_entry->task.scheduled_time > now) {
                // Task scheduled for the future, put it back in the queue
                std::lock_guard<std::mutex> lock(queue_mutex_);
                task_queue_.push(task_entry);
                
                // Sleep for a short time to avoid busy waiting
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            
            // Check if task is still valid (not canceled or suspended)
            {
                std::lock_guard<std::mutex> lock(tasks_mutex_);
                if (task_entry->status != TaskStatus::PENDING) {
                    // Skip this task
                    continue;
                }
                
                // Mark task as running
                task_entry->status = TaskStatus::RUNNING;
                task_entry->actual_start_time = now;
            }
            
            log_info("Executing task: " + task_entry->task.name + 
                     " (ID: " + task_entry->task.task_id + 
                     ", Type: " + std::to_string(static_cast<int>(task_entry->task.type)) + ")");
            
            // Execute the task
            TaskResult result;
            try {
                result = executeTask(task_entry);
                
                // Update task status
                {
                    std::lock_guard<std::mutex> lock(tasks_mutex_);
                    task_entry->status = result.status;
                    task_entry->actual_end_time = result.end_time;
                    task_entry->error_message = result.error_message;
                    task_entry->result_data = result.output_data;
                    task_entry->radiation_event_detected = result.radiation_event_detected;
                }
                
                // Store result
                {
                    std::lock_guard<std::mutex> lock(results_mutex_);
                    task_results_[task_entry->task.task_id] = result;
                }
                
                // Update metrics
                tasks_executed_++;
                if (result.status == TaskStatus::FAILED) {
                    tasks_failed_++;
                }
                if (result.radiation_event_detected) {
                    radiation_events_++;
                }
                
                // Notify completion callbacks
                notifyTaskCompletion(result);
                
                // Handle recurring tasks
                if (task_entry->is_recurring && result.status == TaskStatus::COMPLETED) {
                    auto next_task = std::make_shared<TaskEntry>(*task_entry);
                    next_task->status = TaskStatus::PENDING;
                    next_task->task.scheduled_time = std::chrono::system_clock::now() + 
                                                     next_task->recurring_interval;
                    
                    // Add to queue
                    {
                        std::lock_guard<std::mutex> lock(queue_mutex_);
                        task_queue_.push(next_task);
                    }
                }
            } 
            catch (const std::exception& e) {
                // Log the error
                log_error("Exception occurred while executing task " + 
                         task_entry->task.task_id + ": " + e.what());
                
                // Update task status
                {
                    std::lock_guard<std::mutex> lock(tasks_mutex_);
                    task_entry->status = TaskStatus::FAILED;
                    task_entry->error_message = "Exception: " + std::string(e.what());
                }
                
                // Update metrics
                tasks_executed_++;
                tasks_failed_++;
            }
            catch (...) {
                // Log the error
                log_error("Unknown exception occurred while executing task " + 
                         task_entry->task.task_id);
                
                // Update task status
                {
                    std::lock_guard<std::mutex> lock(tasks_mutex_);
                    task_entry->status = TaskStatus::FAILED;
                    task_entry->error_message = "Unknown exception";
                }
                
                // Update metrics
                tasks_executed_++;
                tasks_failed_++;
            }
        }
    }
    
    log_info("Task execution thread stopped");
}

void OrbitalTaskManagerImpl::taskSchedulingThread() {
    log_info("Task scheduling thread started");
    
    // How often to check conditional tasks (in milliseconds)
    const auto check_interval = std::chrono::milliseconds(1000);
    
    while (running_) {
        // Sleep for a bit to avoid busy waiting
        {
            std::unique_lock<std::mutex> lock(execution_mutex_);
            execution_condition_.wait_for(lock, check_interval, [this] {
                return !running_;
            });
            
            // Check if we should exit
            if (!running_) {
                break;
            }
        }
        
        // Get current time and position
        auto now = std::chrono::system_clock::now();
        OrbitPosition current_pos;
        {
            std::lock_guard<std::mutex> lock(position_mutex_);
            current_pos = current_position_;
        }
        
        // Check all conditional tasks
        std::vector<std::shared_ptr<TaskEntry>> triggered_tasks;
        
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            
            // Identify triggered tasks
            auto it = conditional_tasks_.begin();
            while (it != conditional_tasks_.end()) {
                auto task_entry = *it;
                
                // Skip if not pending
                if (task_entry->status != TaskStatus::PENDING) {
                    ++it;
                    continue;
                }
                
                // Check if task should be triggered
                bool should_trigger = shouldExecuteConditionalTask(task_entry->trigger_condition);
                
                if (should_trigger) {
                    // Set scheduled time to now
                    task_entry->task.scheduled_time = now;
                    
                    // Add to triggered tasks
                    triggered_tasks.push_back(task_entry);
                    
                    // Remove from conditional tasks if non-recurring
                    if (!task_entry->is_recurring) {
                        it = conditional_tasks_.erase(it);
                    } else {
                        ++it;
                    }
                } else {
                    ++it;
                }
            }
            
            // Add triggered tasks to task queue
            for (const auto& task : triggered_tasks) {
                task_queue_.push(task);
            }
        }
        
        // Notify execution thread if we triggered any tasks
        if (!triggered_tasks.empty()) {
            queue_condition_.notify_one();
            log_debug("Triggered " + std::to_string(triggered_tasks.size()) + " conditional tasks");
        }
    }
    
    log_info("Task scheduling thread stopped");
}

TaskResult OrbitalTaskManagerImpl::executeTask(const std::shared_ptr<TaskEntry>& task_entry) {
    TaskResult result;
    result.task_id = task_entry->task.task_id;
    result.start_time = std::chrono::system_clock::now();
    result.radiation_event_detected = false;
    result.retry_attempts = task_entry->actual_retry_count;
    
    // Configure task context
    TaskContext context = default_context_;
    if (task_entry->task.metadata.count("memory_limit_bytes")) {
        context.memory_limit_bytes = 
            std::stoul(task_entry->task.metadata.at("memory_limit_bytes"));
    }
    if (task_entry->task.metadata.count("cpu_time_limit_ms")) {
        context.cpu_time_limit_ms = 
            std::stoul(task_entry->task.metadata.at("cpu_time_limit_ms"));
    }
    if (task_entry->task.metadata.count("allow_io_operations")) {
        context.allow_io_operations = 
            task_entry->task.metadata.at("allow_io_operations") == "true";
    }
    if (task_entry->task.metadata.count("allow_critical_subsystems")) {
        context.allow_critical_subsystems = 
            task_entry->task.metadata.at("allow_critical_subsystems") == "true";
    }
    
    // Execute the task, with radiation protection if needed
    bool success = false;
    try {
        if (task_entry->task.radiation_protected) {
            // Use triple modular redundancy for critical tasks
            success = executeWithTMR(task_entry->task.task_function, context);
        } else {
            // For non-critical tasks, execute without TMR
            success = task_entry->task.task_function(context);
        }
    }
    catch (const std::exception& e) {
        result.status = TaskStatus::FAILED;
        result.error_message = "Exception during execution: " + std::string(e.what());
        result.end_time = std::chrono::system_clock::now();
        return result;
    }
    catch (...) {
        result.status = TaskStatus::FAILED;
        result.error_message = "Unknown exception during execution";
        result.end_time = std::chrono::system_clock::now();
        return result;
    }
    
    result.end_time = std::chrono::system_clock::now();
    
    // Check for timeout
    auto execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        result.end_time - result.start_time).count();
    
    if (execution_time > task_entry->task.timeout.count()) {
        result.status = TaskStatus::FAILED;
        result.error_message = "Task timed out (took " + std::to_string(execution_time) + 
                              " ms, limit: " + 
                              std::to_string(task_entry->task.timeout.count()) + " ms)";
        return result;
    }
    
    // Handle task success or failure
    if (success) {
        result.status = TaskStatus::COMPLETED;
    } else {
        // Check if we should retry
        if (task_entry->actual_retry_count < task_entry->task.retry_count) {
            // Increment retry count
            task_entry->actual_retry_count++;
            
            // Log the retry
            log_info("Retrying task: " + task_entry->task.task_id + 
                     " (Attempt " + std::to_string(task_entry->actual_retry_count) + 
                     " of " + std::to_string(task_entry->task.retry_count) + ")");
            
            // Re-add to priority queue
            {
                std::lock_guard<std::mutex> lock(queue_mutex_);
                task_queue_.push(task_entry);
            }
            
            // Mark as pending for next execution
            result.status = TaskStatus::PENDING;
        } else {
            result.status = TaskStatus::FAILED;
            result.error_message = "Task failed after " + 
                                  std::to_string(task_entry->actual_retry_count) + " retries";
        }
    }
    
    return result;
}

bool OrbitalTaskManagerImpl::executeWithTMR(
    const std::function<bool(const TaskContext&)>& func, const TaskContext& context) {
    
    log_debug("Executing task with Triple Modular Redundancy");
    
    // Execute function three times independently
    bool result1 = false;
    bool result2 = false;
    bool result3 = false;
    bool radiation_detected = false;
    
    try {
        result1 = func(context);
    } catch (const std::exception& e) {
        log_warning("TMR execution 1 failed with exception: " + std::string(e.what()));
        radiation_detected = true;
    } catch (...) {
        log_warning("TMR execution 1 failed with unknown exception");
        radiation_detected = true;
    }
    
    try {
        result2 = func(context);
    } catch (const std::exception& e) {
        log_warning("TMR execution 2 failed with exception: " + std::string(e.what()));
        radiation_detected = true;
    } catch (...) {
        log_warning("TMR execution 2 failed with unknown exception");
        radiation_detected = true;
    }
    
    try {
        result3 = func(context);
    } catch (const std::exception& e) {
        log_warning("TMR execution 3 failed with exception: " + std::string(e.what()));
        radiation_detected = true;
    } catch (...) {
        log_warning("TMR execution 3 failed with unknown exception");
        radiation_detected = true;
    }
    
    // Majority voting
    if (result1 == result2) {
        // Results 1 and 2 agree
        if (result1 != result3) {
            // Result 3 disagrees - potential radiation event
            log_warning("TMR detected potential radiation event (vote: 2-1)");
            radiation_detected = true;
        }
        return result1;
    } else if (result1 == result3) {
        // Results 1 and 3 agree, result 2 disagrees
        log_warning("TMR detected potential radiation event (vote: 2-1)");
        radiation_detected = true;
        return result1;
    } else if (result2 == result3) {
        // Results 2 and 3 agree, result 1 disagrees
        log_warning("TMR detected potential radiation event (vote: 2-1)");
        radiation_detected = true;
        return result2;
    } else {
        // All three results disagree - critical radiation event
        log_error("TMR critical radiation event detected (all results disagree)");
        radiation_detected = true;
        
        // In this case, we default to the "safer" false result
        // In a real implementation, this might trigger a system-level recovery
        return false;
    }
}

bool OrbitalTaskManagerImpl::shouldExecuteConditionalTask(const TriggerCondition& condition) const {
    // Check time-based trigger
    if (condition.time_point.has_value()) {
        auto now = std::chrono::system_clock::now();
        if (now >= condition.time_point.value()) {
            return true;
        }
    }
    
    // Check orbital position trigger
    if (condition.orbit_position.has_value()) {
        OrbitPosition current_pos;
        {
            std::lock_guard<std::mutex> lock(position_mutex_);
            current_pos = current_position_;
        }
        
        if (matchesOrbitPosition(current_pos, condition.orbit_position.value())) {
            return true;
        }
    }
    
    // Check event-based trigger
    if (condition.event_name.has_value()) {
        // In a real implementation, this would check an event registry
        // For now, we'll just log that we would check it
        log_debug("Checking for event trigger: " + condition.event_name.value());
        // Always return false since we don't have an event system
        return false;
    }
    
    // Check dependency task trigger
    if (condition.dependency_task_id.has_value()) {
        std::lock_guard<std::mutex> map_lock(tasks_mutex_);
        
        auto it = task_map_.find(condition.dependency_task_id.value());
        if (it != task_map_.end() && it->second->status == TaskStatus::COMPLETED) {
            return true;
        }
    }
    
    // No triggers were satisfied
    return false;
}

bool OrbitalTaskManagerImpl::matchesOrbitPosition(
    const OrbitPosition& current, const OrbitPosition& trigger) const {
    
    // Parameters for matching
    constexpr double ALTITUDE_TOLERANCE_KM = 10.0;  // km
    constexpr double POSITION_TOLERANCE_DEG = 5.0;  // degrees
    
    // Check altitude
    bool altitude_match = std::abs(current.altitude_km - trigger.altitude_km) <= ALTITUDE_TOLERANCE_KM;
    
    // Check latitude and longitude
    bool latitude_match = std::abs(current.latitude - trigger.latitude) <= POSITION_TOLERANCE_DEG;
    bool longitude_match = std::abs(current.longitude - trigger.longitude) <= POSITION_TOLERANCE_DEG;
    
    // Special case for longitude wrap-around at 180/-180
    if (!longitude_match) {
        double alt_diff = std::abs((current.longitude + 360.0) - trigger.longitude);
        alt_diff = std::min(alt_diff, std::abs(current.longitude - (trigger.longitude + 360.0)));
        longitude_match = alt_diff <= POSITION_TOLERANCE_DEG;
    }
    
    return altitude_match && latitude_match && longitude_match;
}

void OrbitalTaskManagerImpl::notifyTaskCompletion(const TaskResult& result) {
    std::lock_guard<std::mutex> callback_lock(callback_mutex_);
    
    // Get the task type from the task map
    TaskType task_type = TaskType::COMMUNICATION; // Default
    {
        std::lock_guard<std::mutex> map_lock(tasks_mutex_);
        auto it = task_map_.find(result.task_id);
        if (it != task_map_.end()) {
            task_type = it->second->task.type;
        }
    }
    
    // Call all registered callbacks that match the task type
    for (const auto& entry : callbacks_) {
        if (entry.filter_type == task_type) {
            try {
                entry.callback(result);
            } catch (const std::exception& e) {
                log_error("Exception in task completion callback (ID: " + 
                         std::to_string(entry.id) + "): " + e.what());
            } catch (...) {
                log_error("Unknown exception in task completion callback (ID: " + 
                         std::to_string(entry.id) + ")");
            }
        }
    }
}

TaskResult OrbitalTaskManagerImpl::createTaskResult(
    const std::shared_ptr<TaskEntry>& task_entry) const {
    
    TaskResult result;
    result.task_id = task_entry->task.task_id;
    result.status = task_entry->status;
    result.start_time = task_entry->actual_start_time;
    result.end_time = task_entry->actual_end_time;
    result.error_message = task_entry->error_message;
    result.output_data = task_entry->result_data;
    result.retry_attempts = task_entry->actual_retry_count;
    result.radiation_event_detected = task_entry->radiation_event_detected;
    
    return result;
}

// Factory function implementation
std::unique_ptr<OrbitalTaskManager> createOrbitalTaskManager(const std::string& config_path) {
    auto manager = std::make_unique<OrbitalTaskManagerImpl>();
    
    if (!manager->initialize(config_path)) {
        return nullptr;
    }
    
    return manager;
}

} // namespace core
} // namespace skymesh

// Helper function implementations
namespace {

void log_debug(const std::string& message) {
    // In a real implementation, this would use a proper logging system
    std::cout << "[DEBUG] " << message << std::endl;
}

void log_info(const std::string& message) {
    // In a real implementation, this would use a proper logging system
    std::cout << "[INFO] " << message << std::endl;
}

void log_warning(const std::string& message) {
    // In a real implementation, this would use a proper logging system
    std::cout << "[WARNING] " << message << std::endl;
}

void log_error(const std::string& message) {
    // In a real implementation, this would use a proper logging system
    std::cerr << "[ERROR] " << message << std::endl;
}

std::string generate_task_id() {
    // Generate a random ID with timestamp
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto epoch = now_ms.time_since_epoch();
    uint64_t timestamp = static_cast<uint64_t>(epoch.count());
    
    // Add some randomness
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dist(0, UINT32_MAX);
    uint32_t random = dist(gen);
    
    // Format as hexadecimal string
    std::stringstream ss;
    ss << std::hex << std::setfill('0')
       << std::setw(16) << timestamp
       << std::setw(8) << random;
    
    return ss.str();
}

std::string timestamp_to_string(const std::chrono::system_clock::time_point& time) {
    auto time_t = std::chrono::system_clock::to_time_t(time);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        time.time_since_epoch() % std::chrono::seconds(1));
    
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S")
       << '.' << std::setfill('0') << std::setw(3) << ms.count()
       << "Z";
    
    return ss.str();
}

} // anonymous namespace
