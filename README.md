# Cornering Light System - Yamaha XT1200

Sistema inteligente de luces de giro para motocicleta que utiliza un acelerómetro MPU6050 para detectar la inclinación y controlar automáticamente las luces LED de giro izquierdo/derecho.

## 🚀 Características

- **Detección automática de inclinación**: Usa acelerómetro MPU6050 para detectar ángulos de giro
- **Control PWM suave**: Fade graduado entre niveles de brillo
- **Interfaz WiFi de configuración**: Access Point con página web interactiva
- **Calibración del acelerómetro**: Endpoint para calibrar offsets
- **Almacenamiento persistente**: Configuración guardada en EEPROM
- **Filtrado digital**: Filtro exponencial para estabilizar lecturas
- **Máquina de estados**: Gestión clara de estados del sistema (INIT → CONFIG_WINDOW → NORMAL → ERROR)
- **Debug remoto**: Endpoint `/debug` con telemetría en tiempo real
- **Arquitectura modular**: Código dividido en componentes reutilizables

## 📋 Requisitos de Hardware

- **Wemos D1 mini (ESP8266)**: Microcontrolador principal
  - CPU: 80MHz Xtensa
  - RAM: 160KB
  - Flash: 4MB
  - I2C, WiFi, PWM integrados

- **MPU6050**: Acelerómetro de 6 ejes
  - I2C interface
  - Rango aceleración: ±16g
  - Resolución: 16-bit

- **LEDs y transistores**:
  - LED izquierdo en GPIO 14 (D5, PWM)
  - LED derecho en GPIO 12 (D6, PWM)
  - Transistores para conducir los LEDs (si aplica)
  - Resistencias de limitación de corriente

- **Alimentación**:
  - 5V para Wemos D1 mini (entrada Micro-USB)
  - Voltaje de LEDs según especificación

## 🛠️ Requisitos de Software

- **PlatformIO Core**: Gestor de plataformas y bibliotecas
- **Arduino Framework**: Para espressif8266
- **Dependencias**:
  - Adafruit MPU6050 @ 2.2.6
  - Adafruit Unified Sensor @ 1.1.15
  - EEPROM, Wire, ESP8266WiFi, ESP8266WebServer (incluidas en framework)

## ⚙️ Instalación

### 1. Clonar o descargar el repositorio

```bash
git clone https://github.com/stirner/cuneteras-xt1200.git
cd cuneteras-xt1200
```

### 2. Instalar PlatformIO

Si no lo tienes instalado:
```bash
pip install platformio
```

### 3. Compilar el proyecto

```bash
platformio run --environment d1_mini
```

### 4. Cargar en el dispositivo

```bash
platformio run --target upload --environment d1_mini
```

**Nota**: Asegúrate de que la placa está conectada a COM5 (o ajusta `upload_port` en `platformio.ini` según tu puerto serial).

### 5. Monitorear salida serial

```bash
platformio device monitor
```

## 🔧 Configuración

### Parámetros configurable via WiFi

Conecta a la red WiFi **"CorneringLight"** (contraseña: **12345678**)

Abre navegador a: `http://192.168.4.1`

**Parámetros disponibles:**

| Parámetro | Tipo | Rango | Descripción |
|-----------|------|-------|-------------|
| Angle ON | float | 0-90° | Ángulo a partir del cual se enciende la luz |
| Angle OFF | float | 0-90° | Ángulo de desactivación (debe ser < Angle ON) |
| Max PWM | int | 0-1023 | Brillo máximo de los LEDs |
| Filter Alpha | float | 0-1 | Factor de filtro exponencial (0=sin filtro, 1=máximo filtro) |
| Fade Step | int | 1-50 | Velocidad de transición entre brillo (pasos por ciclo) |

### Valores por defecto

```cpp
Angle ON:      10.0°
Angle OFF:     7.0°
Max PWM:       800
Filter Alpha:  0.05
Fade Step:     5
```

## 🌐 API REST

### Endpoints disponibles

#### GET `/`
Interfaz web de configuración
- **Respuesta**: HTML con formulario de configuración

#### GET `/save`
Guardar configuración
- **Parámetros**:
  - `aon`: Angle ON (float)
  - `aoff`: Angle OFF (float)
  - `pwm`: Max PWM (int)
  - `alpha`: Filter Alpha (float)
  - `fade`: Fade Step (int)
- **Respuesta**: HTML confirmación + cierre WiFi

#### GET `/status`
Obtener estado actual (JSON)
```json
{
  "angleOn": 10.0,
  "angleOff": 7.0,
  "maxPWM": 800,
  "filterAlpha": 0.05,
  "fadeStep": 5
}
```

#### GET `/debug`
Telemetría del sistema (JSON)
```json
{
  "uptime_ms": 45230,
  "free_heap": 120000,
  "roll_raw": 12.45,
  "roll_filtered": 12.23,
  "pwm_left": 0,
  "pwm_right": 350,
  "clients_connected": 1
}
```

#### GET `/reset`
Reset a valores por defecto
- **Respuesta**: HTML confirmación + cierre WiFi

#### GET `/calibrate`
Calibrar offset del acelerómetro
- **Parámetros**:
  - `ax`: Offset acelerometro X
  - `ay`: Offset acelerometro Y
  - `az`: Offset acelerometro Z

## 📱 Flujo de Operación

### Estado: INIT (Inicialización)
- Inicializa EEPROM, I2C, MPU6050, PWM, WiFi
- Transición a CONFIG_WINDOW

### Estado: CONFIG_WINDOW (30 segundos o hasta guardar)
- **Sin clientes**: Cierra WiFi después de 30 segundos → NORMAL
- **Con cliente conectado**: WiFi permanece activa indefinidamente
- **Al guardar config**: Expulsa cliente y cierra WiFi → NORMAL

### Estado: NORMAL (Operación)
- Lectura continua del MPU6050
- Actualización de PWM basada en ángulo
- Debug cada 5 segundos en serial
- Mantiene valores de configuración de EEPROM

### Estado: ERROR
- Sistema en espera si falla inicialización crítica

## 📂 Estructura del Proyecto

```
cuneteras-xt1200/
├── platformio.ini                 # Configuración de PlatformIO
├── README.md                      # Este archivo
│
├── include/
│   ├── config.h                   # Constantes y estructura Config
│   ├── config_manager.h           # Gestor de EEPROM
│   ├── mpu_handler.h              # Interfaz acelerómetro
│   ├── light_controller.h         # Control de LEDs PWM
│   ├── wifi_server.h              # Servidor WiFi (APConfigServer)
│   └── system_state.h             # Máquina de estados
│
├── src/
│   ├── main.cpp                   # Punto de entrada y loop principal
│   ├── config_manager.cpp         # Implementación gestor EEPROM
│   ├── mpu_handler.cpp            # Implementación acelerómetro
│   ├── light_controller.cpp       # Implementación control LEDs
│   ├── wifi_server.cpp            # Implementación servidor WiFi
│   └── system_state.cpp           # Implementación máquina de estados
│
├── lib/
│   └── README
│
└── test/
    └── README
```

## 🔌 Pines GPIO

```
GPIO 14 (D5)  ← LED Izquierdo (PWM)
GPIO 12 (D6)  ← LED Derecho (PWM)
GPIO 4  (D2)  ← SDA (I2C) MPU6050
GPIO 5  (D1)  ← SCL (I2C) MPU6050
```

**Referencia Wemos D1 mini:**
- D0 = GPIO16, D1 = GPIO5, D2 = GPIO4, D3 = GPIO0, D4 = GPIO2, D5 = GPIO14, D6 = GPIO12, D7 = GPIO13, D8 = GPIO15

## 📊 Cálculo de Ángulo

El ángulo de inclinación se calcula usando el acelerómetro:

```cpp
roll = atan2(ay, az) * 57.2958  // radianes a grados
```

- Rango válido: ±90°
- Filtrado exponencial: `filtered = alpha * raw + (1-alpha) * filtered_prev`
- Mapeo PWM: `pwm = map(roll, angleOn, 45°, 0, maxPWM)`

## 🐛 Debug y Diagnóstico

### Serial Output (115200 baud)

Monitorea la salida serial para debugging:

```
[STATE] Transition: INIT -> CONFIG_WINDOW
[WiFi] AP started. Connect to: CorneringLight IP: 192.168.4.1
[WiFi] Client connected - timeout disabled
[LOOP] [MPU] Raw Roll: 5.23° | Filtered Roll: 5.18° | Offsets: X=0.000 Y=0.000 Z=0.000
[LIGHTS] Left PWM: 0/0 | Right PWM: 245/340 | Fade Step: 5
```

### Endpoint /debug

Consulta el estado del sistema en tiempo real:

```bash
curl http://192.168.4.1/debug
```

## 🚀 Mejoras Implementadas

✅ **Separación modular**: Código dividido en 6 componentes independientes
✅ **Máquina de estados**: Gestión clara del ciclo de vida
✅ **Timeout WiFi inteligente**: 30s sin cliente, infinito con cliente
✅ **Configuración persistente**: EEPROM con validación
✅ **Calibración remota**: Offset del acelerómetro vía endpoint
✅ **Fade configurable**: Velocidad de transición ajustable
✅ **Protección de valores**: Límites ±90°, validación angelOff < angleOn
✅ **HTML optimizado**: Almacenado en PROGMEM para ahorrar RAM
✅ **Debug remoto**: Telemetría completa vía `/debug`
✅ **Logging mejorado**: Mensajes informativos con timestamps

## 📝 Licencia

MIT License - Libre para usar, modificar y distribuir

## 👤 Autor

Pedro Clemente

## 🔗 Links

- [Documentación ESP8266](https://arduino-esp8266.readthedocs.io/)
- [Wemos D1 mini](https://www.wemos.cc/en/latest/d1/d1_mini.html)
- [Adafruit MPU6050](https://github.com/adafruit/Adafruit_MPU6050)
- [PlatformIO](https://platformio.org/)
- [Troubleshooting esptool](https://docs.espressif.com/projects/esptool/en/latest/troubleshooting.html)

---

**Última actualización**: Enero 2026
