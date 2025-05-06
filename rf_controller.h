/**
 * @file rf_controller.h
 * @brief RF Controller Interface for SkyMesh Satellite System
 *
 * This header file defines the interface for controlling the radio frequency (RF)
 * subsystem in the SkyMesh satellite platform. Provides functions for configuring,
 * transmitting, receiving RF signals with radiation-hardened design considerations.
 */

#ifndef RF_CONTROLLER_H
#define RF_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief RF operating frequency bands
 */
typedef enum {
    RF_BAND_UHF = 0,       /**< Ultra High Frequency (300 MHz to 3 GHz) */
    RF_BAND_S = 1,         /**< S-Band (2 to 4 GHz) */
    RF_BAND_X = 2,         /**< X-Band (8 to 12 GHz) */
    RF_BAND_KU = 3,        /**< Ku-Band (12 to 18 GHz) */
    RF_BAND_KA = 4,        /**< Ka-Band (26 to 40 GHz) */
} rf_band_t;

/**
 * @brief RF modulation schemes
 */
typedef enum {
    RF_MOD_BPSK = 0,       /**< Binary Phase Shift Keying */
    RF_MOD_QPSK = 1,       /**< Quadrature Phase Shift Keying */
    RF_MOD_8PSK = 2,       /**< 8-Phase Shift Keying */
    RF_MOD_16QAM = 3,      /**< 16-Quadrature Amplitude Modulation */
    RF_MOD_FSK = 4,        /**< Frequency Shift Keying */
    RF_MOD_GMSK = 5,       /**< Gaussian Minimum Shift Keying */
} rf_modulation_t;

/**
 * @brief RF error correction coding schemes
 */
typedef enum {
    RF_FEC_NONE = 0,       /**< No Forward Error Correction */
    RF_FEC_CONV_1_2 = 1,   /**< Convolutional coding, rate 1/2 */
    RF_FEC_CONV_2_3 = 2,   /**< Convolutional coding, rate 2/3 */
    RF_FEC_RS = 3,         /**< Reed-Solomon */
    RF_FEC_LDPC = 4,       /**< Low-Density Parity-Check */
    RF_FEC_TURBO = 5,      /**< Turbo code */
} rf_fec_t;

/**
 * @brief RF controller status
 */
typedef enum {
    RF_STATUS_OK = 0,              /**< RF controller functioning normally */
    RF_STATUS_INIT_ERROR = 1,      /**< Initialization error */
    RF_STATUS_CONFIG_ERROR = 2,    /**< Configuration error */
    RF_STATUS_TX_ERROR = 3,        /**< Transmission error */
    RF_STATUS_RX_ERROR = 4,        /**< Reception error */
    RF_STATUS_CALIBRATION_ERROR = 5, /**< Calibration error */
    RF_STATUS_ANTENNA_ERROR = 6,   /**< Antenna control error */
    RF_STATUS_POWER_ERROR = 7,     /**< Power management error */
    RF_STATUS_RADIATION_ERROR = 8, /**< Radiation-induced error detected */
    RF_STATUS_UNKNOWN_ERROR = 9,   /**< Unknown error */
} rf_status_t;

/**
 * @brief RF transmit power levels
 */
typedef enum {
    RF_POWER_ULTRA_LOW = 0,  /**< Ultra low power (e.g., < 0.1W) */
    RF_POWER_LOW = 1,        /**< Low power (e.g., 0.1W - 0.5W) */
    RF_POWER_MEDIUM = 2,     /**< Medium power (e.g., 0.5W - 2W) */
    RF_POWER_HIGH = 3,       /**< High power (e.g., 2W - 5W) */
    RF_POWER_MAX = 4,        /**< Maximum power (e.g., > 5W) */
} rf_power_level_t;

/**
 * @brief RF controller configuration structure
 */
typedef struct {
    rf_band_t band;            /**< RF frequency band */
    uint32_t frequency_hz;     /**< Operating frequency in Hz */
    uint32_t bandwidth_hz;     /**< Channel bandwidth in Hz */
    rf_modulation_t modulation; /**< Modulation scheme */
    rf_fec_t fec;              /**< Forward Error Correction scheme */
    uint16_t preamble_length;  /**< Preamble length in bits */
    uint8_t sync_word[8];      /**< Synchronization word (up to 8 bytes) */
    uint8_t sync_word_size;    /**< Size of sync word in bytes (1-8) */
    rf_power_level_t power_level; /**< Transmit power level */
    bool auto_power_control;   /**< Automatic output power control */
    bool radiation_hardening;  /**< Enable radiation hardening features */
    uint8_t redundancy_level;  /**< Triple Modular Redundancy level (0-3) */
} rf_config_t;

/**
 * @brief RF signal metrics
 */
typedef struct {
    int16_t rssi_dbm;          /**< Received Signal Strength Indicator in dBm */
    int16_t snr_db;            /**< Signal-to-Noise Ratio in dB */
    uint32_t bit_errors;       /**< Number of bit errors detected */
    uint32_t packet_errors;    /**< Number of packet errors detected */
    uint32_t packets_received; /**< Number of packets successfully received */
    uint32_t packets_sent;     /**< Number of packets sent */
    uint32_t bytes_received;   /**< Number of bytes successfully received */
    uint32_t bytes_sent;       /**< Number of bytes sent */
} rf_metrics_t;

/**
 * @brief RF controller statistics and state
 */
typedef struct {
    rf_status_t status;        /**< Current status */
    rf_metrics_t metrics;      /**< Signal metrics */
    float temperature_c;       /**< RF module temperature in Celsius */
    float voltage_v;           /**< RF module supply voltage */
    uint32_t uptime_ms;        /**< RF controller uptime in milliseconds */
    uint32_t error_count;      /**< Total error count */
    uint32_t radiation_errors; /**< Radiation-induced error count */
    bool is_transmitting;      /**< Current transmission state */
    bool is_receiving;         /**< Current reception state */
    uint8_t current_antenna;   /**< Currently selected antenna */
} rf_state_t;

/**
 * @brief RF packet structure
 */
typedef struct {
    uint8_t* data;             /**< Packet data buffer */
    uint32_t length;           /**< Length of data in bytes */
    uint8_t dest_address[6];   /**< Destination address (6 bytes) */
    uint8_t src_address[6];    /**< Source address (6 bytes) */
    uint16_t packet_id;        /**< Packet identifier */
    uint8_t priority;          /**< Transmission priority (0-7) */
    int16_t rssi;              /**< RSSI for received packets */
    int16_t snr;               /**< SNR for received packets */
    bool is_ack_required;      /**< Whether acknowledgment is required */
} rf_packet_t;

/**
 * @brief Callback function type for packet reception
 */
typedef void (*rf_rx_callback_t)(rf_packet_t* packet, void* user_data);

/* Function Prototypes */

/**
 * @brief Initialize the RF controller
 *
 * @return RF_STATUS_OK on success, appropriate error code otherwise
 */
rf_status_t rf_init(void);

/**
 * @brief Deinitialize the RF controller
 *
 * @return RF_STATUS_OK on success, appropriate error code otherwise
 */
rf_status_t rf_deinit(void);

/**
 * @brief Configure the RF controller with specified settings
 *
 * @param config Pointer to configuration structure
 * @return RF_STATUS_OK on success, appropriate error code otherwise
 */
rf_status_t rf_configure(const rf_config_t* config);

/**
 * @brief Start transmission of RF packet
 *
 * @param packet Pointer to RF packet structure
 * @return RF_STATUS_OK on success, appropriate error code otherwise
 */
rf_status_t rf_transmit(const rf_packet_t* packet);

/**
 * @brief Start reception of RF packets
 *
 * @param callback Function to call when packet is received
 * @param user_data User data pointer passed to callback
 * @param timeout_ms Reception timeout in milliseconds (0 for continuous)
 * @return RF_STATUS_OK on success, appropriate error code otherwise
 */
rf_status_t rf_receive(rf_rx_callback_t callback, void* user_data, uint32_t timeout_ms);

/**
 * @brief Stop ongoing RF reception
 *
 * @return RF_STATUS_OK on success, appropriate error code otherwise
 */
rf_status_t rf_stop_receive(void);

/**
 * @brief Set RF transmit power level
 *
 * @param power_level Power level to set
 * @return RF_STATUS_OK on success, appropriate error code otherwise
 */
rf_status_t rf_set_power(rf_power_level_t power_level);

/**
 * @brief Put RF controller into low-power mode
 *
 * @param deep_sleep Whether to enter deep sleep mode
 * @return RF_STATUS_OK on success, appropriate error code otherwise
 */
rf_status_t rf_sleep(bool deep_sleep);

/**
 * @brief Wake up RF controller from low-power mode
 *
 * @return RF_STATUS_OK on success, appropriate error code otherwise
 */
rf_status_t rf_wake(void);

/**
 * @brief Perform RF calibration
 *
 * @return RF_STATUS_OK on success, appropriate error code otherwise
 */
rf_status_t rf_calibrate(void);

/**
 * @brief Select which antenna to use
 *
 * @param antenna_index Index of antenna to use
 * @return RF_STATUS_OK on success, appropriate error code otherwise
 */
rf_status_t rf_select_antenna(uint8_t antenna_index);

/**
 * @brief Enable autonomous antenna diversity (auto-switching)
 *
 * @param enable Whether to enable autonomous antenna diversity
 * @return RF_STATUS_OK on success, appropriate error code otherwise
 */
rf_status_t rf_enable_antenna_diversity(bool enable);

/**
 * @brief Get current RF controller state and statistics
 *
 * @param state Pointer to state structure to fill
 * @return RF_STATUS_OK on success, appropriate error code otherwise
 */
rf_status_t rf_get_state(rf_state_t* state);

/**
 * @brief Reset RF controller statistics
 *
 * @return RF_STATUS_OK on success, appropriate error code otherwise
 */
rf_status_t rf_reset_stats(void);

/**
 * @brief Set callback for RF status changes
 *
 * @param callback Function to call when status changes
 * @param user_data User data pointer passed to callback
 * @return RF_STATUS_OK on success, appropriate error code otherwise
 */
rf_status_t rf_set_status_callback(void (*callback)(rf_status_t status, void* user_data), void* user_data);

/**
 * @brief Perform radiation-hardening mitigation
 *
 * Performs error detection and correction operations to mitigate
 * radiation-induced Single Event Upsets (SEUs).
 *
 * @return RF_STATUS_OK on success, appropriate error code otherwise
 */
rf_status_t rf_radiation_mitigation(void);

/**
 * @brief Enable or disable Triple Modular Redundancy (TMR)
 *
 * @param level TMR level (0 = disabled, 1-3 = redundancy levels)
 * @return RF_STATUS_OK on success, appropriate error code otherwise
 */
rf_status_t rf_set_redundancy_level(uint8_t level);

/**
 * @brief Get detailed error information
 *
 * @param error_code Pointer to variable to receive error code
 * @param error_str Pointer to buffer to receive error string
 * @param error_str_size Size of error_str buffer
 * @return RF_STATUS_OK on success, appropriate error code otherwise
 */
rf_status_t rf_get_error_info(uint32_t* error_code, char* error_str, size_t error_str_size);

#endif /* RF_CONTROLLER_H */

