#include "Buzzer.hpp"

static constexpr uint8_t  LEDC_RESOLUTION_BITS = 8U;
static constexpr uint32_t LEDC_DUTY_50PCT      = 128U; // 50 % duty

void Buzzer::init() {
    ledcSetup(LEDC_CHANNEL, BUZZER_FREQ_WARNING_HZ, LEDC_RESOLUTION_BITS);
    ledcAttachPin(PIN_BUZZER, LEDC_CHANNEL);
    toneOff();
}

void Buzzer::setPattern(BuzzerPattern pattern) {
    m_pattern    = pattern;
    m_lastToggle = millis();
    m_buzzerOn   = false;

    switch (pattern) {
        case BuzzerPattern::SUCCESS_BEEP:
            m_beepStart = millis();
            toneOn(BUZZER_FREQ_WARNING_HZ);
            m_buzzerOn = true;
            break;

        case BuzzerPattern::CRITICAL_START:
            m_criticalStart = millis();
            toneOn(BUZZER_FREQ_CRITICAL_HZ);
            m_buzzerOn = true;
            break;

        case BuzzerPattern::OFF:
            toneOff();
            break;

        default:
            break;
    }
}

void Buzzer::stop() {
    m_pattern = BuzzerPattern::OFF;
    toneOff();
    m_buzzerOn = false;
}

// ============================================================
//  update() — machine à états non-bloquante (pas de delay())
// ============================================================
void Buzzer::update() {
    uint32_t now = millis();

    switch (m_pattern) {

        case BuzzerPattern::SUCCESS_BEEP:
            if (m_buzzerOn && (now - m_beepStart >= SUCCESS_DURATION_MS)) {
                toneOff();
                m_pattern  = BuzzerPattern::OFF;
                m_buzzerOn = false;
            }
            break;

        case BuzzerPattern::WARNING_SLOW:
        {
            uint32_t onDur  = m_buzzerOn ? WARNING_ON_MS : WARNING_OFF_MS;
            if (now - m_lastToggle >= onDur) {
                m_lastToggle = now;
                m_buzzerOn   = !m_buzzerOn;
                if (m_buzzerOn) { toneOn(BUZZER_FREQ_WARNING_HZ); }
                else            { toneOff(); }
            }
            break;
        }

        case BuzzerPattern::CRITICAL_START:
            // Phase 1 : 2 s continu
            if (now - m_criticalStart >= CRITICAL_CONT_MS) {
                // Passage automatique en phase 2
                m_pattern    = BuzzerPattern::CRITICAL_BEEP;
                m_lastToggle = now;
                m_buzzerOn   = true;
                toneOn(BUZZER_FREQ_CRITICAL_HZ);
            }
            break;

        case BuzzerPattern::CRITICAL_BEEP:
        {
            // Phase 2 : intermittent rapide, indéfini jusqu'au reset
            uint32_t dur = m_buzzerOn ? CRITICAL_ON_MS : CRITICAL_OFF_MS;
            if (now - m_lastToggle >= dur) {
                m_lastToggle = now;
                m_buzzerOn   = !m_buzzerOn;
                if (m_buzzerOn) { toneOn(BUZZER_FREQ_CRITICAL_HZ); }
                else            { toneOff(); }
            }
            break;
        }

        case BuzzerPattern::OFF:
        default:
            break;
    }
}

// ============================================================
//  Primitives ledc
// ============================================================
void Buzzer::toneOn(uint32_t freqHz) {
    ledcSetup(LEDC_CHANNEL, freqHz, LEDC_RESOLUTION_BITS);
    ledcWrite(LEDC_CHANNEL, LEDC_DUTY_50PCT);
}

void Buzzer::toneOff() {
    ledcWrite(LEDC_CHANNEL, 0U);
}
