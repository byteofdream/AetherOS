# AetherOS Enhancements

## CPU Support (kernel/cpu.c)

Реализована поддержка CPUID инструкции для обнаружения процессора и его возможностей.

### Функции:
- `cpu_init()` - Инициализация и сбор информации о CPU
- `cpu_get_info()` - Получить информацию о процессоре
- `cpu_has_feature(cpu_feature_t)` - Проверить наличие конкретной возможности

### Поддерживаемые инструкции:
- ✓ FPU, MMX, SSE/SSE2, SSE3, SSE4.1/4.2
- ✓ AVX, AVX2
- ✓ AES-NI
- ✓ BMI1/BMI2
- ✓ TSC, PAE, PSE
- ✓ VMX, SMX (Virtual Machine Extension)

### Информация о CPU:
- Vendor (Intel, AMD, Cyrix и т.д.)
- Brand String (полное имя процессора)
- Stepping, Model, Family
- L2 Cache Size
- Наличие расширенных инструкций

## Graphics Enhancement (kernel/graphics.c + graphics.h)

Улучшена поддержка графики с добавлением определения видеокарты.

### Новые функции:
- `graphics_get_gpu_vendor()` - Получить производителя видеокарты
- `graphics_get_gpu_model()` - Получить модель видеокарты

### Поддерживаемые видеорежимы:
- VESA/VBE Framebuffer
- 32-bit color mode (ARGB)
- Double buffering для плавной анимации
- Hardware independent - работает на любых видеокартах с BIOS

## Shell Commands

Добавлены новые команды в shell для получения информации о системе:

### `sysinfo`
Выводит полную информацию о системе:
- OS Name и Architecture
- Bootloader информация
- Kernel Features (GDT, IDT, PIC, PIT, Scheduler, Memory, Filesystem, WM, ELF Loader)
- Статус различных компонентов

### `cpuinfo`
Информация о процессоре:
- CPU Architecture (x86_64)
- Текущий режим (Long Mode)
- Поддерживаемые инструкции (FPU, MMX, SSE/SSE2, PAE, TSC)

### `gpu`
Информация о видеокарте:
- Vendor (VESA)
- Driver (VBE Framebuffer)
- Rendering mode (Software)
- Buffering (Double-buffered)

## Userland Applications

### sysinfo.c - System Information Display
Красиво оформленная программа, выводящая информацию о системе (Neofetch-подобная):
```
╔═══════════════════════════════════╗
║        AetherOS System Info        ║
╚═══════════════════════════════════╝

┌─ Host
│  OS: AetherOS (x86_64)
│  Arch: x86_64 / Long Mode
├─ CPU
│  Cores: 1 (Single-threaded scheduler)
│  Mode: 64-bit
├─ Memory
│  Heap: Initialized
├─ Storage
│  Filesystem: RAMFS
├─ Graphics
│  Mode: VESA/VBE Framebuffer
│  Buffer: Double-buffered
└─ Devices
   2 PS/2 Controllers, ATA, PCI Bus
```

### demo.cpp - C++ Demo Application

Демонстрирует поддержку C++ в userland:

#### Реализованные возможности:
- **Classes and Objects** - Объектно-ориентированное программирование
- **String Management** - Custom String класс для работы со строками
- **Templates** - Template-based Vector<T> контейнер
- **Dynamic Memory** - Операторы new/delete
- **Operator Overloading** - Перегрузка операторов ([] и других)

#### Пример использования:
```cpp
String app_name("AetherOS");
Vector<int> numbers;
numbers.push_back(42);
numbers.push_back(100);
```

## C++ Runtime Support (userland/cxx_runtime.cpp)

Минимальная реализация C++ runtime для поддержки базовых возможностей:

### Реализовано:
- `operator new` / `operator delete` - Динамическое выделение памяти
- `operator new[]` / `operator delete[]` - Выделение массивов
- `__cxa_allocate_exception` / `__cxa_free_exception` - Exception handling
- `__cxa_guard_acquire` / `__cxa_guard_release` - Static initialization guards
- `__cxa_pure_virtual` - Pure virtual function handling
- Базовая поддержка RTTI (__class_type_info)

## Build System Updates

### Makefile изменения:
- Добавлен `kernel/cpu.c` в KERNEL_C_SRCS
- Добавлены `sysinfo.c` и `demo.cpp` в APP_SRCS
- Добавлены правила для компилирования .cpp файлов
- Новые ELF приложения загружаются в памяти на разных адресах:
  - `hello.elf` → 0x400000
  - `info.elf` → 0x500000
  - `sysinfo.elf` → 0x600000
  - `demo.elf` → 0x700000

## Hardware Compatibility

### Поддерживаемое оборудование:
- ✓ Intel: Core i3/i5/i7/i9 (Sandy Bridge и новее)
- ✓ AMD: Ryzen, EPYC
- ✓ NVIDIA: Доступно через VESA VBE (framebuffer)
- ✓ AMD Radeon: Доступно через VESA VBE (framebuffer)
- ✓ Intel iGPU: Доступно через VESA VBE (framebuffer)

### Требования:
- x86_64 процессор с поддержкой Long Mode
- BIOS с поддержкой VESA VBE
- Минимум 256 MB RAM
- Мультибутовский загрузчик (GRUB2 и выше)

## Integration Points

### Инициализация CPU (kernel/main.c):
```c
cpu_init();  // Вызывается после heap_init()
```

### Использование CPU информации:
```c
const cpu_info_t* info = cpu_get_info();
if (cpu_has_feature(CPU_FEATURE_AVX2)) {
    // Use AVX2 instructions
}
```

### Запуск программ:
```
run hello
run info
run sysinfo
run demo
```

## Future Enhancements

### Запланированные улучшения:
- [ ] CPUID кэширование результатов
- [ ] ACPI поддержка для Power Management
- [ ] MSR поддержка для CPU frequency scaling
- [ ] GPU DMA для ускоренного копирования памяти
- [ ] SVG renderer для векторной графики
- [ ] Full C++ std library support (стандартные контейнеры)
- [ ] Network stack (TCP/IP)
- [ ] Audio subsystem

## Building and Testing

### Компилирование:
```bash
make clean
make
```

### Запуск в QEMU:
```bash
make run
```

### Тестирование новых функций:
```
# Shell команды
sysinfo
cpuinfo  
gpu

# Запуск программ
run sysinfo
run demo
```
