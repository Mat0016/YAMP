#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>

// =============================================================================
// RingBuffer.hpp — Buffer circulaire générique (header-only template)
//
// Utilisé pour stocker les LogEntry en PSRAM.
// Thread-safe par disable d'interruptions si utilisé depuis une seule tâche.
// La mémoire tampon est allouée en PSRAM via PSRAMManager.
// =============================================================================

template<typename T, std::size_t Capacity>
class RingBuffer {
public:
    RingBuffer() : m_head(0U), m_tail(0U), m_count(0U) {}

    // Ajoute un élément (écrase le plus ancien si plein)
    void push(const T& item) {
        m_buffer[m_head] = item;
        m_head = (m_head + 1U) % Capacity;

        if (m_count < Capacity) {
            ++m_count;
        } else {
            // Buffer plein : on écrase le plus ancien
            m_tail = (m_tail + 1U) % Capacity;
        }
    }

    // Lit et retire l'élément le plus ancien
    bool pop(T& out) {
        if (m_count == 0U) { return false; }
        out    = m_buffer[m_tail];
        m_tail = (m_tail + 1U) % Capacity;
        --m_count;
        return true;
    }

    // Lecture sans retrait (peek)
    bool peek(T& out) const {
        if (m_count == 0U) { return false; }
        out = m_buffer[m_tail];
        return true;
    }

    std::size_t count()    const { return m_count;            }
    bool        isEmpty()  const { return m_count == 0U;      }
    bool        isFull()   const { return m_count == Capacity; }
    std::size_t capacity() const { return Capacity;            }

    void clear() {
        m_head  = 0U;
        m_tail  = 0U;
        m_count = 0U;
    }

private:
    T           m_buffer[Capacity];
    std::size_t m_head;
    std::size_t m_tail;
    std::size_t m_count;
};
