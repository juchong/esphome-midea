from esphome import automation
import esphome.codegen as cg
from esphome.components import (
    binary_sensor,
    climate,
    remote_base,
    remote_transmitter,
    sensor,
    uart,
)
from esphome.components.climate import ClimateMode, ClimatePreset, ClimateSwingMode
from esphome.components.remote_base import CONF_TRANSMITTER_ID
import esphome.config_validation as cv
from esphome.const import (
    CONF_AUTOCONF,
    CONF_BEEPER,
    CONF_CUSTOM_FAN_MODES,
    CONF_CUSTOM_PRESETS,
    CONF_ID,
    CONF_NUM_ATTEMPTS,
    CONF_OUTDOOR_TEMPERATURE,
    CONF_PERIOD,
    CONF_SUPPORTED_MODES,
    CONF_SUPPORTED_PRESETS,
    CONF_SUPPORTED_SWING_MODES,
    CONF_TEMPERATURE,
    CONF_TIMEOUT,
    CONF_USE_FAHRENHEIT,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_FREQUENCY,
    DEVICE_CLASS_HUMIDITY,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_PROBLEM,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLTAGE,
    ENTITY_CATEGORY_DIAGNOSTIC,
    ICON_POWER,
    ICON_THERMOMETER,
    ICON_WATER_PERCENT,
    PLATFORM_ESP32,
    PLATFORM_ESP8266,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_AMPERE,
    UNIT_CELSIUS,
    UNIT_HERTZ,
    UNIT_KILOWATT_HOURS,
    UNIT_PERCENT,
    UNIT_VOLT,
    UNIT_WATT,
)
from esphome.core import CORE

CODEOWNERS = ["@juchong"]
DEPENDENCIES = ["climate", "uart"]
AUTO_LOAD = ["sensor", "binary_sensor"]

CONF_POWER_USAGE = "power_usage"
CONF_HUMIDITY_SETPOINT = "humidity_setpoint"
CONF_DIAGNOSTIC_INTERVAL = "diagnostic_interval"
CONF_PROPERTY_INTERVAL = "property_interval"
CONF_POWER_FORMAT = "power_format"

# C0 state
CONF_INDOOR_HUMIDITY = "indoor_humidity"
CONF_APPLIANCE_ERROR = "appliance_error"
CONF_ERROR_CODE = "error_code"
CONF_FILTER_ALERT = "filter_alert"
CONF_DISPLAY_ON = "display_on"
CONF_DISPLAY_LEVEL = "display_level"
CONF_FOLLOW_ME = "follow_me"
CONF_PURIFIER = "purifier"
CONF_AUXILIARY_HEAT = "auxiliary_heat"
CONF_INDEPENDENT_AUXILIARY_HEAT = "independent_auxiliary_heat"
CONF_POWER_SAVING = "power_saving"
CONF_NATURAL_WIND = "natural_wind"
CONF_COMFORT_MODE = "comfort_mode"
CONF_FAHRENHEIT = "fahrenheit"

# C1 data groups
CONF_ELECTRIFIED_HOURS = "electrified_hours"
CONF_TOTAL_OPERATING_HOURS = "total_operating_hours"
CONF_CURRENT_OPERATING_HOURS = "current_operating_hours"
CONF_COMPRESSOR_FREQUENCY = "compressor_frequency"
CONF_TARGET_COMPRESSOR_FREQUENCY = "target_compressor_frequency"
CONF_COMPRESSOR_CURRENT = "compressor_current"
CONF_COMPRESSOR_VOLTAGE = "compressor_voltage"
CONF_INDOOR_COIL_TEMPERATURE = "indoor_coil_temperature"
CONF_OUTDOOR_COIL_TEMPERATURE = "outdoor_coil_temperature"
CONF_DISCHARGE_PIPE_TEMPERATURE = "discharge_pipe_temperature"
CONF_TARGET_INDOOR_FAN_SPEED = "target_indoor_fan_speed"
CONF_INDOOR_FAN_SPEED = "indoor_fan_speed"
CONF_OUTDOOR_FAN_SPEED = "outdoor_fan_speed"
CONF_INDOOR_UNIT_FAULT_FLAGS = "indoor_unit_fault_flags"
CONF_INDOOR_UNIT_PROTECTION_FLAGS = "indoor_unit_protection_flags"
CONF_TOTAL_ENERGY = "total_energy"
CONF_TOTAL_OPERATING_ENERGY = "total_operating_energy"
CONF_CURRENT_ENERGY = "current_energy"
CONF_DEFROSTING = "defrosting"
CONF_OUTDOOR_UNIT_POWER = "outdoor_unit_power"
CONF_HORIZONTAL_LOUVER_ANGLE = "horizontal_louver_angle"
CONF_VERTICAL_LOUVER_ANGLE = "vertical_louver_angle"

# B0/B1 properties
CONF_BREEZELESS = "breezeless"
CONF_PROMPT_TONE = "prompt_tone"
CONF_SELF_CLEAN = "self_clean"
CONF_BREEZE_AWAY = "breeze_away"
CONF_BREEZE_CONTROL = "breeze_control"
CONF_RATE_SELECT = "rate_select"
CONF_FRESH_AIR_SPEED = "fresh_air_speed"
CONF_WATER_PUMP = "water_pump"
CONF_CASCADE = "cascade"
CONF_CASCADE_DIRECTION = "cascade_direction"
CONF_FLASH = "flash"
CONF_OUTDOOR_SILENT = "outdoor_silent"
CONF_IECO = "ieco"
CONF_SOUND = "sound"

midea_ac_ns = cg.esphome_ns.namespace("midea").namespace("ac")
AirConditioner = midea_ac_ns.class_("AirConditioner", climate.Climate, cg.Component)
Capabilities = midea_ac_ns.namespace("Constants")


def measurement_schema(
    unit=None, icon=None, accuracy=0, device_class=None, total_increasing=False
):
    args = {
        "accuracy_decimals": accuracy,
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
        "state_class": (
            STATE_CLASS_TOTAL_INCREASING
            if total_increasing
            else STATE_CLASS_MEASUREMENT
        ),
    }
    if unit is not None:
        args["unit_of_measurement"] = unit
    if icon is not None:
        args["icon"] = icon
    if device_class is not None:
        args["device_class"] = device_class
    return sensor.sensor_schema(**args)


def problem_schema(icon):
    return binary_sensor.binary_sensor_schema(
        icon=icon,
        device_class=DEVICE_CLASS_PROBLEM,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    )


SENSOR_SCHEMAS = {
    CONF_OUTDOOR_TEMPERATURE: measurement_schema(
        UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE
    ),
    CONF_POWER_USAGE: measurement_schema(
        UNIT_WATT, ICON_POWER, 1, DEVICE_CLASS_POWER
    ),
    CONF_HUMIDITY_SETPOINT: measurement_schema(
        UNIT_PERCENT, ICON_WATER_PERCENT, 0, DEVICE_CLASS_HUMIDITY
    ),
    CONF_INDOOR_HUMIDITY: measurement_schema(
        UNIT_PERCENT, ICON_WATER_PERCENT, 0, DEVICE_CLASS_HUMIDITY
    ),
    CONF_ERROR_CODE: measurement_schema(icon="mdi:alert-circle-outline"),
    CONF_DISPLAY_LEVEL: measurement_schema(icon="mdi:brightness-6"),
    CONF_ELECTRIFIED_HOURS: measurement_schema("h", "mdi:clock-outline", 2),
    CONF_TOTAL_OPERATING_HOURS: measurement_schema("h", "mdi:clock-outline", 2),
    CONF_CURRENT_OPERATING_HOURS: measurement_schema("h", "mdi:clock-outline", 2),
    CONF_COMPRESSOR_FREQUENCY: measurement_schema(
        UNIT_HERTZ, "mdi:sine-wave", 0, DEVICE_CLASS_FREQUENCY
    ),
    CONF_TARGET_COMPRESSOR_FREQUENCY: measurement_schema(
        UNIT_HERTZ, "mdi:sine-wave", 0, DEVICE_CLASS_FREQUENCY
    ),
    CONF_COMPRESSOR_CURRENT: measurement_schema(
        UNIT_AMPERE, "mdi:current-ac", 0, DEVICE_CLASS_CURRENT
    ),
    CONF_COMPRESSOR_VOLTAGE: measurement_schema(
        UNIT_VOLT, "mdi:flash", 0, DEVICE_CLASS_VOLTAGE
    ),
    CONF_INDOOR_COIL_TEMPERATURE: measurement_schema(
        UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE
    ),
    CONF_OUTDOOR_COIL_TEMPERATURE: measurement_schema(
        UNIT_CELSIUS, ICON_THERMOMETER, 1, DEVICE_CLASS_TEMPERATURE
    ),
    CONF_DISCHARGE_PIPE_TEMPERATURE: measurement_schema(
        UNIT_CELSIUS, ICON_THERMOMETER, 0, DEVICE_CLASS_TEMPERATURE
    ),
    CONF_TARGET_INDOOR_FAN_SPEED: measurement_schema("rpm", "mdi:fan", 0),
    CONF_INDOOR_FAN_SPEED: measurement_schema("rpm", "mdi:fan", 0),
    CONF_OUTDOOR_FAN_SPEED: measurement_schema("rpm", "mdi:fan", 0),
    CONF_INDOOR_UNIT_FAULT_FLAGS: measurement_schema(icon="mdi:code-braces"),
    CONF_INDOOR_UNIT_PROTECTION_FLAGS: measurement_schema(icon="mdi:shield-outline"),
    CONF_TOTAL_ENERGY: measurement_schema(
        UNIT_KILOWATT_HOURS,
        "mdi:counter",
        2,
        DEVICE_CLASS_ENERGY,
        total_increasing=True,
    ),
    CONF_TOTAL_OPERATING_ENERGY: measurement_schema(
        UNIT_KILOWATT_HOURS,
        "mdi:counter",
        2,
        DEVICE_CLASS_ENERGY,
        total_increasing=True,
    ),
    CONF_CURRENT_ENERGY: measurement_schema(
        UNIT_KILOWATT_HOURS, "mdi:lightning-bolt", 2, DEVICE_CLASS_ENERGY
    ),
    CONF_OUTDOOR_UNIT_POWER: measurement_schema(
        UNIT_WATT, ICON_POWER, 0, DEVICE_CLASS_POWER
    ),
    CONF_HORIZONTAL_LOUVER_ANGLE: measurement_schema("°", "mdi:angle-acute", 0),
    CONF_VERTICAL_LOUVER_ANGLE: measurement_schema("°", "mdi:angle-acute", 0),
    CONF_BREEZE_CONTROL: measurement_schema(icon="mdi:weather-windy"),
    CONF_RATE_SELECT: measurement_schema(icon="mdi:speedometer"),
    CONF_FRESH_AIR_SPEED: measurement_schema(icon="mdi:air-filter"),
    CONF_CASCADE_DIRECTION: measurement_schema(icon="mdi:swap-vertical"),
}

BINARY_SENSOR_SCHEMAS = {
    CONF_APPLIANCE_ERROR: problem_schema("mdi:alert-circle-outline"),
    CONF_FILTER_ALERT: problem_schema("mdi:air-filter"),
    CONF_DISPLAY_ON: binary_sensor.binary_sensor_schema(
        icon="mdi:television-ambient-light", entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    CONF_FOLLOW_ME: binary_sensor.binary_sensor_schema(
        icon="mdi:map-marker-account-outline", entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    CONF_PURIFIER: binary_sensor.binary_sensor_schema(
        icon="mdi:air-purifier", entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    CONF_AUXILIARY_HEAT: binary_sensor.binary_sensor_schema(
        icon="mdi:radiator", entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    CONF_INDEPENDENT_AUXILIARY_HEAT: binary_sensor.binary_sensor_schema(
        icon="mdi:radiator", entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    CONF_POWER_SAVING: binary_sensor.binary_sensor_schema(
        icon="mdi:leaf", entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    CONF_NATURAL_WIND: binary_sensor.binary_sensor_schema(
        icon="mdi:weather-windy", entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    CONF_COMFORT_MODE: binary_sensor.binary_sensor_schema(
        icon="mdi:sofa-outline", entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    CONF_FAHRENHEIT: binary_sensor.binary_sensor_schema(
        icon="mdi:temperature-fahrenheit", entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    CONF_DEFROSTING: binary_sensor.binary_sensor_schema(
        icon="mdi:snowflake-melt", entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    CONF_BREEZELESS: binary_sensor.binary_sensor_schema(
        icon="mdi:weather-windy-variant", entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    CONF_PROMPT_TONE: binary_sensor.binary_sensor_schema(
        icon="mdi:volume-high", entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    CONF_SELF_CLEAN: binary_sensor.binary_sensor_schema(
        icon="mdi:spray-bottle", entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    CONF_BREEZE_AWAY: binary_sensor.binary_sensor_schema(
        icon="mdi:weather-windy", entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    CONF_WATER_PUMP: binary_sensor.binary_sensor_schema(
        icon="mdi:pump", entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    CONF_CASCADE: binary_sensor.binary_sensor_schema(
        icon="mdi:swap-vertical", entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    CONF_FLASH: binary_sensor.binary_sensor_schema(
        icon="mdi:flash", entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    CONF_OUTDOOR_SILENT: binary_sensor.binary_sensor_schema(
        icon="mdi:volume-off", entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    CONF_IECO: binary_sensor.binary_sensor_schema(
        icon="mdi:leaf", entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    CONF_SOUND: binary_sensor.binary_sensor_schema(
        icon="mdi:volume-high", entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
}

POWER_FORMATS = {
    "BCD": 1,
    "BINARY": 2,
    "MIXED": 3,
    "BINARY_0_01_KWH": 12,
    "BCD_ENERGY_BINARY_POWER": 101,
}


def templatize(value):
    if isinstance(value, cv.Schema):
        value = value.schema
    return cv.Schema({key: cv.templatable(val) for key, val in value.items()})


ALLOWED_CLIMATE_MODES = {
    "HEAT_COOL": ClimateMode.CLIMATE_MODE_HEAT_COOL,
    "COOL": ClimateMode.CLIMATE_MODE_COOL,
    "HEAT": ClimateMode.CLIMATE_MODE_HEAT,
    "DRY": ClimateMode.CLIMATE_MODE_DRY,
    "FAN_ONLY": ClimateMode.CLIMATE_MODE_FAN_ONLY,
}
ALLOWED_CLIMATE_PRESETS = {
    "ECO": ClimatePreset.CLIMATE_PRESET_ECO,
    "BOOST": ClimatePreset.CLIMATE_PRESET_BOOST,
    "SLEEP": ClimatePreset.CLIMATE_PRESET_SLEEP,
}
ALLOWED_CLIMATE_SWING_MODES = {
    "BOTH": ClimateSwingMode.CLIMATE_SWING_BOTH,
    "VERTICAL": ClimateSwingMode.CLIMATE_SWING_VERTICAL,
    "HORIZONTAL": ClimateSwingMode.CLIMATE_SWING_HORIZONTAL,
}
CUSTOM_FAN_MODES = {"SILENT": Capabilities.SILENT, "TURBO": Capabilities.TURBO}
CUSTOM_PRESETS = {"FREEZE_PROTECTION": Capabilities.FREEZE_PROTECTION}

schema_fields = {
    cv.Optional(CONF_PERIOD, default="1s"): cv.time_period,
    cv.Optional(CONF_TIMEOUT, default="2s"): cv.time_period,
    cv.Optional(CONF_NUM_ATTEMPTS, default=3): cv.int_range(min=1, max=5),
    cv.OnlyWith(CONF_TRANSMITTER_ID, "remote_transmitter"): cv.use_id(
        remote_transmitter.RemoteTransmitterComponent
    ),
    cv.Optional(CONF_BEEPER, default=False): cv.boolean,
    cv.Optional(CONF_AUTOCONF, default=True): cv.boolean,
    cv.Optional(CONF_SUPPORTED_MODES): cv.ensure_list(
        cv.enum(ALLOWED_CLIMATE_MODES, upper=True)
    ),
    cv.Optional(CONF_SUPPORTED_SWING_MODES): cv.ensure_list(
        cv.enum(ALLOWED_CLIMATE_SWING_MODES, upper=True)
    ),
    cv.Optional(CONF_SUPPORTED_PRESETS): cv.ensure_list(
        cv.enum(ALLOWED_CLIMATE_PRESETS, upper=True)
    ),
    cv.Optional(CONF_CUSTOM_PRESETS): cv.ensure_list(
        cv.enum(CUSTOM_PRESETS, upper=True)
    ),
    cv.Optional(CONF_CUSTOM_FAN_MODES): cv.ensure_list(
        cv.enum(CUSTOM_FAN_MODES, upper=True)
    ),
    cv.Optional(CONF_DIAGNOSTIC_INTERVAL, default="10s"): cv.positive_time_period_milliseconds,
    cv.Optional(CONF_PROPERTY_INTERVAL, default="30s"): cv.positive_time_period_milliseconds,
    cv.Optional(CONF_POWER_FORMAT, default="BCD"): cv.enum(POWER_FORMATS, upper=True),
}
schema_fields.update({cv.Optional(key): value for key, value in SENSOR_SCHEMAS.items()})
schema_fields.update(
    {cv.Optional(key): value for key, value in BINARY_SENSOR_SCHEMAS.items()}
)

platform_validator = (
    cv.only_on([PLATFORM_ESP32, PLATFORM_ESP8266])
    if hasattr(cv, "only_on")
    else cv.only_with_arduino
)

CONFIG_SCHEMA = cv.All(
    climate.climate_schema(AirConditioner)
    .extend(schema_fields)
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA),
    platform_validator,
)

MIDEA_ACTION_BASE_SCHEMA = cv.Schema({cv.GenerateID(CONF_ID): cv.use_id(AirConditioner)})
MIDEA_FOLLOW_ME_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_TEMPERATURE): cv.templatable(cv.temperature),
        cv.Optional(CONF_USE_FAHRENHEIT, default=False): cv.templatable(cv.boolean),
        cv.Optional(CONF_BEEPER, default=False): cv.templatable(cv.boolean),
    }
)

if hasattr(automation, "register_apply_action"):
    automation.register_apply_action(
        "midea_ac.follow_me",
        templatize(MIDEA_FOLLOW_ME_SCHEMA).extend(MIDEA_ACTION_BASE_SCHEMA),
        automation.ApplyCall(
            "do_follow_me({}, {}, {})",
            (
                (CONF_TEMPERATURE, cg.float_),
                (CONF_USE_FAHRENHEIT, cg.bool_),
                (CONF_BEEPER, cg.bool_),
            ),
        ),
    )
    for _name, _call in (
        ("midea_ac.display_toggle", "do_display_toggle()"),
        ("midea_ac.swing_step", "do_swing_step()"),
        ("midea_ac.beeper_on", "do_beeper_on()"),
        ("midea_ac.beeper_off", "do_beeper_off()"),
        ("midea_ac.power_on", "do_power_on()"),
        ("midea_ac.power_off", "do_power_off()"),
        ("midea_ac.power_toggle", "do_power_toggle()"),
    ):
        automation.register_apply_action(
            _name, MIDEA_ACTION_BASE_SCHEMA, automation.ApplyCall(_call)
        )
else:
    from esphome.core import coroutine

    FollowMeAction = midea_ac_ns.class_("FollowMeAction", automation.Action)
    DisplayToggleAction = midea_ac_ns.class_("DisplayToggleAction", automation.Action)
    SwingStepAction = midea_ac_ns.class_("SwingStepAction", automation.Action)
    BeeperOnAction = midea_ac_ns.class_("BeeperOnAction", automation.Action)
    BeeperOffAction = midea_ac_ns.class_("BeeperOffAction", automation.Action)
    PowerOnAction = midea_ac_ns.class_("PowerOnAction", automation.Action)
    PowerOffAction = midea_ac_ns.class_("PowerOffAction", automation.Action)
    PowerToggleAction = midea_ac_ns.class_("PowerToggleAction", automation.Action)

    def register_action(name, type_, schema):
        validator = templatize(schema).extend(MIDEA_ACTION_BASE_SCHEMA)
        registerer = automation.register_action(
            f"midea_ac.{name}", type_, validator, synchronous=True
        )

        def decorator(func):
            async def new_func(config, action_id, template_arg, args):
                ac_ = await cg.get_variable(config[CONF_ID])
                var = cg.new_Pvariable(action_id, template_arg)
                cg.add(var.set_parent(ac_))
                await coroutine(func)(var, config, args)
                return var

            return registerer(new_func)

        return decorator

    @register_action("follow_me", FollowMeAction, MIDEA_FOLLOW_ME_SCHEMA)
    async def follow_me_to_code(var, config, args):
        cg.add(var.set_beeper(await cg.templatable(config[CONF_BEEPER], args, cg.bool_)))
        cg.add(
            var.set_use_fahrenheit(
                await cg.templatable(config[CONF_USE_FAHRENHEIT], args, cg.bool_)
            )
        )
        cg.add(
            var.set_temperature(
                await cg.templatable(config[CONF_TEMPERATURE], args, cg.float_)
            )
        )

    for _name, _type in (
        ("display_toggle", DisplayToggleAction),
        ("swing_step", SwingStepAction),
        ("beeper_on", BeeperOnAction),
        ("beeper_off", BeeperOffAction),
        ("power_on", PowerOnAction),
        ("power_off", PowerOffAction),
        ("power_toggle", PowerToggleAction),
    ):

        async def action_to_code(var, config, args):
            pass

        register_action(_name, _type, cv.Schema({}))(action_to_code)

if hasattr(uart, "final_validate_device_schema"):
    FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
        "midea", baud_rate=9600, require_rx=True, require_tx=True
    )


SENSOR_SETTERS = {key: f"set_{key}_sensor" for key in SENSOR_SCHEMAS}
BINARY_SENSOR_SETTERS = {
    key: f"set_{key}_sensor" for key in BINARY_SENSOR_SCHEMAS
}


async def register_entities(config, var, definitions, factory):
    for key, setter in definitions.items():
        if key in config:
            entity = await factory(config[key])
            cg.add(getattr(var, setter)(entity))


async def to_code(config):
    var = await climate.new_climate(config)
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    cg.add(var.set_period(config[CONF_PERIOD].total_milliseconds))
    cg.add(var.set_response_timeout(config[CONF_TIMEOUT].total_milliseconds))
    cg.add(var.set_request_attempts(config[CONF_NUM_ATTEMPTS]))
    cg.add(var.set_diagnostic_interval(config[CONF_DIAGNOSTIC_INTERVAL].total_milliseconds))
    cg.add(var.set_property_interval(config[CONF_PROPERTY_INTERVAL].total_milliseconds))
    cg.add(var.set_power_format(config[CONF_POWER_FORMAT]))
    if CONF_TRANSMITTER_ID in config:
        if hasattr(remote_base, "request_protocol"):
            remote_base.request_protocol("midea")
        cg.add_define("USE_REMOTE_TRANSMITTER")
        transmitter_ = await cg.get_variable(config[CONF_TRANSMITTER_ID])
        cg.add(var.set_transmitter(transmitter_))
    cg.add(var.set_beeper_feedback(config[CONF_BEEPER]))
    cg.add(var.set_autoconf(config[CONF_AUTOCONF]))
    if CONF_SUPPORTED_MODES in config:
        cg.add(var.set_supported_modes(config[CONF_SUPPORTED_MODES]))
    if CONF_SUPPORTED_SWING_MODES in config:
        cg.add(var.set_supported_swing_modes(config[CONF_SUPPORTED_SWING_MODES]))
    if CONF_SUPPORTED_PRESETS in config:
        cg.add(var.set_supported_presets(config[CONF_SUPPORTED_PRESETS]))
    if CONF_CUSTOM_PRESETS in config:
        cg.add(var.set_custom_presets(config[CONF_CUSTOM_PRESETS]))
    if CONF_CUSTOM_FAN_MODES in config:
        cg.add(var.set_custom_fan_modes(config[CONF_CUSTOM_FAN_MODES]))
    await register_entities(config, var, SENSOR_SETTERS, sensor.new_sensor)
    await register_entities(
        config, var, BINARY_SENSOR_SETTERS, binary_sensor.new_binary_sensor
    )
    if CORE.is_esp32 and CORE.using_arduino:
        cg.add_library("WiFi", None)
    cg.add_library(
        name="MideaUART",
        version=None,
        repository="https://github.com/juchong/MideaUART.git#fa7df0763eb36432e60a9721ea5734adeb3086ab",
    )
