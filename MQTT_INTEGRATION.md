# MQTT Інтеграція з Home Assistant

## Опис

Проєкт Girlianda підтримує інтеграцію з Home Assistant через MQTT. Це дозволяє керувати двома гірляндами (A та B) через Home Assistant з можливістю зміни режимів роботи, яскравості та швидкості анімації.

## Налаштування

### 1. MQTT Брокер

Переконайтеся, що у вас встановлений та запущений MQTT брокер. Рекомендується використовувати Mosquitto.

### 2. Конфігурація в secrets.h

Додайте MQTT налаштування у файл `secrets.h`:

```cpp
// MQTT налаштування
inline const char* mqttHosts[] = {"mqtt.lan", "192.168.0.120"};
inline const int mqttHostCount = sizeof(mqttHosts) / sizeof(mqttHosts[0]);
```

Ви можете вказати кілька хостів для fallback - якщо перший хост недоступний, ESP32 автоматично спробує підключитися до наступного.

### 3. Home Assistant

MQTT інтеграція використовує Home Assistant Discovery, тому пристрій автоматично з'явиться в Home Assistant після першого підключення.

## Сутності в Home Assistant

Для кожної гірлянди (A та B) створюються наступні сутності:

### Гірлянда A

- **select.girlianda_a_rezhim** - Вибір режиму роботи
  - Постійне
  - Почергове
  - Дихання
  - Хаос
  - Свічка

- **number.girlianda_a_iaskravist** - Яскравість (0-255)

- **number.girlianda_a_shvidkist** - Швидкість анімації (1-100)

### Гірлянда B

- **select.girlianda_b_rezhim** - Вибір режиму роботи
  - Постійне
  - Почергове
  - Дихання
  - Хаос
  - Свічка

- **number.girlianda_b_iaskravist** - Яскравість (0-255)

- **number.girlianda_b_shvidkist** - Швидкість анімації (1-100)

## MQTT Топіки

### Топіки для гірлянди A

**Режим:**
- Config: `homeassistant/select/girlianda/garland_a_mode/config`
- State: `homeassistant/select/girlianda/garland_a_mode/state`
- Command: `homeassistant/select/girlianda/garland_a_mode/set`

**Яскравість:**
- Config: `homeassistant/number/girlianda/garland_a_brightness/config`
- State: `homeassistant/number/girlianda/garland_a_brightness/state`
- Command: `homeassistant/number/girlianda/garland_a_brightness/set`

**Швидкість:**
- Config: `homeassistant/number/girlianda/garland_a_speed/config`
- State: `homeassistant/number/girlianda/garland_a_speed/state`
- Command: `homeassistant/number/girlianda/garland_a_speed/set`

### Топіки для гірлянди B

Аналогічні топіки з заміною `garland_a` на `garland_b`.

### Availability

- Topic: `homeassistant/girlianda_01/availability`
- Payload: `online` / `offline`

## Синхронізація стану

- Стани гірлянд публікуються кожні 5 секунд
- При зміні параметрів через Home Assistant, стан оновлюється миттєво
- При зміні параметрів через веб-інтерфейс, стан в Home Assistant оновиться протягом 5 секунд

## Fallback MQTT хостів

Якщо основний MQTT брокер недоступний, ESP32 автоматично спробує підключитися до наступного хоста зі списку. Це забезпечує надійність підключення.

Приклад конфігурації з кількома хостами:

```cpp
inline const char* mqttHosts[] = {
    "mqtt.lan",           // Основний хост (DNS)
    "192.168.0.120",      // Резервний хост (IP)
    "192.168.0.150"       // Додатковий резервний хост
};
```

## Налагодження

Для перегляду логів MQTT підключення відкрийте Serial Monitor (115200 baud). Ви побачите повідомлення про:
- Спроби підключення до MQTT брокера
- Успішне підключення
- Публікацію конфігурації Home Assistant Discovery
- Отримані команди від Home Assistant
- Публікацію станів гірлянд

## Приклад використання в Home Assistant

### Автоматизація

```yaml
automation:
  - alias: "Увімкнути гірлянду ввечері"
    trigger:
      - platform: sun
        event: sunset
    action:
      - service: select.select_option
        target:
          entity_id: select.girlianda_a_rezhim
        data:
          option: "Дихання"
      - service: number.set_value
        target:
          entity_id: number.girlianda_a_iaskravist
        data:
          value: 200
```

### Lovelace Card

```yaml
type: entities
title: Гірлянда
entities:
  - entity: select.girlianda_a_rezhim
  - entity: number.girlianda_a_iaskravist
  - entity: number.girlianda_a_shvidkist
  - entity: select.girlianda_b_rezhim
  - entity: number.girlianda_b_iaskravist
  - entity: number.girlianda_b_shvidkist
```
