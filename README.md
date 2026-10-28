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
are queried only when a property-backed entity is configured. Each property is queried by
itself, and only an individual property is retired after three empty, failed, or timed-out
responses. Core C0 climate polling is unchanged. This isolates unsupported tags and keeps
optional protocol traffic and ESP8266 memory use proportional to the entities actually needed.

The opt-in entity surface covers:

- extended C0 state, including smart-eye, natural-wind, dry, eco, sleep, prevent-cold, PMV,
  frost-protection, comfort-mode, and fresh-filter fields;
- C1 runtime, compressor, refrigerant temperature, indoor/outdoor fan, condensate-pump,
  energy, humidity, defrost, outdoor-power, and louver diagnostics;
- isolated B0/B1 polling for breezeless, prompt tone, self-clean, breeze controls, rate
  select, fresh air, water pump, cascade, flash, outdoor-silent, iECO, and sound.

The library preserves valid unknown B5 and B0/B1 records for future interpretation. It does
not expose a guessed entity name merely because a tag or non-zero byte exists.

The component does not yet claim an “opposing mode disabled” Boolean because the protocol
evidence does not establish one. Compressor actual/target frequency, outdoor-unit power,
appliance-error state, error code, defrost state, coil temperatures, and fan speeds are exposed
independently so a bounded hardware qualification can establish the signature without encoding
an unvalidated inference into firmware.

The byte-level coverage contract, evidence levels, property vocabulary, and intentionally
unimplemented model-specific families are documented in
[`MideaUART/docs/air-conditioner-protocol.md`](https://github.com/juchong/MideaUART/blob/rearchitecture/docs/air-conditioner-protocol.md).

## Energy encoding

`power_format` selects the unit's Group 4 representation:

- `BCD`
- `BINARY`
- `MIXED`
- `BINARY_0_01_KWH`
- `BCD_ENERGY_BINARY_POWER`

## Validation

CI validates and compiles the full entity surface for an actual `esp12e` target against ESPHome
2026.6.4 and the latest tagged ESPHome GitHub release. The latest-version job also runs weekly,
so a future ESPHome change is detected even when this repository has not changed. The component
pins MideaUART to an exact commit; the pin is advanced only after the library tests pass.
