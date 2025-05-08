/**
 * @file test_radiation_hardening.cpp
 * @brief Unit tests for radiation hardening mechanisms used in SkyMesh
 * 
 * These tests validate the radiation hardening mechanisms including:
 * - Triple Modular Redundancy (TMR)
 * - Memory Scrubbing
 * - Error Detection & Correction
 * - System Resilience to SEUs (Single Event Upsets)
 */

#include "test_utils.h"
#include <gtest/gtest.h>
#include <array>
#include <algorithm>
#include <cstring>
#include <memory>
#include <bitset>
#include <random>
#include <chrono>

#include "skymesh/core/power_manager.h"
#include "skymesh/core/command_control.h"

namespace skymesh {
namespace core {

/**
 * Custom fixture for radiation hardening tests
 */
class RadiationHardeningTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test objects
        powerManager = std::make_shared<PowerManager>();
        
        // Initialize subsystems for power manager
        std::vector<SubsystemID> subsystems = {
            SubsystemID::RF_SYSTEM,
            SubsystemID::OBC,
            SubsystemID::ADCS,
            SubsystemID::THERMAL,
            SubsystemID::PAYLOAD,
            SubsystemID::SENSORS
        };
        
        powerManager->initialize(subsystems);
    }

    void TearDown() override {
        // Clean up
    }
    
    // Test helpers
    
    /**
     * @brief Simulate a radiation-induced bit flip in memory
     * 
     * @param data Pointer to the data to corrupt
     * @param sizeInBytes Size of the data in bytes
     * @param numBitsToFlip Number of bits to flip
     */
    template<typename T>
    void simulateRadiationBitFlip(T* data, size_t numBitsToFlip = 1) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distr(0, sizeof(T) * 8 - 1);
        
        // Create a copy of original data for verification
        T originalValue = *data;
        
        // Flip specified number of random bits
        for (size_t i = 0; i < numBitsToFlip; ++i) {
            int bitPosition = distr(gen);
            *data ^= (1ULL << bitPosition);
        }
        
        // Log the corruption for debugging
        std::cout << "Simulated radiation bit flip: " 
                  << "Original value: " << originalValue 
                  << ", Corrupted value: " << *data << std::endl;
    }
    
    /**
     * @brief Simulate radiation hit on an object by corrupting its memory
     * 
     * @param pm Power manager instance
     * @param memberName Name of the member to target
     * @param offset Offset within the member (in bytes)
     * @param numBitsToFlip Number of bits to flip
     */
    void simulateRadiationHit(std::shared_ptr<PowerManager> pm, const char* memberName, size_t offset, size_t numBitsToFlip = 1) {
        void* ptr = PowerManager::RadiationTestInterface::getInternalStatePtr(pm.get(), memberName);
        if (ptr) {
            uint8_t* bytes = reinterpret_cast<uint8_t*>(ptr);
            
            // Generate random bits to flip
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> distr(0, 7); // Bit position in byte
            
            // Flip bits
            for (size_t i = 0; i < numBitsToFlip; ++i) {
                int bitPosition = distr(gen);
                bytes[offset] ^= (1 << bitPosition);
            }
        }
    }


    std::shared_ptr<PowerManager> powerManager;
};

/**
 * TMR Tests
 */
TEST_F(RadiationHardeningTest, TestTMRWithBooleanValues) {
    std::array<bool, 3> values = {true, true, false};
    
    // Use the TMR implementation from PowerManager
    // Since applyTMR is protected in PowerManager, we test directly through
    // PowerManager's interface methods that use TMR internally
    
    // Enable a subsystem - this uses TMR internally
    powerManager->enableSubsystem(SubsystemID::RF_SYSTEM, 0.5f);
    
    // Verify it's enabled
    EXPECT_TRUE(powerManager->isSubsystemEnabled(SubsystemID::RF_SYSTEM));
    
    // Simulate radiation hit by directly corrupting one of the redundant states
    // This is a white-box test that knows about the implementation details
    simulateRadiationHit(powerManager, "subsystemStates", 10, 1);
    
    // The system should still report correct state due to TMR
    EXPECT_TRUE(powerManager->isSubsystemEnabled(SubsystemID::RF_SYSTEM));
    
    // Verify that TMR can handle two corrupted values
    powerManager->disableSubsystem(SubsystemID::RF_SYSTEM);
    EXPECT_FALSE(powerManager->isSubsystemEnabled(SubsystemID::RF_SYSTEM));
    
    // Simulate radiation affecting two redundant copies
    simulateRadiationHit(powerManager, "subsystemStates", 10, 1);
    simulateRadiationHit(powerManager, "subsystemStates", 15, 1);
    
    // Scrubbing should fix the corrupted values
    powerManager->handleRadiationErrors();
    
    // Verify the system still reports the correct state
    EXPECT_FALSE(powerManager->isSubsystemEnabled(SubsystemID::RF_SYSTEM));
}

TEST_F(RadiationHardeningTest, TestTMRWithEnumValues) {
    // Set power mode and verify
    powerManager->setPowerMode(PowerMode::NORMAL);
    EXPECT_EQ(PowerMode::NORMAL, powerManager->getCurrentPowerMode());
    
    // Corrupt the power mode (simulate radiation hit)
    // This is a white-box test assuming knowledge of internal structure
    simulateRadiationHit(powerManager, "currentMode", 0, 1);
    
    // The corrupted value should be corrected by TMR
    powerManager->handleRadiationErrors();
    EXPECT_EQ(PowerMode::NORMAL, powerManager->getCurrentPowerMode());
    
    // Try a different power mode
    powerManager->setPowerMode(PowerMode::LOW_POWER);
    EXPECT_EQ(PowerMode::LOW_POWER, powerManager->getCurrentPowerMode());
    
    // Corrupt power mode again (more severely this time)
    simulateRadiationHit(powerManager, "currentMode", 0, 2);
    
    // Test radiation error handling - should correct the mode
    powerManager->handleRadiationErrors();
    EXPECT_EQ(PowerMode::LOW_POWER, powerManager->getCurrentPowerMode());
}

TEST_F(RadiationHardeningTest, TestTMRWithFloatingPointValues) {
    // Subsystem power level uses floating point with TMR
    powerManager->enableSubsystem(SubsystemID::PAYLOAD, 0.75f);
    
    // Setup a callback to monitor power changes
    bool callbackCalled = false;
    auto callback = [&callbackCalled](PowerMode mode) {
        callbackCalled = true;
    };
    uint32_t callbackId = powerManager->registerPowerWarningCallback(callback);
    
    // Simulate radiation affecting subsystem power level
    // This targets the internal subsystemPowerLevels map
    simulateRadiationHit(powerManager, "subsystemPowerLevels", 20, 1);
    
    // Run power update which should detect and fix errors
    powerManager->update(1000); // 1 second update
    
    // Verify that power level is maintained correctly despite corruption
    // We can't directly test the float value, but we can test system stability
    EXPECT_TRUE(powerManager->isSubsystemEnabled(SubsystemID::PAYLOAD));
    
    // Cleanup
    powerManager->unregisterPowerWarningCallback(callbackId);
}

/**
 * Memory Scrubbing Tests
 */
TEST_F(RadiationHardeningTest, TestMemoryScrubbing) {
    // Initialize with multiple subsystems
    powerManager->enableSubsystem(SubsystemID::RF_SYSTEM, 0.8f);
    powerManager->enableSubsystem(SubsystemID::OBC, 0.9f);
    powerManager->enableSubsystem(SubsystemID::ADCS, 0.7f);
    
    // Verify initial state
    EXPECT_TRUE(powerManager->isSubsystemEnabled(SubsystemID::RF_SYSTEM));
    EXPECT_TRUE(powerManager->isSubsystemEnabled(SubsystemID::OBC));
    EXPECT_TRUE(powerManager->isSubsystemEnabled(SubsystemID::ADCS));
    
    // Simulate multiple radiation hits
    simulateRadiationHit(powerManager, "subsystemStates", 5, 1);
    simulateRadiationHit(powerManager, "subsystemPowerLevels", 10, 1);
    simulateRadiationHit(powerManager, "currentMode", 0, 1);
    
    // Apply scrubbing (this is public in PowerManager)
    powerManager->handleRadiationErrors();
    
    // Verify the system state is preserved despite corruption
    EXPECT_TRUE(powerManager->isSubsystemEnabled(SubsystemID::RF_SYSTEM));
    EXPECT_TRUE(powerManager->isSubsystemEnabled(SubsystemID::OBC));
    EXPECT_TRUE(powerManager->isSubsystemEnabled(SubsystemID::ADCS));
    
    // Test more severe corruption
    // Simulate a burst of radiation affecting multiple memory locations
    for (int i = 0; i < 5; i++) {
        simulateRadiationHit(powerManager, "subsystemStates", i*5, 2);
    }
    
    // Run health check which includes scrubbing
    bool healthStatus = powerManager->performHealthCheck();
    
    // System may report unhealthy status but should remain functional
    EXPECT_TRUE(powerManager->isSubsystemEnabled(SubsystemID::RF_SYSTEM));
}

/**
 * Error Detection Tests 
 */
TEST_F(RadiationHardeningTest, TestErrorDetection) {
    // Set up system state
    powerManager->setPowerMode(PowerMode::NORMAL);
    powerManager->enableSubsystem(SubsystemID::RF_SYSTEM, 0.8f);
    
    // Corrupt RF system state - we'll simulate this by trying to 
    // directly manipulate the subsystem state map with bit flips
    simulateRadiationHit(powerManager, "subsystemStates", 8, 1);
    
    // Run radiation error handler which should detect the inconsistency
    bool errorsDetected = powerManager->handleRadiationErrors();
    
    // Errors should be detected and corrected
    EXPECT_TRUE(errorsDetected);
    
    // System should still report the correct state
    EXPECT_TRUE(powerManager->isSubsystemEnabled(SubsystemID::RF_SYSTEM));
}

/**
 * System Resilience Tests
 */
TEST_F(RadiationHardeningTest, TestSystemResilience) {
    // Configure power manager for an orbital operation
    powerManager->setPowerMode(PowerMode::NORMAL);
    powerManager->enableSubsystem(SubsystemID::RF_SYSTEM, 0.9f);
    powerManager->enableSubsystem(SubsystemID::ADCS, 0.8f);
    powerManager->enableSubsystem(SubsystemID::PAYLOAD, 0.7f);
    
    // Simulate passing through a radiation belt - multiple bit flips
    const char* members[] = {"currentMode", "subsystemStates", "subsystemPowerLevels"};
    for (int i = 0; i < 10; i++) {
        // Randomly select which member to corrupt
        size_t memberIndex = rand() % 3;
        size_t offset = rand() % 20;  // Random offset within the member
        simulateRadiationHit(powerManager, members[memberIndex], offset, 1);
    }
    
    // Run system update multiple times to simulate continuous operation
    for (int i = 0; i < 5; i++) {
        powerManager->update(1000); // 1 second updates
    }
    
    // The system should maintain correct operation despite corruption
    EXPECT_TRUE(powerManager->isSubsystemEnabled(SubsystemID::RF_SYSTEM));
    EXPECT_TRUE(powerManager->isSubsystemEnabled(SubsystemID::ADCS));
    EXPECT_TRUE(powerManager->isSubsystemEnabled(SubsystemID::PAYLOAD));
    
    // Try to prepare for RF burst which uses complex checks
    bool rfBurstReady = powerManager->prepareForRFBurst(5000, 0.95f);
    
    // Test power budget calculation which uses TMR internally
    PowerBudget budget = powerManager->getPowerBudget();
    EXPECT_GT(budget.totalAvailable, 0.0f);
    
    // Test system reset which should clear all radiation errors
    powerManager->reset(true);
    
    // After reset, system should be in a clean state
    EXPECT_EQ(PowerMode::NORMAL, powerManager->getCurrentPowerMode());
}

/**
 * Integration Test 
 */
TEST_F(RadiationHardeningTest, TestIntegrationWithMultipleSubsystems) {
    // This test would simulate radiation affecting multiple subsystems
    // and test their combined radiation hardening capabilities
    
    // Enable systems
    powerManager->setPowerMode(PowerMode::NORMAL);
    powerManager->enableSubsystem(SubsystemID::RF_SYSTEM, 0.8f);
    powerManager->enableSubsystem(SubsystemID::OBC, 0.9f);
    powerManager->enableSubsystem(SubsystemID::ADCS, 0.7f);
    
    // Simulate radiation affecting various parts of the system
    simulateRadiationHit(powerManager, "currentMode", 0, 2);
    simulateRadiationHit(powerManager, "subsystemStates", 15, 1);
    simulateRadiationHit(powerManager, "subsystemPowerLevels", 25, 2);
    
    // Update orbit power profile during radiation exposure
    powerManager->updateOrbitPowerProfile(5400, 3600); // 90 min orbit, 60 min in eclipse
    
    // Verify all systems are still functional after radiation hits
    EXPECT_TRUE(powerManager->isSubsystemEnabled(SubsystemID::RF_SYSTEM));
    EXPECT_TRUE(powerManager->isSubsystemEnabled(SubsystemID::OBC));
    EXPECT_TRUE(powerManager->isSubsystemEnabled(SubsystemID::ADCS));
}

} // namespace core
} // namespace skymesh
