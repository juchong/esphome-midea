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
    c0_byte_8_bit_7:
      name: Loft C0 byte 8 bit 7
```

Only configured diagnostics are queried. C1 groups are polled one at a time; B0/B1 properties
are queried only when a property-backed entity is configured. Each property is queried by
itself. If an expanded reply arrives before the first C0 status, its diagnostic values are
kept but the Climate entity is not published until C0 has supplied the real operating mode.
A configured query is not retired after an arbitrary failure count. Core C0 climate polling
is unchanged. This isolates unsupported tags and keeps
optional protocol traffic and ESP8266 memory use proportional to the entities actually needed.
`indoor_humidity` is sourced from C1 Group 5 and therefore opts into that query; it is not a
passive C0 field. `humidity_setpoint`, by contrast, is carried in the ordinary C0 status reply.

The opt-in entity surface covers:

- corroborated extended C0 state, including smart-eye, natural-wind, eco, sleep, prevent-cold,
  PMV, frost-protection, comfort-mode, and fresh-filter fields, plus explicitly named raw fields
  for disputed byte 8 bit 7, byte 9 bit 2, and the three-bit display field;
- C1 compressor, refrigerant temperature, indoor/outdoor fan, condensate-pump,
  energy, humidity, and outdoor-power diagnostics;
- isolated B0/B1 polling for display state, louver angles, breezeless, prompt tone, self-clean,
  breeze-away, rate select, fresh air, outdoor-silent, and sound.

The library preserves valid unknown B5 and B0/B1 records for future interpretation. It does
not expose a guessed entity name merely because a tag or non-zero byte exists.

The component does not expose a generic “opposing mode disabled” Boolean. A bounded test on a
two-head MRCOOL system found that the denied head sets `appliance_error` while `error_code`
remains zero. In a later isolated-head test, turning only the other head off left every setting
on the denied Office head unchanged; the flag cleared on the next C0 cycle and that head began
actively cooling. That is a model-qualified candidate signature, not a universal
reinterpretation of a generic appliance-error bit. The shared component keeps the protocol
fields separate and truthful so a deployment can derive an explicitly model-scoped sensor.

Optional support is also head-specific. On the same installation, the Loft head returned live
C1 Groups 1–3 while the different-capacity Office head returned none, even after isolation had
cleared its inhibit flag and it was actively cooling. Both heads continued to answer C0 and the
qualified B1 display/louver properties. An unanswered C1 entity therefore remains unknown; it
does not block climate publication or imply a transport failure.

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
2026.10.1 and the latest tagged ESPHome GitHub release. The latest-version job also runs weekly,
so a future ESPHome change is detected even when this repository has not changed. The component
pins MideaUART to an exact commit; the pin is advanced only after the library tests pass.
