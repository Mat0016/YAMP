#include "LED.hpp"
#include "esp_log.h"

static const char* TAG = "LED";

// =============================================================================
// LED.cpp — LED bicolore (Verte / Rouge)
// =============================================================================

LED::LED(gpio_num_t greenPin, gpio_num_t redPin)
    : m_greenPin(greenPin)
    , m_redPin(redPin)
{}

Status LED::init() {
    // Configuration des GPIOs en sortie
    gpio_config_t cfg = {};
    cfg.pin_bit_mask = (1ULL << m_greenPin) | (1ULL << m_redPin);
    cfg.mode         = GPIO_MODE_OUTPUT;
    cfg.pull_up_en   = GPIO_PULLUP_DISABLE;
    cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    cfg.intr_type    = GPIO_INTR_DISABLE;

    esp_err_t err = gpio_config(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "GPIO config FAILED: %s", esp_err_to_name(err));
        return Status::ERROR;
    }

    allOff();
    ESP_LOGI(TAG, "LED init OK (GREEN=GPIO%d, RED=GPIO%d)", m_greenPin, m_redPin);
    return Status::OK;
}

void LED::setMode(LedMode mode) {
    if (m_mode == mode) { return; }
    m_mode = mode;
    m_blinkState   = false;
    m_lastToggleMs = 0U;

    // Application immédiate de l'état statique
    switch (m_mode) {
        case LedMode::OFF:
            allOff();
            break;
        case LedMode::GREEN_STEADY:
            setGreen(true);
            setRed(false);
            break;
        case LedMode::RED_STEADY:
            setGreen(false);
            setRed(true);
            break;
        case LedMode::RED_BLINK:
            // Le clignotement est géré dans update()
            allOff();
            break;
        default:
            allOff();
            break;
    }
}

void LED::update(uint32_t tickMs) {
    if (m_mode != LedMode::RED_BLINK) { return; }

    if ((tickMs - m_lastToggleMs) >= BLINK_PERIOD_MS) {
        m_lastToggleMs = tickMs;
        m_blinkState   = !m_blinkState;
        setRed(m_blinkState);
        setGreen(false);
    }
}

// ---------------------------------------------------------------------------
// Privées
// ---------------------------------------------------------------------------

void LED::setGreen(bool on) const {
    gpio_set_level(m_greenPin, on ? 1 : 0);
}

void LED::setRed(bool on) const {
    gpio_set_level(m_redPin, on ? 1 : 0);
}

void LED::allOff() const {
    setGreen(false);
    setRed(false);
}
