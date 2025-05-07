/**
 * @file orbital_task_manager_test.cpp
 * @brief Unit tests for the OrbitalTaskManager implementation
 */

#include "skymesh/core/orbital_task_manager.h"

#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>

using namespace skymesh::core;
using namespace std::chrono_literals;

// Test fixtures
class OrbitalTaskManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = createOrbitalTaskManager();
        ASSERT_TRUE(manager != nullptr);
        ASSERT_TRUE(manager->start());
    }

    void TearDown() override {
        if (manager) {
            manager->stop();
        }
    }

    // Helper functions
    OrbitalTask createBasicTask(const std::string& name, TaskPriority priority = TaskPriority::NORMAL) {
        OrbitalTask task;
        task.name = name;
        task.type = TaskType::MAINTENANCE;
        task.priority = priority;
        task.scheduled_time = std::chrono::system_clock::now();
        task.timeout = std::chrono::milliseconds(5000);
        task.recovery_strategy = RecoveryStrategy::RETRY;
        task.radiation_protected = false;
        task.retry_count = 1;

        // Set the task function to a simple function that returns true
        task.task_function = [](const TaskContext&) -> bool {
            return true;
        };

        return task;
    }

    // Wait for task to complete with timeout
    bool waitForTaskCompletion(const std::string& task_id, std::chrono::milliseconds timeout = 5s) {
        auto start = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - start < timeout) {
            auto status = manager->getTaskStatus(task_id);
            if (status == TaskStatus::COMPLETED || status == TaskStatus::FAILED) {
                return true;
            }
            std::this_thread::sleep_for(50ms);
        }
        return false;
    }

    std::unique_ptr<OrbitalTaskManager> manager;
};

// Test basic task scheduling and execution
TEST_F(OrbitalTaskManagerTest, ScheduleAndExecuteTask) {
    // Create a task
    auto task = createBasicTask("TestTask");
    
    // Schedule the task
    std::string task_id = manager->scheduleTask(task);
    ASSERT_FALSE(task_id.empty());
    
    // Wait for task to complete
    ASSERT_TRUE(waitForTaskCompletion(task_id));
    
    // Verify task status
    ASSERT_EQ(manager->getTaskStatus(task_id), TaskStatus::COMPLETED);
    
    // Verify task result
    auto result = manager->getTaskResult(task_id);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->status, TaskStatus::COMPLETED);
}

// Test task priority handling
TEST_F(OrbitalTaskManagerTest, TaskPriorityHandling) {
    // Create a mutex and condition variable to control task execution
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> task1_started{false};
    std::atomic<bool> task2_started{false};
    std::atomic<bool> task3_started{false};
    std::string started_order;
    
    // Create three tasks with different priorities
    OrbitalTask task1 = createBasicTask("LowPriorityTask", TaskPriority::LOW);
    OrbitalTask task2 = createBasicTask("NormalPriorityTask", TaskPriority::NORMAL);
    OrbitalTask task3 = createBasicTask("HighPriorityTask", TaskPriority::HIGH);
    
    // Set task functions that will record their execution order
    task1.task_function = [&](const TaskContext&) -> bool {
        task1_started = true;
        started_order += "1";
        return true;
    };
    
    task2.task_function = [&](const TaskContext&) -> bool {
        task2_started = true;
        started_order += "2";
        return true;
    };
    
    task3.task_function = [&](const TaskContext&) -> bool {
        task3_started = true;
        started_order += "3";
        return true;
    };
    
    // Schedule tasks in reverse order of priority
    std::string task1_id = manager->scheduleTask(task1);
    std::string task2_id = manager->scheduleTask(task2);
    std::string task3_id = manager->scheduleTask(task3);
    
    // Wait for all tasks to complete
    ASSERT_TRUE(waitForTaskCompletion(task1_id));
    ASSERT_TRUE(waitForTaskCompletion(task2_id));
    ASSERT_TRUE(waitForTaskCompletion(task3_id));
    
    // Verify execution order (highest priority should be executed first)
    ASSERT_EQ(started_order, "321");
}

// Test TMR functionality
TEST_F(OrbitalTaskManagerTest, TripleModularRedundancy) {
    // Create a counter to track how many times the task function is called
    std::atomic<int> execution_count{0};
    
    // Create a task with radiation protection enabled
    OrbitalTask task = createBasicTask("RadiationProtectedTask");
    task.radiation_protected = true;
    
    // Set task function that will increment the counter
    task.task_function = [&](const TaskContext&) -> bool {
        execution_count++;
        return true;
    };
    
    // Schedule the task
    std::string task_id = manager->scheduleTask(task);
    
    // Wait for task to complete
    ASSERT_TRUE(waitForTaskCompletion(task_id));
    
    // Verify task was executed three times (TMR)
    ASSERT_EQ(execution_count, 3);
    
    // Verify task status
    ASSERT_EQ(manager->getTaskStatus(task_id), TaskStatus::COMPLETED);
}

// Test radiation recovery strategies
TEST_F(OrbitalTaskManagerTest, RadiationRecoveryStrategies) {
    // Create a task that will fail
    OrbitalTask task = createBasicTask("FailingTask");
    task.task_function = [](const TaskContext&) -> bool {
        return false;
    };
    
    // Schedule the task
    std::string task_id = manager->scheduleTask(task);
    
    // Wait for task to fail
    ASSERT_TRUE(waitForTaskCompletion(task_id));
    ASSERT_EQ(manager->getTaskStatus(task_id), TaskStatus::FAILED);
    
    // Test recovery with retry strategy
    ASSERT_TRUE(manager->recoverTask(task_id, RecoveryStrategy::RETRY));
    
    // Wait for task to be processed again
    std::this_thread::sleep_for(100ms);
    
    // Task should be pending or failed again
    auto status = manager->getTaskStatus(task_id);
    ASSERT_TRUE(status == TaskStatus::PENDING || status == TaskStatus::FAILED);
    
    // Test recovery with checkpoint restore strategy
    if (status == TaskStatus::FAILED) {
        ASSERT_TRUE(manager->recoverTask(task_id, RecoveryStrategy::CHECKPOINT_RESTORE));
        
        // Verify task metadata
        auto tasks = manager->getAllScheduledTasks();
        for (const auto& t : tasks) {
            if (t.task_id == task_id) {
                ASSERT_EQ(t.metadata.at("recovery_type"), "checkpoint");
                break;
            }
        }
    }
    
    // Wait for task to complete/fail
    ASSERT_TRUE(waitForTaskCompletion(task_id));
}

// Test orbital position triggers
TEST_F(OrbitalTaskManagerTest, OrbitalPositionTriggers) {
    std::atomic<bool> task_executed{false};
    
    // Create a task that will be triggered by orbital position
    OrbitalTask task = createBasicTask("PositionTriggeredTask");
    task.task_function = [&](const TaskContext&) -> bool {
        task_executed = true;
        return true;
    };
    
    // Create orbital position trigger at specific location
    TriggerCondition trigger;
    OrbitPosition position;
    position.altitude_km = 550.0;
    position.latitude = 45.0;
    position.longitude = 90.0;
    position.velocity_kmps = 7.6;
    position.timestamp = std::chrono::system_clock::now();
    trigger.orbit_position = position;
    
    // Schedule the conditional task
    std::string task_id = manager->scheduleConditionalTask(task, trigger);
    ASSERT_FALSE(task_id.empty());
    
    // Task should not execute yet
    std::this_thread::sleep_for(200ms);
    ASSERT_FALSE(task_executed);
    
    // Update orbital position to match the trigger
    manager->updateOrbitalPosition(position);
    
    // Wait for task to execute
    auto start = std::chrono::steady_clock::now();
    while (!task_executed && std::chrono::steady_clock::now() - start < 3s) {
        std::this_thread::sleep_for(100ms);
    }
    
    // Verify task executed
    ASSERT_TRUE(task_executed);
    
    // Wait for task to complete
    ASSERT_TRUE(waitForTaskCompletion(task_id));
    ASSERT_EQ(manager->getTaskStatus(task_id), TaskStatus::COMPLETED);
}

// Test task completion callbacks
TEST_F(OrbitalTaskManagerTest, TaskCompletionCallbacks) {
    std::atomic<bool> callback_called{false};
    TaskResult callback_result;
    
    // Register a callback for maintenance tasks
    int callback_id = manager->registerCompletionCallback(
        [&](const TaskResult& result) {
            callback_called = true;
            callback_result = result;
        }, 
        TaskType::MAINTENANCE
    );
    
    // Create and schedule a task
    OrbitalTask task = createBasicTask("CallbackTestTask");
    task.type = TaskType::MAINTENANCE;
    std::string task_id = manager->scheduleTask(task);
    
    // Wait for task to complete
    ASSERT_TRUE(waitForTaskCompletion(task_id));
    
    // Wait for callback to be called
    auto start = std::chrono::steady_clock::now();
    while (!callback_called && std::chrono::steady_clock::now() - start < 1s) {
        std::this_thread::sleep_for(50ms);
    }
    
    // Verify callback was called with correct result
    ASSERT_TRUE(callback_called);
    ASSERT_EQ(callback_result.task_id, task_id);
    ASSERT_EQ(callback_result.status, TaskStatus::COMPLETED);
    
    // Unregister callback
    manager->unregisterCompletionCallback(callback_id);
}

// Test error handling
TEST_F(OrbitalTaskManagerTest, ErrorHandling) {
    // Create a task that will throw an exception
    OrbitalTask task = createBasicTask("ExceptionThrowingTask");
    task.task_function = [](const TaskContext&) -> bool {
        throw std::runtime_error("Test exception");
    };
    
    // Schedule the task
    std::string task_id = manager->scheduleTask(task);
    
    // Wait for task to complete
    ASSERT_TRUE(waitForTaskCompletion(task_id));
    
    // Verify task failed
    ASSERT_EQ(manager->getTaskStatus(task_id), TaskStatus::FAILED);
    
    // Verify error message
    auto result = manager->getTaskResult(task_id);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->status, TaskStatus::FAILED);
    ASSERT_NE(result->error_message.find("Test exception"), std::string::npos);
}

// Test recurring tasks
TEST_F(OrbitalTaskManagerTest, RecurringTasks) {
    std::atomic<int> execution_count{0};
    
    // Create a recurring task
    OrbitalTask task = createBasicTask("RecurringTask");
    task.task_function = [&](const TaskContext&) -> bool {
        execution_count++;
        return true;
    };
    
    // Schedule the recurring task with a short interval
    std::string task_id = manager->scheduleRecurringTask(task, 100ms);
    
    // Wait for multiple executions
    std::this_thread::sleep_for(550ms);
    
    // Verify task executed multiple times
    ASSERT_GE(execution_count, 3);
    
    // Cancel the recurring task
    ASSERT_TRUE(manager->cancelTask(task_id));
}

// Main function
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

