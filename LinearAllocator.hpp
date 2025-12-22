#ifndef LINEAR_ALLOCATOR_HPP
#define LINEAR_ALLOCATOR_HPP

#include <cstddef>
#include <cstdlib>
#include <stdexcept>
#include <iostream>

// --- GESTOR DE MEMORIA COMPARTIDO ---
// Sacamos esto fuera del template para que sea común a todos los tipos de datos.
// (int, nodos de lista, vectores, etc. compartirán esta única arena).
struct LinearArena {
    static inline char* start = nullptr;
    static inline char* end = nullptr;
    static inline char* current = nullptr;
};

template <typename T>
struct LinearAllocator {
    using value_type = T;

    LinearAllocator() noexcept {}
    template <typename U> LinearAllocator(const LinearAllocator<U>&) noexcept {}
    template <typename U> struct rebind { using other = LinearAllocator<U>; };

    // --- INICIALIZACIÓN (Llamar una vez en main) ---
    static void init(std::size_t size_bytes) {
        if (LinearArena::start) {
            std::free(LinearArena::start);
        }
        
        // Pedimos la memoria al sistema operativo
        LinearArena::start = static_cast<char*>(std::malloc(size_bytes));
        if (!LinearArena::start) throw std::bad_alloc();

        LinearArena::end = LinearArena::start + size_bytes;
        LinearArena::current = LinearArena::start;
        
        // Opcional: Imprimir para confirmar
        // std::cout << "[LinearArena] Reservados " << size_bytes / 1024 << " KB\n";
    }

    // --- RESET ---
    static void reset() {
        LinearArena::current = LinearArena::start;
    }

    // --- ASIGNACIÓN ---
    T* allocate(std::size_t n) {
        std::size_t bytes_needed = n * sizeof(T);
        
        // Verificar si cabe en la Arena compartida
        if (LinearArena::current + bytes_needed > LinearArena::end) {
            throw std::bad_alloc();
        }

        char* user_ptr = LinearArena::current;
        LinearArena::current += bytes_needed; // Avanzamos el puntero global
        
        return reinterpret_cast<T*>(user_ptr);
    }

    // --- LIBERACIÓN ---
    void deallocate(T*, std::size_t) noexcept {
        // No hace nada. Se libera todo con reset().
    }
};

#endif