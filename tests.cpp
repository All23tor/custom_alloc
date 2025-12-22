#include <chrono>
#include <iostream>
#include <list>
#include <memory>
#include <string_view>
#include <unistd.h>
#include <utility>
#include <vector>

#include "LinearAllocator.hpp"
#include "PoolAllocator.hpp"
#include "SegregatedAllocator.hpp"
#include "SimpleAllocator.hpp"

size_t obtener_uso_memoria_kb() {
  long rss = 0;
  FILE* f = fopen("/proc/self/statm", "r");
  if (!f)
    return 0;
  if (fscanf(f, "%*s %ld", &rss) != 1) {
    fclose(f);
    return 0;
  }
  fclose(f);
  return (size_t)rss * (size_t)sysconf(_SC_PAGESIZE) / 1024;
}

template <template <class> class AllocatorType>
struct MedirMetricasEstandar {
  void operator()(std::string_view nombre, int num_elementos) const {
    size_t ram_base = obtener_uso_memoria_kb();

    auto start = std::chrono::high_resolution_clock::now();
    auto end = start;
    double tiempo_alloc = 0;
    double tiempo_free = 0;
    size_t ram_usada = 0;

    {
      std::list<int, AllocatorType<int>> lista;

      start = std::chrono::high_resolution_clock::now();
      for (int i = 0; i < num_elementos; ++i) {
        lista.push_back(i);
      }
      end = std::chrono::high_resolution_clock::now();
      tiempo_alloc =
        std::chrono::duration<double, std::milli>(end - start).count();

      size_t ram_pico = obtener_uso_memoria_kb();
      ram_usada = (ram_pico > ram_base) ? (ram_pico - ram_base) : 0;

      start = std::chrono::high_resolution_clock::now();
      lista.clear();
      end = std::chrono::high_resolution_clock::now();
      tiempo_free =
        std::chrono::duration<double, std::milli>(end - start).count();
    }

    std::cout << nombre;
    if (nombre.length() < 8)
      std::cout << "\t";
    std::cout << "\t| Alloc: " << tiempo_alloc << " ms"
              << "\t| Free: " << tiempo_free << " ms"
              << "\t| RAM Usada: " << ram_usada << " KB" << std::endl;
  }
};

template <template <class> class AllocatorType>
struct PruebaVelocidadPura {
  void operator()(std::string_view nombre, int n) const {
    std::vector<int*> punteros;
    punteros.reserve(n);
    AllocatorType<int> alloc;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; ++i) {
      punteros.push_back(alloc.allocate(1));
    }
    auto end = std::chrono::high_resolution_clock::now();
    double t_alloc =
      std::chrono::duration<double, std::milli>(end - start).count();

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; ++i) {
      alloc.deallocate(punteros[i], 1);
    }
    end = std::chrono::high_resolution_clock::now();
    double t_free =
      std::chrono::duration<double, std::milli>(end - start).count();

    std::cout << nombre;
    if (nombre.length() < 8)
      std::cout << "\t";
    std::cout << "\t| Alloc Puro: " << t_alloc << " ms"
              << "\t| Free Puro: " << t_free << " ms" << std::endl;
  }
};

template <template <class> class AllocatorType>
struct PrubaEstresMemoria {
  void operator()(std::string_view nombre) const {
    std::cout << ">>> Probando: " << nombre << " <<<\n";
    size_t ram_inicial = obtener_uso_memoria_kb();

    for (int i = 1; i <= 5; ++i) {
      {
        std::list<int, AllocatorType<int>> lista;
        for (int j = 0; j < 50000; ++j) {
          lista.push_back(j);
        }
      }

      size_t ram_actual = obtener_uso_memoria_kb();
      long diferencia = (long)ram_actual - (long)ram_inicial;
      std::cout << "  Ciclo " << i << " | RAM Acumulada: " << diferencia
                << " KB\n";
    }
    std::cout << "--------------------------------\n";
  }
};

template <template <template <class> class> class Test, class... Args>
void test(Args&&... args) {
  LinearArena::reset();
  Test<std::allocator>{}("Standard", std::forward<Args>(args)...);
  Test<SimpleAllocator>{}("Simple", std::forward<Args>(args)...);
  Test<PoolAllocator>{}("Pool", std::forward<Args>(args)...);
  Test<LinearAllocator>{}("Linear", std::forward<Args>(args)...);
  Test<SegregatedAllocator>{}("Segregated", std::forward<Args>(args)...);
}

int main() {
  LinearArena::init(1024 * 1024 * 100);

  static constexpr int ELEMENTOS = 50000;
  std::cout << "\n=== 1. COMPARATIVA ESTANDAR (std::list) ===\n";
  test<MedirMetricasEstandar>(ELEMENTOS);
  std::cout << "\n=== 2. VELOCIDAD PURA (Sin overhead de lista) ===\n";
  test<PruebaVelocidadPura>(ELEMENTOS);
  std::cout << "\n=== 3. TEST DE ESTRES (Fugas de memoria) ===\n";
  test<PrubaEstresMemoria>();
}
