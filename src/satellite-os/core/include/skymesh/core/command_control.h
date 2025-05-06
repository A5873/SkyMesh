/**
 * @file command_control.h
 * @brief Command and Control System for SkyMesh satellites
 *
 * Defines the interface for satellite command processing and control,
 * coordinating between RF, power, and orbital task systems.
 * 
 * This system implements radiation-tolerant design practices including:
 * - Triple Modular Redundancy (TMR) for critical operations
 * - Command validation and authentication
 * - Safe mode fallback mechanisms
 * - Robust error detection and correction
 */

#ifndef SKYMESH_CORE_COMMAND_CONTROL_H
#define SKYMESH_CORE_COMMAND_CONTROL_H

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <atomic>
#include <mutex>

// Include dependencies for subsystem coordination
#include "skymesh/core/rf_controller.h"
#include "skymesh/core/power_manager.h"
#include "skymesh/core/orbital_task_manager.h"
#include "skymesh/core/health_monitor.h"

namespace skymesh {
namespace core {

/**
 * @brief Command priority levels
 * 
 * Defines priority levels for command execution with safety considerations
 */
enum class CommandPriority {
    EMERGENCY = 0,   ///< Critical safety commands (highest priority)
    HIGH = 1,        ///< Time-sensitive operations
    NORMAL = 2,      ///< Standard operations
    LOW = 3,         ///< Background/maintenance operations
    DEFERRED = 4     ///< Non-critical operations to be executed when resources available
};

/**
 * @brief Command status codes
 */
enum class CommandStatus {
    SUCCESS = 0,             ///< Command executed successfully
    PENDING = 1,             ///< Command queued for execution
    INVALID_COMMAND = 2,     ///< Command format or checksum invalid
    UNAUTHORIZED = 3,        ///< Command authentication failed
    EXECUTION_ERROR = 4,     ///< Error during command execution
    RESOURCE_UNAVAILABLE = 5,///< Required resources not available
    TIMEOUT = 6,             ///< Command execution timed out
    REDUNDANCY_MISMATCH = 7, ///< TMR validation failed
    RADIATION_ERROR = 8      ///< Radiation-induced error detected
};

/**
 * @brief Command source identification
 */
enum class CommandSource {
    GROUND_STATION = 0,      ///< Command from Earth ground station
    MESH_PEER = 1,           ///< Command from another satellite in mesh
    ONBOARD_SCHEDULER = 2,   ///< Internally scheduled command
    AUTONOMOUS_SYSTEM = 3,   ///< Command from onboard autonomous system
    RECOVERY_SYSTEM = 4      ///< Command from system recovery mechanisms
};

/**
 * @brief Structure representing a satellite command
 * 
 * Includes authentication, redundancy, and checksums for radiation tolerance
 */
struct Command {
    uint32_t commandId;          ///< Unique command identifier
    uint16_t commandCode;        ///< Command operation code
    CommandPriority priority;    ///< Command priority level
    CommandSource source;        ///< Command originator
    uint64_t timestamp;          ///< Command creation timestamp
    std::vector<uint8_t> data;   ///< Command payload data
    uint32_t checksum;           ///< Command data checksum
    std::vector<uint8_t> signature; ///< Command authentication signature
    
    // Redundant copies for TMR (not stored in transmission)
    mutable uint16_t commandCode_copy1;  ///< Redundant copy 1 for TMR
    mutable uint16_t commandCode_copy2;  ///< Redundant copy 2 for TMR
    
    // Validation methods
    bool validateChecksum() const;
    bool validateSignature() const;
    bool validateTMR() const;
    
    // TMR voting mechanism
    uint16_t getCommandCodeTMR() const;
};

/**
 * @brief Telemetry data structure
 * 
 * Contains satellite status information for downlink transmission
 */
struct TelemetryPacket {
    uint32_t packetId;           ///< Unique telemetry packet identifier
    uint64_t timestamp;          ///< Packet creation timestamp
    uint16_t packetType;         ///< Type of telemetry data
    std::vector<uint8_t> data;   ///< Telemetry payload data
    uint32_t checksum;           ///< Packet checksum for validation
    
    // Methods
    void generateChecksum();
    bool validateChecksum() const;
    
    // Error correction code
    std::vector<uint8_t> ecc;    ///< Error correction code
    void generateECC();
    bool applyECCCorrection();
};

/**
 * @brief Command execution callback type
 */
using CommandCallback = std::function<void(CommandStatus, const std::string&)>;

/**
 * @class CommandControl
 * @brief Core command and control system for satellite operations
 * 
 * Manages command processing, telemetry collection, and subsystem coordination
 * with radiation-tolerant design principles.
 */
class CommandControl {
public:
    /**
     * @brief Constructor
     * 
     * @param rfController Reference to the RF controller subsystem
     * @param powerManager Reference to the power management subsystem
     * @param orbitalTaskManager Reference to the orbital task manager
     * @param healthMonitor Reference to the health monitoring system
     */
    CommandControl(
        std::shared_ptr<RFController> rfController,
        std::shared_ptr<PowerManager> powerManager,
        std::shared_ptr<OrbitalTaskManager> orbitalTaskManager,
        std::shared_ptr<HealthMonitor> healthMonitor
    );
    
    /**
     * @brief Destructor
     */
    ~CommandControl();
    
    /**
     * @brief Initialize the command and control system
     * 
     * @return True if initialization successful, false otherwise
     */
    bool initialize();
    
    // ---- Command Processing ----
    
    /**
     * @brief Process a received command
     * 
     * Validates, authenticates, and schedules command execution with
     * radiation-tolerant verification.
     * 
     * @param command The command to process
     * @param callback Optional callback for command completion notification
     * @return Status of command processing
     */
    CommandStatus processCommand(const Command& command, CommandCallback callback = nullptr);
    
    /**
     * @brief Queue a command for deferred execution
     * 
     * @param command The command to queue
     * @param callback Optional callback for command completion notification
     * @return True if successfully queued, false otherwise
     */
    bool queueCommand(const Command& command, CommandCallback callback = nullptr);
    
    /**
     * @brief Create a new command for internal execution
     * 
     * @param commandCode The command operation code
     * @param priority Command priority
     * @param data Command payload data
     * @return Newly created command
     */
    Command createCommand(uint16_t commandCode, CommandPriority priority, 
                         const std::vector<uint8_t>& data);
    
    // ---- Telemetry Management ----
    
    /**
     * @brief Collect telemetry from all subsystems
     * 
     * @param fullTelemetry If true, collect comprehensive telemetry
     * @return Vector of telemetry packets
     */
    std::vector<TelemetryPacket> collectTelemetry(bool fullTelemetry = false);
    
    /**
     * @brief Queue telemetry for transmission
     * 
     * @param packet Telemetry packet to transmit
     * @return True if successfully queued, false otherwise
     */
    bool queueTelemetry(const TelemetryPacket& packet);
    
    /**
     * @brief Process received telemetry request
     * 
     * @param requestType Type of telemetry requested
     * @return True if request processed successfully
     */
    bool processTelemetryRequest(uint16_t requestType);
    
    // ---- System Coordination ----
    
    /**
     * @brief Execute a system mode change
     * 
     * Coordinates mode changes across all subsystems with safety checks
     * 
     * @param newMode The target system mode
     * @return True if mode change successful
     */
    bool changeSystemMode(SystemMode newMode);
    
    /**
     * @brief Coordinate a scheduled orbital operation
     * 
     * @param operationType The type of orbital operation
     * @param parameters Operation parameters
     * @return True if operation scheduled successfully
     */
    bool scheduleOrbitalOperation(uint16_t operationType,
                                 const std::vector<uint8_t>& parameters);
    
    /**
     * @brief Register a callback for a specific event type
     * 
     * @param eventType Type of event to monitor
     * @param callback Function to call when event occurs
     * @return True if registration successful
     */
    bool registerEventCallback(uint16_t eventType,
                              std::function<void(const std::vector<uint8_t>&)> callback);
    
    // ---- Error Handling ----
    
    /**
     * @brief Enter safe mode due to critical error
     * 
     * @param errorCode The error code that triggered safe mode
     * @param errorDetails Optional error details
     */
    void enterSafeMode(uint32_t errorCode, const std::string& errorDetails = "");
    
    /**
     * @brief Attempt recovery from error condition
     * 
     * @param recoveryLevel Depth of recovery attempt
     * @return True if recovery successful
     */
    bool attemptRecovery(uint8_t recoveryLevel);
    
    /**
     * @brief Log error with radiation-tolerant storage
     * 
     * @param severity Error severity level
     * @param component Affected component
     * @param message Error message
     * @param data Additional error data
     */
    void logError(uint8_t severity, uint16_t component,
                 const std::string& message, const std::vector<uint8_t>& data = {});
    
    /**
     * @brief Check if system is in secure state
     * 
     * @return True if system is in a secure operational state
     */
    bool isSystemSecure() const;
    
private:
    // Subsystem references
    std::shared_ptr<RFController> m_rfController;
    std::shared_ptr<PowerManager> m_powerManager;
    std::shared_ptr<OrbitalTaskManager> m_orbitalTaskManager;
    std::shared_ptr<HealthMonitor> m_healthMonitor;
    
    // Command processing queues
    std::vector<std::pair<Command, CommandCallback>> m_highPriorityQueue;
    std::vector<std::pair<Command, CommandCallback>> m_normalPriorityQueue;
    std::vector<std::pair<Command, CommandCallback>> m_lowPriorityQueue;
    
    // Synchronization and protection
    std::mutex m_commandQueueMutex;
    std::mutex m_telemetryQueueMutex;
    std::atomic<bool> m_isProcessingCommands;
    
    // Internal state
    std::atomic<SystemMode> m_currentMode;
    std::atomic<bool> m_inSafeMode;
    std::atomic<uint32_t> m_lastErrorCode;
    
    // Private implementation methods
    bool authenticateCommand(const Command& command);
    bool validateCommandParameters(const Command& command);
    void executeCommand(const Command& command, CommandCallback callback);
    void processCommandQueues();
    
    // Radiation mitigation methods
    bool performTripleCommandValidation(const Command& command);
    void scrubCommandQueue();
    void performStateValidation();
    
    // Error recovery methods
    void recordSubsystemState();
    bool restoreLastKnownGoodState();
    void notifyGroundOfStateChange(uint16_t stateChangeType);
};

} // namespace core
} // namespace skymesh

#endif // SKYMESH_CORE_COMMAND_CONTROL_H

