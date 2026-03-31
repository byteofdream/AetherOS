# 🚀 AetherOS - Краткая инструкция по новым возможностям

## Компилирование

```bash
cd /home/dozirod/Documents/OS
make clean          # Очистить старые файлы
make                # Собрать всё (ядро, драйверы, приложения)
make run            # Запустить в QEMU
```

## Взаимодействие в Shell

### 1️⃣ Просмотр информации о системе

```bash
$ sysinfo          # Красиво отформатированная информация о ОС
$ cpuinfo          # Информация о процессоре (x86_64, инструкции)
$ gpu              # Информация о видеокарте (VESA/VBE)
```

### 2️⃣ Запуск приложений

```bash
$ run hello        # Классическое "Hello from ELF app"
$ run info         # Минимальное приложение
$ run sysinfo      # Neofetch-подобная программа с красивым выводом
$ run demo         # C++ демонстрационная программа
```

### 3️⃣ Другие команды

```bash
$ ls               # Список файлов
$ pwd              # Текущая директория
$ cd <directory>   # Изменить директорию
$ cat <file>       # Выести содержимое файла
$ mem              # Памяти (в МБ)
$ uptime           # Время работы системы (в секундах)
$ mouse            # Состояние мыши (x, y, кнопки)
$ clear            # Очистить экран
```

## Архитектура системы

### Kernel Features ✅
- **x86_64 Long Mode** - 64-битный режим работы
- **GDT/IDT/PIC/PIT** - Управление памятью, прерываниями, таймер
- **Process Scheduler** - Планировщик задач
- **Memory Management** - Управление памятью (heap, paging)
- **RAMFS** - Файловая система в оперативной памяти
- **Window Manager** - Графический интерфейс
- **ELF Loader** - Загрузка и запуск программ
- **CPU Detection** - CPUID для определения процессора

### Graphics 🎨
- **VESA/VBE Framebuffer** - Универсальная поддержка видеокарт
- **32-bit Color Mode** - Поддержка ARGB
- **Double Buffering** - Плавная анимация без мерцания
- **Software Rendering** - Рисование на CPU
- **GPU Detection** - Определение типа видеокарты

### Devices 🖱️
- **PS/2 Keyboard** - Поддержка USB-клавиатур (через адаптер)
- **PS/2 Mouse** - Поддержка мыши
- **ATA Storage** - Поддержка жестких дисков
- **PCI Bus** - Сканирование устройств

### Userland 💻
- **Shell** - Интерпретатор команд
- **ELF Loader** - Запуск программ
- **C Applications** - Программы на C
- **C++ Support** - Базовая поддержка C++

## CPU Features детектирование

Система автоматически определяет поддерживаемые инструкции:

```
✓ FPU (Floating Point)        ✓ SSE/SSE2/SSE3
✓ MMX                          ✓ SSE4.1/SSE4.2
✓ AVX/AVX2                     ✓ AES-NI
✓ BMI1/BMI2                    ✓ PAE/PSE
✓ TSC (Time Stamp Counter)     ✓ VMX (Virtual Machine Extensions)
```

## C++ Поддержка

### Реализовано:
```cpp
// Классы и объекты
class String { ... };

// Конструкторы & Деструкторы
String::String(const char *str) { ... }

// Методы
const char* c_str() const { return buffer; }

// Const correctness
String::length() const { return len; }

// Использование
String name("AetherOS");
name.append(" System");
```

### Ограничения:
- ❌ Нет исключений (exceptions disabled)
- ❌ Нет RTTI (type_info)
- ❌ Нет STL контейнеров
- ⚠️ Минимальная поддержка глобальных объектов

## Примеры программ

### C Программа (hello.c)
```c
void app_entry(const app_syscalls_t *syscalls) {
    syscalls->write("Hello from ELF app\n");
}
```

### C++ Программа (demo.cpp)
```cpp
class String {
    char buffer[256];
    size_t len;
public:
    String(const char *str) { ... }
    const char* c_str() const { return buffer; }
};

extern "C" void app_entry(const app_syscalls_t *syscalls) {
    String greeting("Welcome to AetherOS!");
    syscalls->write(greeting.c_str());
}
```

## Файлы приложений

```
userland/apps/
├── hello.c        - Простое C приложение
├── info.c         - Информация о системе (C)
├── sysinfo.c      - Neofetch-подобная программа (C)
├── demo.cpp       - Демонстрация C++ (Classes, Objects)
└── app.ld         - Linker script для ELF программ
```

## Загрузка адреса ELF программ

```
hello.elf   → 0x400000
info.elf    → 0x500000  
sysinfo.elf → 0x600000
demo.elf    → 0x700000
```

## Требования

**Для компилирования:**
- gcc (64-bit)
- nasm (assembler)
- ld (linker)
- grub-mkrescue (для ISO)

**Для запуска:**
- QEMU (qemu-system-x86_64)
- >= 256 MB RAM
- BIOS с VESA VBE поддержкой

## Отладка

### Просмотр сборки:
```bash
ls -la build/kernel.elf              # Размер ядра
ls -la build/userland/*.elf          # Размеры приложений
file build/aetheros.iso              # Информация об ISO
```

### Результат в QEMU:
```
[BOOT] loader: GRUB / Multiboot2
[BOOT] cpu mode: x86_64 long mode
[BOOT] interrupts: IDT / PIC / PIT online
[BOOT] filesystem: RAMFS mounted
[BOOT] input: PS/2 keyboard + mouse
[BOOT] services: window manager armed
[SHELL] aether:/ $ _
```

## Troubleshooting

**Проблема:** `make: Not found command`
- **Решение:** `sudo apt install build-essential nasm grub-pc-bin xorriso qemu qemu-system-x86`

**Проблема:** Видеокарта не определяется
- **Решение:** VESA VBE всегда доступна - это fallback для всех видеокарт

**Проблема:** Программа не запускается
- **Проверка:** `run hello` даст более подробное сообщение об ошибке

**Проблема:** C++ программа падает
- **Причина:** Требуется полная C++ runtime для new/delete
- **Решение:** Использовать стек-выделение (stack allocation)

---

**Happy coding! 🎉 Система готова к использованию и расширению!**
