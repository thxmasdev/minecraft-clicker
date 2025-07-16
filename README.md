## 🎮 Minecraft Stealth Clicker

Un autoclicker avanzado y sigiloso diseñado específicamente para Minecraft con características anti-detección y funcionalidades de seguridad.

### ✨ Características Principales

#### 🖥️ Versión Consola (`minecraft_stealth_clicker.cpp`)
- **Sistema de detección inteligente**: Detecta automáticamente ventanas de Minecraft (incluyendo Lunar, Badlion, Forge, Fabric, OptiFine)
- **Algoritmo de CPS gaussiano**: Distribución natural de clicks por segundo para evitar detección
- **Jitter humano**: Micro-movimientos del cursor para simular comportamiento humano
- **Modo CLICK/HOLD**: Dos modos de operación diferentes
- **Clicks izquierdo/derecho**: Soporte para ambos botones del mouse
- **Auto-destrucción segura**: Eliminación completa del programa y rastros
- **Hotkeys globales**: Control total mediante teclas de acceso rápido
- **Interfaz de consola**: Información en tiempo real del estado y configuración

#### 🎨 Versión GUI (`minecraft_gui_clicker.cpp`)
- **Interfaz gráfica moderna**: Ventana elegante con tema oscuro y emojis
- **Funcionalidad completa**: Todas las características de la versión consola
- **Barras deslizantes**: Ajuste visual de CPS mínimo y máximo (1-50 CPS)
- **Indicadores en tiempo real**: Estado, CPS actual, tipo de click y modo
- **Tema oscuro atractivo**: Colores modernos y diseño profesional
- **Logo y branding**: Interfaz visualmente atractiva con ⚡ iconos
- **Mismos hotkeys**: Y, P, L, O, N para control completo
- **Detección de Minecraft**: Sistema idéntico a la versión consola

### 🎯 Controles y Hotkeys

#### Ambas Versiones (Consola y GUI):
- **Y**: Toggle ON/OFF del autoclicker
- **P**: Cambiar entre click izquierdo/derecho
- **L**: Cambiar entre modo CLICK/HOLD
- **O**: Abrir menú de configuración
- **N**: Auto-destrucción segura

## ⚙️ Configuración

Ambas versiones permiten configurar:
- **CPS objetivo**: 1-50 clicks por segundo
- **Porcentaje de jitter**: 0-100% para variación natural
- **Modo stealth**: Solo clic cuando Minecraft está activo
- **Randomización**: Patrones de clic variables

## 🔧 Compilación

### Versión Consola
```bash
g++ -O2 -std=c++17 minecraft_stealth_clicker.cpp -o minecraft_clicker.exe -luser32 -lkernel32 -ladvapi32
```

### Versión GUI (Recomendada)
```bash
g++ -O2 -std=c++17 -mwindows minecraft_gui_clicker.cpp -o minecraft_gui_clicker.exe -luser32 -lkernel32 -ladvapi32 -lcomctl32 -lshell32 -lgdi32
```

### Requisitos
- **Windows**: Sistema operativo compatible
- **MinGW/MSYS2**: Compilador GCC para Windows
- **C++17**: Estándar mínimo requerido

## 🚀 Ejecución

### Versión Consola
1. Ejecuta `minecraft_clicker.exe`
2. Configura CPS y opciones en el menú
3. Usa hotkeys para control:
   - **Y**: Toggle ON/OFF
   - **P**: Cambiar click izquierdo/derecho
   - **L**: Cambiar modo CLICK/HOLD
   - **O**: Configuración
   - **N**: Auto-destruir

### Versión GUI (Recomendada) ⭐
1. Ejecuta `minecraft_gui_clicker.exe`
2. **Interfaz moderna con tema oscuro**
3. **Barras deslizantes** para ajustar:
   - **CPS Mínimo**: 1-25 CPS
   - **CPS Máximo**: 1-50 CPS
4. **Indicadores en tiempo real**:
   - Estado (ACTIVO/INACTIVO)
   - CPS actual en tiempo real
   - Tipo de click (IZQUIERDO/DERECHO)
   - Modo (CLICK/HOLD)
5. **Control mediante botones o hotkeys**:
   - **🎯 TOGGLE (Y)**: Activar/desactivar
   - **🖱️ CLICK TYPE (P)**: Cambiar tipo de click
   - **⚙️ MODE (L)**: Cambiar modo
   - **🔧 SETTINGS (O)**: Configuración
   - **💥 SELF-DESTRUCT (N)**: Auto-destruir

## 🛡️ Características de Seguridad

- **Eliminación segura de memoria**: Limpia rastros en RAM
- **Auto-destrucción avanzada**: Elimina archivos de forma segura
- **Detección de ventana**: Solo actúa cuando Minecraft está activo
- **Patrones humanos**: Simula comportamiento natural de clic
- **Proceso oculto**: Operación discreta en segundo plano

## 📋 Requisitos del Sistema

- **OS**: Windows 7/8/10/11
- **Compilador**: MinGW-w64 o Visual Studio
- **RAM**: Mínimo 50MB
- **Permisos**: Administrador (recomendado para funciones avanzadas)

## ⚠️ Uso Responsable

Este software es **solo para fines educativos y de prueba**. 

- ❌ **NO usar en servidores públicos** (puede violar ToS)
- ✅ **Usar en mundos privados** o para pruebas
- ✅ **Respetar las reglas** de cada servidor
- ✅ **Uso ético** y responsable

## 👨‍💻 Autor

- **Discord**: thxmasdev
- **GitHub**: [thxmasdev/minecraft-clicker](https://github.com/thxmasdev/minecraft-clicker)

## 📄 Licencia

Este proyecto es de código abierto y está disponible bajo licencia MIT.