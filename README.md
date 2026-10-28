# ESPHome Midea UART component

This is a focused external component for Midea-compatible HVAC heads connected through a UART
bridge. It starts from ESPHome's official `midea` component and pins the companion
[`juchong/MideaUART`](https://github.com/juchong/MideaUART) protocol library by commit.

It is deliberately separate from a complete appliance firmware project. Your ESPHome YAML,
board choice, Wi-Fi, OTA, API, and device-specific substitutions stay in your own configuration.
The component works with the ESP8266 `esp12e` used by the current bridges.

## Use

```yaml
external_components:
  - source: github://juchong/esphome-midea@main
    components: [midea]

uart:
  id: midea_uart
  tx_pin: GPIO1
  rx_pin: GPIO3
  baud_rate: 9600

climate:
  - platform: midea
    name: Loft MrCool
    uart_id: midea_uart
    compressor_frequency:
      name: Loft compressor frequency
    target_compressor_frequency:
      name: Loft target compressor frequency
    outdoor_unit_power:
      name: Loft outdoor unit power
    appliance_error:
      name: Loft appliance error
    error_code:
      name: Loft error code
    defrosting:
      name: Loft defrosting
```

Only configured diagnostics are queried. C1 groups are polled one at a time; B0/B1 properties
are queried only when a property-backed entity is configured. Core C0 climate polling is
unchanged. This keeps optional protocol traffic and ESP8266 memory use proportional to the
entities actually needed.

The component does not claim an “opposing mode disabled” Boolean because the protocol evidence
does not establish one. Compressor actual/target frequency, outdoor-unit power, appliance-error
state, error code, defrost state, coil temperatures, and fan speeds are exposed independently so
the real signature can be observed without encoding an unvalidated inference into firmware.

## Energy encoding

`power_format` selects the unit's Group 4 representation:

- `BCD`
- `BINARY`
- `MIXED`
- `BINARY_0_01_KWH`
- `BCD_ENERGY_BINARY_POWER`

## Validation

CI compiles the full entity surface for an actual `esp12e` target against ESPHome 2026.6.4 and
the newest ESPHome release available on PyPI. The latest-version job also runs weekly so a future
ESPHome change is detected even when this repository has not changed.
