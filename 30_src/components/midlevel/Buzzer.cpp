#include "Buzzer.hpp"
#include "../../../config/AppConfig.hpp"
#include "esp_log.h"

static const char* TAG = "Buzzer";

// =============================================================================
// Buzzer.cpp
// Pilotage via LEDC (PWM hardware ESP32-IDF)
// =============================================================================

Buzzer::Buzzer(uint8_t pin) : m_pin(pin) {}

Status Buzzer::init() {
    // --- Timer LEDC ---
    ledc_timer_config_t timerCfg = {};
    timerCfg.speed_mode      = LEDC_MODE;
    timerCfg.duty_resolution = LEDC_TIMER_10_BIT;
    timerCfg.timer_num       = LEDC_TIMER;
    timerCfg.freq_hz         = AppConfig::BUZZER_FREQ_WARNING_HZ;
    timerCfg.clk_cfg         = LEDC_AUTO_CLK;

    esp_err_t err = ledc_timer_config(&timerCfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ledc_timer_config FAILED: %s", esp_err_to_name(err));
        return Status::ERROR;
    }

    // --- Canal LEDC ---
    ledc_channel_config_t chanCfg = {};
    chanCfg.gpio_num   = static_cast<int>(m_pin);
    chanCfg.speed_mode = LEDC_MODE;
    chanCfg.channel    = LEDC_CHANNEL;
    chanCfg.timer_sel  = LEDC_TIMER;
    chanCfg.duty       = 0U;     // Muet au démarrage
    chanCfg.hpoint     = 0U;
    chanCfg.intr_type  = LEDC_INTR_DISABLE;

    err = ledc_channel_config(&chanCfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ledc_channel_config FAILED: %s", esp_err_to_name(err));
        return Status::ERROR;
    }

    toneOff();
    ESP_LOGI(TAG, "Buzzer init OK (GPIO%d, LEDC)", m_pin);
    return Status::OK;
}

void Buzzer::setMode(BuzzerMode mode) {
    if (m_mode == mode) { return; }

    m_mode            = mode;
    m_modeStartMs     = 0U;   // Sera mis à jour au premier update()
    m_lastToggleMs    = 0U;
    m_buzzOn          = false;
    m_critContinuDone = false;

    if (mode == BuzzerMode::SILENT) {
        toneOff();
    }
}

// =============================================================================
// update() — Appelé à chaque tick de la boucle principale
// =============================================================================
void Buzzer::update(uint32_t tickMs) {
    // Initialisation du timestamp de début de mode
    if (m_modeStartMs == 0U) {
        m_modeStartMs  = tickMs;
        m_lastToggleMs = tickMs;
    }

    switch (m_mode) {
        // ----------------------------------------------------------------
        case BuzzerMode::SILENT:
            toneOff();
            break;

        // ----------------------------------------------------------------
        // WARNING : intermittent lent — 500 Hz, 500ms ON / 1000ms OFF
        // ----------------------------------------------------------------
        case BuzzerMode::WARNING_SLOW: {
            const uint32_t elapsed = tickMs - m_lastToggleMs;
            if (m_buzzOn) {
                if (elapsed >= AppConfig::BUZZER_WARN_ON_MS) {
                    m_buzzOn       = false;
                    m_lastToggleMs = tickMs;
                    toneOff();
                }
            } else {
                if (elapsed >= AppConfig::BUZZER_WARN_OFF_MS) {
                    m_buzzOn       = true;
                    m_lastToggleMs = tickMs;
                    toneOn(AppConfig::BUZZER_FREQ_WARNING_HZ);
                }
            }
            break;
        }

        // ----------------------------------------------------------------
        // CRITICAL :
        //   Phase 1 — 2 sec continus à 2 kHz
        //   Phase 2 — intermittent rapide 300ms ON / 300ms OFF jusqu'au reset
        // ----------------------------------------------------------------
        case BuzzerMode::CRITICAL_ACTIVE: {
            const uint32_t totalElapsed = tickMs - m_modeStartMs;

            if (!m_critContinuDone) {
                // Phase 1 : continu
                if (totalElapsed == 0U) {
                    toneOn(AppConfig::BUZZER_FREQ_CRITICAL_HZ);
                }
                if (totalElapsed >= AppConfig::BUZZER_CRIT_CONTINUOUS_MS) {
                    m_critContinuDone = true;
                    m_lastToggleMs    = tickMs;
                    m_buzzOn          = false;
                    toneOff();
                }
            } else {
                // Phase 2 : intermittent rapide
                const uint32_t elapsed = tickMs - m_lastToggleMs;
                if (m_buzzOn) {
                    if (elapsed >= AppConfig::BUZZER_CRIT_ON_MS) {
                        m_buzzOn       = false;
                        m_lastToggleMs = tickMs;
                        toneOff();
                    }
                } else {
                    if (elapsed >= AppConfig::BUZZER_CRIT_OFF_MS) {
                        m_buzzOn       = true;
                        m_lastToggleMs = tickMs;
                        toneOn(AppConfig::BUZZER_FREQ_CRITICAL_HZ);
                    }
                }
            }
            break;
        }

        default:
            toneOff();
            break;
    }
}

// =============================================================================
// Reset — appelé lors du bouton reset pour stopper le buzzer critique
// =============================================================================
void Buzzer::reset() {
    setMode(BuzzerMode::SILENT);
    toneOff();
    ESP_LOGI(TAG, "Buzzer reset — mode SILENT");
}

// =============================================================================
// Privées
// =============================================================================

void Buzzer::toneOn(uint32_t freqHz) const {
    ledc_set_freq(LEDC_MODE, LEDC_TIMER, freqHz);
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, DUTY_50PCT);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

void Buzzer::toneOff() const {
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0U);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}
