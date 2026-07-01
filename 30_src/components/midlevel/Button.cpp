#include "Button.hpp"
#include "esp_log.h"

static const char* TAG = "Button";

Button::Button(gpio_num_t pin) : m_pin(pin) {}

Status Button::init() {
    gpio_config_t cfg = {};
    cfg.pin_bit_mask = 1ULL << m_pin;
    cfg.mode         = GPIO_MODE_INPUT;
    cfg.pull_up_en   = GPIO_PULLUP_ENABLE;   // Actif bas
    cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    cfg.intr_type    = GPIO_INTR_DISABLE;

    esp_err_t err = gpio_config(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "GPIO config FAILED: %s", esp_err_to_name(err));
        return Status::ERROR;
    }

    ESP_LOGI(TAG, "Button init OK (GPIO%d, actif bas)", m_pin);
    return Status::OK;
}

void Button::update(uint32_t tickMs) {
    const bool currentState = (gpio_get_level(m_pin) == 1);

    if (m_lastState && !currentState) {
        // Front descendant — début d'appui
        m_pressStartMs = tickMs;
    } else if (!m_lastState && currentState) {
        // Front montant — relâchement
        const uint32_t duration = tickMs - m_pressStartMs;
        if (duration >= HOLD_MS) {
            m_pressedEvent = true;
            ESP_LOGI(TAG, "Reset button pressed (%lu ms)", duration);
        }
    }

    m_lastState = currentState;
}

bool Button::isResetPressed() {
    if (m_pressedEvent) {
        m_pressedEvent = false;
        return true;
    }
    return false;
}
