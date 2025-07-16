# Minecraft AutoClicker

## Descripción
Un autoclicker sigiloso diseñado específicamente para Minecraft con interfaz colorida y funciones avanzadas.

## Características
- **Dos modos de funcionamiento:**
  - **CLICK**: Autoclick solo mientras mantienes presionado el botón del mouse
  - **HOLD**: Autoclick continuo sin necesidad de presionar botón
- **CPS configurables** con distribución gaussiana para mayor naturalidad
- **Interfaz colorida** con códigos ANSI
- **Detección de ventana activa** para mayor seguridad
- **Jitter humano** para evitar detección
- **Auto-destrucción** para mayor discreción

## Controles
- **Y** - Activar/Desactivar autoclicker
- **P** - Cambiar tipo de click (Izquierdo/Derecho)
- **L** - Alternar modo (CLICK/HOLD)
- **O** - Abrir configuración de CPS
- **N** - Auto-destrucción

## Compilación
```bash
g++ -O2 -static -std=c++17 -o minecraft_clicker.exe minecraft_stealth_clicker.cpp -luser32 -lkernel32
```

## Uso
1. Ejecuta el programa
2. Configura los CPS mínimos y máximos con la tecla **O**
3. Selecciona el modo deseado con **L**
4. Activa el autoclicker con **Y**
5. En modo CLICK: mantén presionado el botón del mouse
6. En modo HOLD: el autoclicker funcionará automáticamente

## Configuración por defecto
- CPS mínimo: 8
- CPS máximo: 12
- Modo: CLICK
- Tipo de click: Izquierdo

## Créditos
Creado por **Thomàs**  
Discord: **thxmasdev**

## Advertencia
Este software es solo para fines educativos. El uso de autoclickers puede violar los términos de servicio de algunos juegos. Úsalo bajo tu propia responsabilidad.