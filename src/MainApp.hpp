#pragma once

#include <Arduino.h>

#include "config/SystemConfig.hpp"
#include "config/AppConfig.hpp"

// Couche MID - materiel
#include "mid/hal/sensors/INA237Driver.hpp"
#include "mid/hal/sensors/TMP126Driver.hpp"
#include "mid/hal/ihm/LED.hpp"
#include "mid/hal/ihm/Buzzer.hpp"
#include "mid/hal/ihm/Button.hpp"
#include "mid/memory/RingBuffer.hpp"
#include "mid/memory/FlashStorage.hpp"

// Couche MID - communication
#include "mid/ble/BLEManager.hpp"

// Couche APP - services
#include "app/services/MonitoringService.hpp"
#include "app/services/AlarmService.hpp"
#include "app/services/EventLoggingService.hpp"
#include "app/services/ConfigService.hpp"

class MainApp final
{
public:
    Status init();
    void process();

private:
    void processNominalState(bool is_boot_button_pressed);
    void updateHmi();
    void triggerSafetyShutdown();

    static const char* stateToString(AgvState state);

    // Instances drivers : ownership dans MainApp.
    INA237Driver ina237_driver_;
    TMP126Driver tmp126_driver_;
    LED status_led_;
    Buzzer buzzer_;
    Button boot_button_;
    RingBuffer ring_buffer_;
    FlashStorage flash_storage_;
    BLEManager ble_manager_;

    // Services applicatifs.
    MonitoringService monitoring_service_{ina237_driver_, tmp126_driver_};
    AlarmService alarm_service_;
    EventLoggingService event_log_service_{ring_buffer_};
    ConfigService config_service_{flash_storage_, alarm_service_};

    // Etat courant de la FSM.
    AgvState current_state_ = AgvState::INIT;

    // Derniere mesure acquise.
    Measurement current_measurement_{};

    // Cadencement de la boucle principale.
    uint32_t last_loop_time_ms_ = 0U;
};
