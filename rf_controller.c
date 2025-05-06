/**
 * @file rf_controller.c
 * @brief RF Controller Implementation for SkyMesh Satellite System
 *
 * This file implements the RF controller interface for the SkyMesh satellite platform.
 * It provides support for both the UHF/VHF (AX5043) and S-band (AT86RF233) transceivers
 * with radiation-hardened design considerations.
 */

#include "rf_controller.h"
#include <string.h>
#include <stdlib.h>

/* Include hardware-specific headers */
#include "ax5043_driver.h"   /* For UHF/VHF transceiver */
#include "at86rf233_driver.h" /* For S-band transceiver */
#include "radiation_hardening.h" /* For radiation protection features */

/* Define hardware-specific constants */
#define AX5043_DEVICE_ADDR      0x3C  /* I2C device address for AX5043 */
#define AT86RF233_SPI_PORT      0     /* SPI port for AT86RF233 */
#define MAX_PACKET_SIZE         256   /* Maximum packet size in bytes */
#define MAX_ANTENNAS            4     /* Maximum number of antennas */

/* Static variables for RF controller state */
static bool rf_initialized = false;
static rf_config_t current_config;
static rf_state_t current_state;
static rf_rx_callback_t rx_callback = NULL;
static void* rx_callback_data = NULL;
static void (*status_callback)(rf_status_t status, void* user_data) = NULL;
static void* status_callback_data = NULL;
static uint8_t current_antenna = 0;
static bool antenna_diversity_enabled = false;
static uint8_t redundancy_level = 0;

/* Triple Modular Redundancy (TMR) buffers for radiation hardening */
static uint8_t tmr_config_copies[3][sizeof(rf_config_t)];
static uint8_t tmr_state_copies[3][sizeof(rf_state_t)];

/**
 * @brief Update status and notify callback if registered
 *
 * @param status New status to set
 */
static void update_status(rf_status_t status) {
    current_state.status = status;
    
    /* Notify status change via callback if registered */
    if (status_callback != NULL) {
        status_callback(status, status_callback_data);
    }
}

/**
 * @brief Perform Triple Modular Redundancy (TMR) protection on a data structure
 *
 * @param data Pointer to the data to protect
 * @param copies Array of TMR copies
 * @param size Size of the data structure
 * @return true if correction was successful, false if uncorrectable error
 */
static bool tmr_protect(void* data, uint8_t copies[][sizeof(rf_state_t)], size_t size) {
    if (redundancy_level == 0) {
        return true; /* TMR disabled, no protection */
    }
    
    /* Copy original data to TMR buffers */
    for (int i = 0; i < redundancy_level; i++) {
        memcpy(copies[i], data, size);
    }
    
    return true;
}

/**
 * @brief Recover data using Triple Modular Redundancy (TMR)
 *
 * @param data Pointer to the data to recover
 * @param copies Array of TMR copies
 * @param size Size of the data structure
 * @return true if recovery was successful, false if uncorrectable error
 */
static bool tmr_recover(void* data, uint8_t copies[][sizeof(rf_state_t)], size_t size) {
    if (redundancy_level < 2) {
        return true; /* Not enough redundancy for recovery */
    }
    
    /* Simple majority voting for recovery */
    int errors_detected = 0;
    
    /* Check if original data matches any copy */
    bool match_found = false;
    for (int i = 0; i < redundancy_level; i++) {
        if (memcmp(data, copies[i], size) == 0) {
            match_found = true;
            break;
        }
    }
    
    if (!match_found) {
        /* Original data doesn't match any copy, attempt recovery */
        if (redundancy_level >= 3) {
            /* With 3 copies, we can do majority voting */
            /* This is a simplified implementation - a real one would vote bit by bit */
            if (memcmp(copies[0], copies[1], size) == 0) {
                /* Copies 0 and 1 match, use them */
                memcpy(data, copies[0], size);
                errors_detected = 1;
            } else if (memcmp(copies[0], copies[2], size) == 0) {
                /* Copies 0 and 2 match, use them */
                memcpy(data, copies[0], size);
                errors_detected = 1;
            } else if (memcmp(copies[1], copies[2], size) == 0) {
                /* Copies 1 and 2 match, use them */
                memcpy(data, copies[1], size);
                errors_detected = 1;
            } else {
                /* All three copies are different, uncorrectable error */
                current_state.radiation_errors++;
                return false;
            }
        } else {
            /* With 2 copies, we can only detect errors, not correct them */
            current_state.radiation_errors++;
            return false;
        }
    }
    
    current_state.radiation_errors += errors_detected;
    return true;
}

/**
 * @brief Initialize the RF controller
 *
 * @return RF_STATUS_OK on success, appropriate error code otherwise
 */
rf_status_t rf_init(void) {
    if (rf_initialized) {
        return RF_STATUS_OK; /* Already initialized */
    }
    
    /* Initialize state variables */
    memset(&current_state, 0, sizeof(rf_state_t));
    current_state.status = RF_STATUS_OK;
    
    /* Set defaults for configuration */
    memset(&current_config, 0, sizeof(rf_config_t));
    current_config.band = RF_BAND_UHF;
    current_config.frequency_hz = 437000000; /* 437 MHz - common CubeSat frequency */
    current_config.bandwidth_hz = 25000;     /* 25 kHz */
    current_config.modulation = RF_MOD_GMSK;
    current_config.fec = RF_FEC_NONE;
    current_config.preamble_length = 32;
    current_config.sync_word_size = 4;
    memcpy(current_config.sync_word, "\xAA\xBB\xCC\xDD", 4);
    current_config.power_level = RF_POWER_MEDIUM;
    current_config.auto_power_control = true;
    current_config.radiation_hardening = true;
    current_config.redundancy_level = 3;
    
    /* Initialize hardware based on default band */
    bool hw_init_success = false;
    
    if (current_config.band == RF_BAND_UHF) {
        /* Initialize UHF/VHF transceiver (AX5043) */
        hw_init_success = ax5043_init(AX5043_DEVICE_ADDR);
    } else if (current_config.band == RF_BAND_S) {
        /* Initialize S-band transceiver (AT86RF233) */
        hw_init_success = at86rf233_init(AT86RF233_SPI_PORT);
    }
    
    if (!hw_init_success) {
        update_status(RF_STATUS_INIT_ERROR);
        return RF_STATUS_INIT_ERROR;
    }
    
    /* Initialize radiation hardening */
    if (current_config.radiation_hardening) {
        redundancy_level = current_config.redundancy_level;
        radiation_hardening_init(redundancy_level);
        
        /* Protect initial configuration and state with TMR */
        tmr_protect(&current_config, (uint8_t (*)[sizeof(rf_state_t)])tmr_config_copies, sizeof(rf_config_t));
        tmr_protect(&current_state, tmr_state_copies, sizeof(rf_state_t));
    }
    
    rf_initialized = true;
    update_status(RF_STATUS_OK);
    return RF_STATUS_OK;
}

/**
 * @brief Deinitialize the RF controller
 *
 * @return RF_STATUS_OK on success, appropriate error code otherwise
 */
rf_status_t rf_deinit(void) {
    if (!rf_initialized) {
        return RF_STATUS_OK; /* Already deinitialized */
    }
    
    /* Deinitialize hardware based on current band */
    bool hw_deinit_success = false;
    
    if (current_config.band == RF_BAND_UHF) {
        /* Deinitialize UHF/VHF transceiver (AX5043) */
        hw_deinit_success = ax5043_deinit();
    } else if (current_config.band == RF_BAND_S) {
        /* Deinitialize S-band transceiver (AT86RF233) */
        hw_deinit_success = at86rf233_deinit();
    }
    
    if (!hw_deinit_success) {
        update_status(RF_STATUS_UNKNOWN_ERROR);
        return RF_STATUS_UNKNOWN_ERROR;
    }
    
    /* Reset state variables */
    rf_initialized = false;
    rx_callback = NULL;
    rx_callback_data = NULL;
    status_callback = NULL;
    status_callback_data = NULL;
    
    update_status(RF_STATUS_OK);
    return RF_STATUS_OK;
}

/**
 * @brief Configure the RF controller with specified settings
 *
 * @param config Pointer to configuration structure
 * @return RF_STATUS_OK on success, appropriate error code otherwise
 */
rf_status_t rf_configure(const rf_config_t* config) {
    if (!rf_initialized) {
        return RF_STATUS_INIT_ERROR;
    }
    
    if (config == NULL) {
        update_status(RF_STATUS_CONFIG_ERROR);
        return RF_STATUS_CONFIG_ERROR;
    }
    
    /* Check if we need to switch transceivers due to band change */
    if (current_config.band != config->band) {
        /* Deinitialize current transceiver */
        rf_deinit();
        
        /* Update configuration */
        memcpy(&current_config, config, sizeof(rf_config_t));
        
        /* Reinitialize with new band */
        return rf_init();
    }
    
    /* Apply configuration to appropriate hardware */
    bool config_success = false;
    
    if (config->band == RF_BAND_UHF) {
        /* Configure UHF/VHF transceiver (AX5043) */
        ax5043_config_t ax_config;
        
        /* Map generic config to AX5043-specific config */
        ax_config.frequency = config->frequency_hz;
        ax_config.bandwidth = config->bandwidth_hz;
        ax_config.modulation = (config->modulation == RF_MOD_GMSK) ? AX5043_MOD_GMSK :
                              (config->modulation == RF_MOD_FSK) ? AX5043_MOD_FSK :
                              (config->modulation == RF_MOD_BPSK) ? AX5043_MOD_BPSK : AX5043_MOD_GMSK;
        ax_config.power_level = config->power_level;
        ax_config.preamble_length = config->preamble_length;
        memcpy(ax_config.sync_word, config->sync_word, config->sync_word_size);
        ax_config.sync_word_size = config->sync_word_size;
        
        config_success = ax5043_configure(&ax_config);
    } else if (config->band == RF_BAND_S) {
        /* Configure S-band transceiver (AT86RF233) */
        at86rf233_config_t at_config;
        
        /* Map generic config to AT86RF233-specific config */
        at_config.frequency = config->frequency_hz;
        at_config.bandwidth = config->bandwidth_hz;
        at_config.modulation = (config->modulation == RF_MOD_BPSK) ? AT86RF233_MOD_BPSK :
                              (config->modulation == RF_MOD_QPSK) ? AT86RF233_MOD_QPSK :
                              (config->modulation == RF_MOD_16QAM) ? AT86RF233_MOD_16QAM : AT86RF233_MOD_BPSK;
        at_config.power_level = config->power_level;
        at_config.preamble_length = config->preamble_length;
        memcpy(at_config.sync_word, config->sync_word, config->sync_word_size);
        at_config.sync_word_size = config->sync_word_size;
        
        config_success = at86rf233_configure(&at_config);
    }
    
    if (!config_success) {
        update_status(RF_STATUS_CONFIG_ERROR);
        current_state.error_count++;
        return RF_STATUS_CONFIG_ERROR;
    }
    
    /* Update current configuration */
    memcpy(&current_config, config, sizeof(rf_config_t));
    
    /* Update redundancy level if changed */
    if (config->radiation_hardening) {
        redundancy_level = config->redundancy_level;
        
        /* Protect new configuration with TMR */
        tmr_protect(&current_config, (uint8_t (*)[sizeof(rf_state_t)])tmr_config_copies, sizeof(rf_config_t));
    } else {
        redundancy_level = 0;
    }
    
    update_status(RF_STATUS_OK);
    return RF_STATUS_OK;
}

/**
 * @brief Start transmission of RF packet
 *
 * @param packet Pointer to RF packet structure
 * @return RF_STATUS_OK on success, appropriate error code otherwise
 */
rf_status_t rf_transmit(const rf_packet_t* packet) {
    if (!rf_initialized) {
        return RF_STATUS_INIT_ERROR;
    }
    
    if (packet == NULL || packet->data == NULL || packet->length == 0 || 
        packet->length > MAX_PACKET_SIZE) {
        update_status(RF_STATUS_TX_ERROR);
        current_state.error_count++;
        return RF_STATUS_TX_ERROR;
    }
    
    /* Set state to transmitting */
    current_state.is_transmitting = true;
    
    bool tx_success = false;
    
    /* Transmit on appropriate hardware */
    if (current_config.band == RF_BAND_UHF) {
        /* Transmit using UHF/VHF transceiver (AX5043) */
        tx_success = ax5043_transmit(packet->data, packet->length);
    } else if (current_config.band == RF_BAND_S) {
        /* Transmit using S-band transceiver (AT86RF233) */
        tx_success = at86rf233_transmit(packet->data, packet->length);
    }
    
    if (!tx_success) {
        current_state.is_transmitting = false;
        update_status(RF_STATUS_TX_ERROR);
        current_state.error_count++;
        return RF_STATUS_TX_ERROR;
    }
    
    /* Update statistics */
    current_state.tx_packets++;
    current_state.tx_bytes += packet->length;
    
    /* Apply TMR protection to updated state */
    if (current_config.radiation_hardening) {
        tmr_protect(&current_state, tmr_state_copies, sizeof(rf_state_t));
    }
    
    update_status(RF_STATUS_OK);
    return RF_STATUS_OK;
}

/**
 * @brief Start non-blocking transmission of RF packet with callback on completion
 *
 * @param packet Pointer to RF packet structure
 * @param callback Function to call when transmission completes
 * @param user_data User data to pass to callback
 * @return RF_STATUS_OK on success, appropriate error code otherwise
 */
rf_status_t rf_transmit_async(const rf_packet_t* packet, rf_tx_callback_t callback, void* user_data) {
    if (!rf_initialized) {
        return RF_STATUS_INIT_ERROR;
    }
    
    if (packet == NULL || packet->data == NULL || packet->length == 0 || 
        packet->length > MAX_PACKET_SIZE) {
        update_status(RF_STATUS_TX_ERROR);
        current_state.error_count++;
        return RF_STATUS_TX_ERROR;
    }
    
    /* Set state to transmitting */
    current_state.is_transmitting = true;
    
    bool tx_success = false;
    
    /* Transmit asynchronously on appropriate hardware */
    if (current_config.band == RF_BAND_UHF) {
        /* Transmit using UHF/VHF transceiver (AX5043) */
        tx_success = ax5043_transmit_async(packet->data, packet->length, callback, user_data);
    } else if (current_config.band == RF_BAND_S) {
        /* Transmit using S-band transceiver (AT86RF233) */
        tx_success = at86rf233_transmit_async(packet->data, packet->length, callback, user_data);
    }
    
    if (!tx_success) {
        current_state.is_transmitting = false;
        update_status(RF_STATUS_TX_ERROR);
        current_state.error_count++;
        return RF_STATUS_TX_ERROR;
    }
    
    /* Update statistics */
    current_state.tx_packets++;
    current_state.tx_bytes += packet->length;
    
    /* Apply TMR protection to updated state */
    if (current_config.radiation_hardening) {
        tmr_protect(&current_state, tmr_state_copies, sizeof(rf_state_t));
    }
    
    update_status(RF_STATUS_OK);
    return RF_STATUS_OK;
}

/**
 * @brief Start receiving RF packets
 *
 * @param callback Function to call when packet is received
 * @param user_data User data to pass to callback
 * @return RF_STATUS_OK on success, appropriate error code otherwise
 */
rf_status_t rf_start_receive(rf_rx_callback_t callback, void* user_data) {
    if (!rf_initialized) {
        return RF_STATUS_INIT_ERROR;
    }
    
    if (callback == NULL) {
        update_status(RF_STATUS_RX_ERROR);
        current_state.error_count++;
        return RF_STATUS_RX_ERROR;
    }
    
    /* Store callback and user data */
    rx_callback = callback;
    rx_callback_data = user_data;
    
    /* Set state to receiving */
    current_state.is_receiving = true;
    
    bool rx_success = false;
    
    /* Start receiving on appropriate hardware */
    if (current_config.band == RF_BAND_UHF) {
        /* Start receiving on UHF/VHF transceiver (AX5043) */
        rx_success = ax5043_start_receive(rx_internal_callback, rx_callback_data);
    } else if (current_config.band == RF_BAND_S) {
        /* Start receiving on S-band transceiver (AT86RF233) */
        rx_success = at86rf233_start_receive(rx_internal_callback, rx_callback_data);
    }
    
    if (!rx_success) {
        current_state.is_receiving = false;
        rx_callback = NULL;
        rx_callback_data = NULL;
        update_status(RF_STATUS_RX_ERROR);
        current_state.error_count++;
        return RF_STATUS_RX_ERROR;
    }
    
    /* Apply TMR protection to updated state */
    if (current_config.radiation_hardening) {
        tmr_protect(&current_state, tmr_state_copies, sizeof(rf_state_t));
    }
    
    update_status(RF_STATUS_OK);
    return RF_STATUS_OK;
}

/**
 * @brief Internal callback for processing received packets
 *
 * @param data Pointer to received data
 * @param length Length of received data
 * @param rssi RSSI value of received packet
 * @param user_data User data passed to receive function
 */
static void rx_internal_callback(const uint8_t* data, size_t length, int8_t rssi, void* user_data) {
    /* Update statistics */
    current_state.rx_packets++;
    current_state.rx_bytes += length;
    current_state.last_rssi = rssi;
    
    /* Create packet structure */
    rf_packet_t packet;
    packet.data = data;
    packet.length = length;
    packet.rssi = rssi;
    
    /* Apply error detection/correction if enabled */
    if (current_config.fec != RF_FEC_NONE) {
        /* Perform FEC decoding based on configuration */
        bool decode_success = false;
        
        switch (current_config.fec) {
            case RF_FEC_HAMMING:
                decode_success = fec_decode_hamming((uint8_t*)data, length);
                break;
            case RF_FEC_GOLAY:
                decode_success = fec_decode_golay((uint8_t*)data, length);
                break;
            case RF_FEC_REED_SOLOMON:
                decode_success = fec_decode_reed_solomon((uint8_t*)data, length);
                break;
            default:
                /* No FEC or unknown FEC type */
                decode_success = true;
                break;
        }
        
        if (!decode_success) {
            current_state.rx_errors++;
            return; /* Skip callback if decode failed */
        }
    }
    
    /* Call user callback with packet */
    if (rx_callback != NULL) {
        rx_callback(&packet, user_data);
    }
    
    /* Apply TMR protection to updated state */
    if (current_config.radiation_hardening) {
        tmr_protect(&current_state, tmr_state_copies, sizeof(rf_state_t));
    }
}

/**
 * @brief Stop receiving RF packets
 *
 * @return RF_STATUS_OK on success, appropriate error code otherwise
 */
rf_status_t rf_stop_receive(void) {
    if (!rf_initialized) {
        return RF_STATUS_INIT_ERROR;
    }
    
    if (!current_state.is_receiving) {
        return RF_STATUS_OK; /* Already stopped */
    }
    
    bool rx_stop_success = false;
    
    /* Stop receiving on appropriate hardware */
    if (current_config.band == RF_BAND_UHF) {
        /* Stop receiving on UHF/VHF transceiver (AX5043) */
        rx_stop_success = ax5043_stop_receive();
    } else if (current_config.band == RF_BAND_S) {
        /* Stop receiving on S-band transceiver (AT86RF233) */
        rx_stop_success = at86rf233_stop_receive();
    }
    
    if (!rx_stop_success) {
        update_status(RF_STATUS_RX_ERROR);
        current_state.error_count++;
        return RF_STATUS_RX_ERROR;
    }
    
    /* Reset receiving state */
    current_state.is_receiving = false;
    rx_callback = NULL;
    rx_callback_data = NULL;
    
    /* Apply TMR protection to updated state */
    if (current_config.radiation_hardening) {
        tmr_protect(&current_state, tmr_state_copies, sizeof(rf_state_t));
    }
    
    update_status(RF_STATUS_OK);
    return RF_STATUS_OK;
}

/**
 * @brief Set the power state of the RF transceivers
 *
 * @param power_state Power state to set
 * @return RF_STATUS_OK on success, appropriate error code otherwise
 */
rf_status_t rf_set_power_state(rf_power_state_t power_state) {
    if (!rf_initialized) {
        return RF_STATUS_INIT_ERROR;
    }
    
    bool power_success = false;
    
    /* Set power state on appropriate hardware */
    if (current_config.band == RF_BAND_UHF) {
        /* Set power state on UHF/VHF transceiver (AX5043) */
        ax5043_power_state_t ax_power_state;
        
        /* Map generic power state to AX5043-specific power state */
        switch (power_state) {
            case RF_POWER_STATE_OFF:
                ax_power_state = AX5043_POWER_STATE_OFF;
                break;
            case RF_POWER_STATE_SLEEP:
                ax_power_state = AX5043_POWER_STATE_SLEEP;
                break;
            case RF_POWER_STATE_STANDBY:
                ax_power_state = AX5043_POWER_STATE_STANDBY;
                break;
            case RF_POWER_STATE_ACTIVE:
                ax_power_state = AX5043_POWER_STATE_ACTIVE;
                break;
            default:
                update_status(RF_STATUS_CONFIG_ERROR);
                current_state.error_count++;
                return RF_STATUS_CONFIG_ERROR;
        }
        
        power_success = ax5043_set_power_state(ax_power_state);
    } else if (current_config.band == RF_BAND_S) {
        /* Set power state on S-band transceiver (AT86RF233) */
        at86rf233_power_state_t at_power_state;
        
        /* Map generic power state to AT86RF233-specific power state */
        switch (power_state) {
            case RF_POWER_STATE_OFF:
                at_power_state = AT86RF233_POWER_STATE_OFF;
                break;
            case RF_POWER_STATE_SLEEP:
                at_power_state = AT86RF233_POWER_STATE_SLEEP;
                break;
            case RF_POWER_STATE_STANDBY:
                at_power_state = AT86RF233_POWER_STATE_STANDBY;
                break;
            case RF_POWER_STATE_ACTIVE:
                at_power_state = AT86RF233_POWER_STATE_ACTIVE;
                break;
            default:
                update_status(RF_STATUS_CONFIG_ERROR);
                current_state.error_count++;
                return RF_STATUS_CONFIG_ERROR;
        }
        
        power_success = at86rf233_set_power_state(at_power_state);
    }
    
    if (!power_success) {
        update_status(RF_STATUS_POWER_ERROR);
        current_state.
