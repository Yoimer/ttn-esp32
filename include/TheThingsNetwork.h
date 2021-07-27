/*******************************************************************************
 * 
 * ttn-esp32 - The Things Network device library for ESP-IDF / SX127x
 * 
 * Copyright (c) 2018-2021 Manuel Bleichenbacher
 * 
 * Licensed under MIT License
 * https://opensource.org/licenses/MIT
 *
 * High-level C++ API for ttn-esp32.
 *******************************************************************************/

#ifndef _THETHINGSNETWORK_H_
#define _THETHINGSNETWORK_H_

#include "ttn.h"

typedef ttn_port_t port_t;

/**
 * @brief Response codes
 */
enum TTNResponseCode
{
    kTTNErrorTransmissionFailed = TTN_ERROR_TRANSMISSION_FAILED,
    kTTNErrorUnexpected = TTN_ERROR_UNEXPECTED,
    kTTNSuccessfulTransmission = TTN_SUCCESSFUL_TRANSMISSION,
    kTTNSuccessfulReceive = TTN_SUCCESSFUL_RECEIVE
};


/**
 * @brief RX/TX window
 */
enum TTNRxTxWindow
{
    /**
     * @brief Outside RX/TX window
     */
    kTTNIdleWindow = TTN_WINDOW_IDLE,
    /**
     * @brief Transmission window (up to RX1 window)
     */
    kTTNTxWindow = TTN_WINDOW_TX,
    /**
     * @brief Reception window 1 (up to RX2 window)
     */
    kTTNRx1Window = TTN_WINDOW_RX1,
    /**
     * @brief Reception window 2
     */
    kTTNRx2Window = TTN_WINDOW_RX2
};


/**
 * @brief Spreading Factor
 */
enum TTNSpreadingFactor
{
    /**
     * @brief Unused / undefined spreading factor
     */
    kTTNSFNone = TTN_SF_NONE,
    /**
     * @brief Frequency Shift Keying (FSK)
     */
    kTTNFSK = TTN_FSK,
    /**
     * @brief Spreading Factor 7 (SF7)
     */
    kTTNSF7 = TTN_SF7,
    /**
     * @brief Spreading Factor 8 (SF8)
     */
    kTTNSF8 = TTN_SF8,
    /**
     * @brief Spreading Factor 9 (SF9)
     */
    kTTNSF9 = TTN_SF9,
    /**
     * @brief Spreading Factor 10 (SF10)
     */
    kTTNSF10 = TTN_SF10,
    /**
     * @brief Spreading Factor 11 (SF11)
     */
    kTTNSF11 = TTN_SF11,
    /**
     * @brief Spreading Factor 12 (SF12)
     */
    kTTNSF12 = TTN_SF12
};


/**
 * @brief Bandwidth
 */
enum TTNBandwidth
{
    /**
     * @brief Undefined/unused bandwidth
     */
    kTTNBWNone = TTN_BW_NONE,
    /**
     * @brief Bandwidth of 125 kHz
     */
    kTTNBW125 = TTN_BW125,
    /**
     * @brief Bandwidth of 250 kHz
     */
    kTTNBW250 = TTN_BW250,
    /**
     * @brief Bandwidth of 500 kHz
     */
    kTTNBW500 = TTN_BW500
};


/**
 * @brief RF settings for TX or RX
 */
struct TTNRFSettings
{
    /**
     * @brief Spreading Factor (SF)
     */
    TTNSpreadingFactor spreadingFactor;
    /**
     * @brief Bandwidth (BW)
     */
    TTNBandwidth bandwidth;
    /**
     * @brief Frequency, in Hz
     */
    uint32_t frequency;
};


/**
 * @brief Callback for recieved messages
 * 
 * @param payload  pointer to the received bytes
 * @param length   number of received bytes
 * @param port     port the message was received on
 */
typedef void (*TTNMessageCallback)(const uint8_t* payload, size_t length, port_t port);

/**
 * @brief TTN device
 * 
 * This class enables ESP32 devices with SX1272/73/76/77/78/79 LoRaWAN chips
 * to communicate via The Things Network.
 * 
 * Only one instance of this class may be created.
 */
class TheThingsNetwork
{
public:
    /**
     * @brief Constructs a new The Things Network device instance.
     */
    TheThingsNetwork() { ttn_init(); }

    /**
     * @brief Destroys the The Things Network device instance.
     */
    ~TheThingsNetwork() { }

    /**
     * @brief Resets the LoRaWAN radio.
     * 
     * To restart communication, join() must be called.
     * It neither clears the provisioned keys nor the configured pins.
     */
    void reset() { ttn_reset(); }

    /**
     * @brief Configures the pins used to communicate with the LoRaWAN radio chip.
     * 
     * Before calling this member function, the SPI bus needs to be configured using `spi_bus_initialize()`. 
     * Additionally, `gpio_install_isr_service()` must have been called to initialize the GPIO ISR handler service.
     * 
     * @param spi_host  The SPI bus/peripherial to use (`SPI_HOST`, `HSPI_HOST` or `VSPI_HOST`).
     * @param nss       The GPIO pin number connected to the radio chip's NSS pin (serving as the SPI chip select)
     * @param rxtx      The GPIO pin number connected to the radio chip's RXTX pin (`TTN_NOT_CONNECTED` if not connected)
     * @param rst       The GPIO pin number connected to the radio chip's RST pin (`TTN_NOT_CONNECTED` if not connected)
     * @param dio0      The GPIO pin number connected to the radio chip's DIO0 pin
     * @param dio1      The GPIO pin number connected to the radio chip's DIO1 pin
     */
    void configurePins(spi_host_device_t spi_host, uint8_t nss, uint8_t rxtx, uint8_t rst, uint8_t dio0, uint8_t dio1)
    {
        ttn_configure_pins(spi_host, nss, rxtx, rst, dio0, dio1);
    }

    /**
     * @brief Sets the frequency sub-band to be used.
     * 
     * For regions with sub-bands (USA, Australia), sets the sub-band to be used for uplink communication.
     * For other regions, this function has no effect.
     * 
     * The sub-band must be set before joining or sending the first message.
     * 
     * If not set, it defaults to sub-band 2 as defined by TTN.
     * 
     * @param band band (0 for all bands, or value between 1 and 8)
     */
    void setSubband(int band) { ttn_set_subband(band); }

    /**
     * @brief Sets the credentials needed to activate the device via OTAA, without activating it.
     * 
     * The provided DevEUI, AppEUI/JoinEUI and AppKey are saved in non-volatile memory. Before
     * this function is called, `nvs_flash_init()` must have been called once.
     * 
     * Call join() to activate the device.
     * 
     * @param devEui  DevEUI (16 character string with hexadecimal data)
     * @param appEui  AppEUI/JoinEUI of the device (16 character string with hexadecimal data)
     * @param appKey  AppKey of the device (32 character string with hexadecimal data)
     * @return `true` if the provisioning was successful, `false`  if the provisioning failed
     */
    bool provision(const char *devEui, const char *appEui, const char *appKey) { return ttn_provision(devEui, appEui, appKey); }

    /**
     * @brief Sets the information needed to activate the device via OTAA, using the MAC to generate the DevEUI
     * and without activating it.
     * 
     * The generated DevEUI and the provided AppEUI/JoinEUI and AppKey are saved in non-volatile memory. Before
     * this function is called, 'nvs_flash_init' must have been called once.
     * 
     * The DevEUI is generated by retrieving the ESP32's WiFi MAC address and expanding it into a DevEUI
     * by adding FFFE in the middle. So the MAC address A0:B1:C2:01:02:03 becomes the EUI A0B1C2FFFE010203.
     * This hexadecimal data can be entered into the DevEUI field in the TTN console.
     * 
     * Generating the DevEUI from the MAC address allows to flash the same AppEUI/JoinEUI and AppKey to a batch of
     * devices. However, using the same AppKey for multiple devices is insecure. Only use this approach if
     * it is okay for that the LoRa communication of your application can easily be intercepted and that
     * forged data can be injected.
     * 
     * Call join() to activate.
     * 
     * @param appEui  AppEUI/JoinEUI of the device (16 character string with hexadecimal data)
     * @param appKey  AppKey of the device (32 character string with hexadecimal data)
     * @return `true` if the provisioning was successful, `false`  if the provisioning failed
     */
    bool provisionWithMAC(const char *appEui, const char *appKey) { return ttn_provision_with_mac(appEui, appKey); }

    /**
     * @brief Starts task listening on configured UART for AT commands.
     * 
     * Run `make menuconfig` to configure it.
     */
    void startProvisioningTask() { ttn_start_provisioning_task(); }

    /**
     * @brief Waits until the DevEUI, AppEUI/JoinEUI and AppKey have been provisioned
     * by the provisioning task.
     * 
     * If the device has already been provisioned (stored data in NVS, call of provision()
     * or call of join(const char*, const char*, const char*), this function
     * immediately returns.
     */
    void waitForProvisioning() { ttn_wait_for_provisioning(); }

     /**
     * @brief Activates the device via OTAA.
     * 
     * The DevEUI, AppEUI/JoinEUI and AppKey must have already been provisioned by a call to provision().
     * Before this function is called, `nvs_flash_init()` must have been called once.
     * 
     * The function blocks until the activation has completed or failed.
     * 
     * @return `true` if the activation was succesful, `false` if the activation failed
     */
    bool join() { return ttn_join_provisioned(); }

   /**
     * @brief Sets the DevEUI, AppEUI/JoinEUI and AppKey and activate the device via OTAA.
     * 
     * The DevEUI, AppEUI/JoinEUI and AppKey are NOT saved in non-volatile memory.
     * 
     * The function blocks until the activation has completed or failed.
     * 
     * @param devEui  DevEUI (16 character string with hexadecimal data)
     * @param appEui  AppEUI/JoinEUI of the device (16 character string with hexadecimal data)
     * @param appKey  AppKey of the device (32 character string with hexadecimal data)
     * @return `true` if the activation was succesful, `false` if the activation failed
     */
    bool join(const char *devEui, const char *appEui, const char *appKey) { return ttn_join(devEui, appEui, appKey); }

    /**
     * @brief Transmits a message
     * 
     * The function blocks until the message could be transmitted and a message has been received
     * in the subsequent receive window (or the window expires). Additionally, the function will
     * first wait until the duty cycle allows a transmission (enforcing the duty cycle limits).
     * 
     * @param payload  bytes to be transmitted
     * @param length   number of bytes to be transmitted
     * @param port     port (defaults to 1)
     * @param confirm  flag indicating if a confirmation should be requested. Defaults to `false`
     * @return `kTTNSuccessfulTransmission` for successful transmission, `kTTNErrorTransmissionFailed` for failed transmission, `kTTNErrorUnexpected` for unexpected error
     */
    TTNResponseCode transmitMessage(const uint8_t *payload, size_t length, port_t port = 1, bool confirm = false)
    {
        return static_cast<TTNResponseCode>(ttn_transmit_message(payload, length, port, confirm));
    }

    /**
     * @brief Sets the function to be called when a message is received
     * 
     * When a message is received, the specified function is called. The
     * message, its length and the port number are provided as
     * parameters. The values are only valid during the duration of the
     * callback. So they must be immediately processed or copied.
     * 
     * Messages are received as a result of transmitMessage(). The callback is called
     * in the task that called any of these functions and it occurs before these functions
     * return control to the caller.
     * 
     * @param callback  the callback function
     */
    void onMessage(TTNMessageCallback callback) { ttn_on_message(callback); }

    /**
     * @brief Checks if DevEUI, AppEUI/JoinEUI and AppKey have been stored in non-volatile storage
     * or have been provided as by a call to join(const char*, const char*, const char*).
     * 
     * @return `true` if they are stored, complete and of the correct size, `false` otherwise
     */
    bool isProvisioned() { return ttn_is_provisioned(); }

    /**
     * @brief Sets the RSSI calibration value for LBT (Listen Before Talk).
     * 
     * This value is added to RSSI measured prior to decision. It must include the guardband.
     * Ignored in US, EU, IN and other countries where LBT is not required.
     * Defaults to 10 dB.
     * 
     * @param rssiCal RSSI calibration value, in dB
     */
    void setRSSICal(int8_t rssiCal) { ttn_set_rssi_cal(rssiCal); }

    /**
     * Returns whether Adaptive Data Rate (ADR) is enabled.
     * 
     * @return `true` if enabled, `false` if disabled
     */
    bool adrEnabled() { return ttn_adr_enabled(); }

    /**
     * @brief Enables or disabled Adaptive Data Rate (ADR).
     * 
     * ADR is enabled by default. It optimizes data rate, airtime and energy consumption
     * for devices with stable RF conditions. It should be turned off for mobile devices.
     * 
     * @param enabled `true` to enable, `false` to disable
     */ 
    void setAdrEnabled(bool enabled) { ttn_set_adr_enabled(enabled); }

    /**
     * @brief Stops all activies and shuts down the RF module and the background tasks.
     * 
     * To restart communication, startup() and join() must be called.
     * it neither clears the provisioned keys nor the configured pins.
     */
    void shutdown() { ttn_shutdown(); }

    /**
     * @brief Restarts the background tasks and RF module.
     * 
     * This member function must only be called after a call to shutdowna().
     */
    void startup() { ttn_startup(); }

    /**
     * @brief Gets current RX/TX window
     * @return window
     */
    TTNRxTxWindow rxTxWindow() { return static_cast<TTNRxTxWindow>(ttn_rx_tx_window()); }

    /**
     * @brief Gets the RF settings for the specified window
     * @param window RX/TX windows (valid values are `kTTNTxWindow`, `kTTNRx1Window` and `kTTNRx2Window`)
     */
    TTNRFSettings getRFSettings(TTNRxTxWindow window);

    /**
     * @brief Gets the RF settings of the last (or ongoing) transmission.
     * @return RF settings
     */
    TTNRFSettings txSettings() { return getRFSettings(kTTNTxWindow); }

    /**
     * @brief Gets the RF settings of the last (or ongoing) reception of RX window 1.
     * @return RF settings
     */
    TTNRFSettings rx1Settings() { return getRFSettings(kTTNRx1Window); }

    /**
     * @brief Gets the RF settings of the last (or ongoing) reception of RX window 2.
     * @return RF settings
     */
    TTNRFSettings rx2Settings() { return getRFSettings(kTTNRx2Window); }

    /**
     * @brief Gets the received signal strength indicator (RSSI).
     * 
     * RSSI is the measured signal strength of the last recevied message (incl. join responses).
     * 
     * @return RSSI, in dBm
     */
    int rssi() { return ttn_rssi(); }
};

#endif
