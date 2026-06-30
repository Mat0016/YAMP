EnergyManager.cpp

#include "EnergyManager.hpp"

void EnergyManager::process() {
    // 1. Lecture des capteurs (Mid-Level)
    float32_t current   = m_powerMonitor.getCurrent();
    float32_t ntcTemp   = m_tempMonitor.getNtcTemp();
    float32_t tmpTemp   = m_tempMonitor.getTmpSensorTemp();
    bool      isReset   = m_hmi.isResetPressed();

    // 2. Gestionnaire d'État (FSM)
    switch (m_currentState) {
        case AgvState::INIT:
            if ((m_powerMonitor.configure() == Status::OK) && (m_tempMonitor.init() == Status::OK)) {
                m_hmi.setNotification(HmiMode::SUCCESS_BEEP);
                m_currentState = AgvState::NORMAL;
            } else {
                m_currentState = AgvState::CRITICAL; // Échec d'initialisation des registres
            }
            break;

        case AgvState::NORMAL:
            m_hmi.setNotification(HmiMode::LED_GREEN_ON);
            
            // Évaluation des seuils (Fonction privée)
            m_currentState = evaluateThresholds(current, ntcTemp, tmpTemp);
            
            if (isReset) {
                m_currentState = AgvState::INIT; // Demande de reboot manuel
            }
            break;

        case AgvState::WARNING:
            m_hmi.setNotification(HmiMode::LED_ORANGE_BLINK_BUZZER_SLOW);
            m_currentState = evaluateThresholds(current, ntcTemp, tmpTemp);
            break;

        case AgvState::CRITICAL:
            m_hmi.setNotification(HmiMode::LED_RED_ON_BUZZER_FAST);
            triggerSafetyShutdown(); // Action immédiate de coupure
            m_currentState = AgvState::SAFE_SHUTDOWN;
            break;

        case AgvState::SAFE_SHUTDOWN:
            // Le système est sécurisé, on attend un reset physique ou une commande externe
            if (isReset) {
                m_currentState = AgvState::INIT;
            }
            break;

        default:
            // Règle MISRA impérative : Sécurité défensive si l'état est corrompu en mémoire
            m_currentState = AgvState::CRITICAL;
            break;
    }
}


// Constantes de sécurité (Seuils) définies proprement pour MISRA
namespace ThermalThresholds {
    constexpr float32_t CRITICAL_TEMP = 65.0f; // Température critique en °C
    constexpr float32_t WARNING_TEMP  = 50.0f; // Température d'alerte en °C
    constexpr float32_t HYSTERESIS    = 5.0f;  // Hystérésis pour revenir à la normale
}

namespace PowerThresholds {
    constexpr float32_t CRITICAL_CURRENT = 10.0f; // Courant max en Ampères
}

// Constructeur : On démarre obligatoirement en état INIT
EnergyManager::EnergyManager() : m_currentState(AgvState::INIT) {}

// ==========================================
// FONCTIONS PUBLIQUES
// ==========================================

/**
 * @brief Initialise la FSM à INIT et configure tous les registres des périphériques.
 * @return Status::OK si tout est configuré, Status::ERROR sinon.
 */
Status EnergyManager::init() {
    Status result = Status::OK;
    
    // Forcer le retour à l'état INIT
    m_currentState = AgvState::INIT;

    // Initialisation des registres du capteur de courant (INA237)
    Status powerStatus = m_powerMonitor.configure();
    
    // Initialisation des capteurs de température (CTN + TMP)
    Status tempStatus = m_tempMonitor.init();

    if ((powerStatus != Status::OK) || (tempStatus != Status::OK)) {
        result = Status::ERROR;
        m_currentState = AgvState::CRITICAL; // Échec matériel grave direct
    } else {
        m_hmi.setNotification(HmiMode::SUCCESS_BEEP);
        m_currentState = AgvState::NORMAL;  // Tout est OK, on commence la surveillance
    }

    return result;
}

/**
 * @brief Boucle principale de la FSM (Machine à états). Gère les transitions.
 */
void EnergyManager::process() {
    // 1. Récupération des données physiques des capteurs (Mid-Level)
    const float32_t current   = m_powerMonitor.getCurrent();
    const float32_t ntcTemp   = m_tempMonitor.getNtcTemp();
    const float32_t tmpTemp   = m_tempMonitor.getTmpSensorTemp();
    const bool      isReset   = m_hmi.isResetPressed();

    // 2. Traitement de la FSM
    switch (m_currentState) {
        case AgvState::INIT:
            // Appel de la fonction d'initialisation
            (void)init(); 
            break;

        case AgvState::NORMAL:
            // Surveillance nominale : on évalue les seuils
            m_currentState = checkThresholds(current, ntcTemp, tmpTemp);
            
            if (isReset) {
                m_currentState = AgvState::INIT; // Demande de reboot manuel via le bouton
            }
            break;

        case AgvState::WARNING:
            // Mode alerte : on continue de surveiller si ça empire ou si ça redescend
            m_currentState = checkThresholds(current, ntcTemp, tmpTemp);
            
            if (isReset) {
                m_currentState = AgvState::INIT;
            }
            break;

        case AgvState::CRITICAL:
            // Action immédiate de protection
            triggerSafetyShutdown();
            m_currentState = AgvState::SAFE_SHUTDOWN;
            break;

        case AgvState::SAFE_SHUTDOWN:
            // Le système est figé en sécurité. Seul le bouton Reset peut nous relancer.
            if (isReset) {
                m_currentState = AgvState::INIT;
            }
            break;

        default:
            // Règle MISRA impérative : Sécurité défensive si l'état mémoire est corrompu
            m_currentState = AgvState::CRITICAL;
            break;
    }

    // 3. Mise à jour des LED et du Buzzer en fonction du nouvel état
    updateHmi();
}

// ==========================================
// FONCTIONS PRIVÉES
// ==========================================

/**
 * @brief Compare les valeurs issues des capteurs avec les seuils de la datasheet.
 * @return Le prochain état logique (AgvState) de la FSM.
 */
AgvState EnergyManager::checkThresholds(float32_t current, float32_t ntcTemp, float32_t tmpTemp) {
    AgvState nextState = m_currentState;

    // Détermination de la température la plus haute par sécurité (Redondance CTN / TMP)
    float32_t maxTemp = ntcTemp;
    if (tmpTemp > ntcTemp) {
        maxTemp = tmpTemp;
    }

    // 1. Analyse des seuils Critiques (Surchauffe ou Court-circuit)
    if ((maxTemp >= ThermalThresholds::CRITICAL_TEMP) || (current >= PowerThresholds::CRITICAL_CURRENT)) {
        nextState = AgvState::CRITICAL;
    }
    // 2. Analyse des seuils d'Alerte (Warning)
    else if (maxTemp >= ThermalThresholds::WARNING_TEMP) {
        nextState = AgvState::WARNING;
    }
    // 3. Retour à la normale avec Hystérésis (Évite les oscillations de la FSM)
    else if (m_currentState == AgvState::WARNING) {
        if (maxTemp < (ThermalThresholds::WARNING_TEMP - ThermalThresholds::HYSTERESIS)) {
            nextState = AgvState::NORMAL;
        }
    }
    else {
        // On reste dans l'état actuel (NORMAL)
        nextState = AgvState::NORMAL;
    }

    return nextState;
}

/**
 * @brief Pilote les retours visuels (LED) et sonores (Buzzer) selon l'état actuel de la FSM.
 */
void EnergyManager::updateHmi() {
    switch (m_currentState) {
        case AgvState::INIT:
            // Pendant l'init, HMI gérée directement dans init() ou éteinte
            break;

        case AgvState::NORMAL:
            m_hmi.setNotification(HmiMode::LED_GREEN_ON);
            break;

        case AgvState::WARNING:
            m_hmi.setNotification(HmiMode::LED_ORANGE_BLINK_BUZZER_SLOW);
            break;

        case AgvState::CRITICAL:
        case AgvState::SAFE_SHUTDOWN:
            m_hmi.setNotification(HmiMode::LED_RED_ON_BUZZER_FAST);
            break;

        default:
            // Sécurité HMI par défaut
            m_hmi.setNotification(HmiMode::LED_RED_ON_BUZZER_FAST);
            break;
    }
}

/**
 * @brief Coupe l'alimentation des moteurs ou isole la batterie de l'AGV.
 */
void EnergyManager::triggerSafetyShutdown() {
    // Insérer ici l'appel vers ton GpioDriver pour couper les relais de puissance.
    // Ex: m_gpio.writePin(PIN_RELAIS_PUISSANCE, false);
}