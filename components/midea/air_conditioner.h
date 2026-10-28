#pragma once

#if (defined(USE_ARDUINO) && !defined(USE_RP2) && !defined(USE_LIBRETINY)) || defined(USE_ESP_IDF)

#include <Appliance/AirConditioner/AirConditioner.h>

#include "appliance_base.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome::midea::ac {

using binary_sensor::BinarySensor;
using climate::ClimateCall;
using climate::ClimateFanMode;
using climate::ClimateMode;
using climate::ClimateModeMask;
using climate::ClimatePreset;
using climate::ClimatePresetMask;
using climate::ClimateSwingMode;
using climate::ClimateSwingModeMask;
using climate::ClimateTraits;
using sensor::Sensor;

class AirConditioner final : public ApplianceBase<dudanov::midea::ac::AirConditioner>, public climate::Climate {
 public:
  void setup() override;
  void dump_config() override;
  void on_status_change() override;

  void set_diagnostic_interval(uint32_t value) { this->diagnostic_interval_ = value; }
  void set_property_interval(uint32_t value) { this->property_interval_ = value; }
  void set_power_format(uint8_t value) {
    this->base_.setPowerFormat(static_cast<dudanov::midea::ac::PowerFormat>(value));
  }

  void set_outdoor_temperature_sensor(Sensor *value) { this->outdoor_temperature_sensor_ = value; }
  void set_power_usage_sensor(Sensor *value) { this->power_usage_sensor_ = value; }
  void set_humidity_setpoint_sensor(Sensor *value) { this->humidity_setpoint_sensor_ = value; }
  void set_indoor_humidity_sensor(Sensor *value) { this->indoor_humidity_sensor_ = value; }
  void set_error_code_sensor(Sensor *value) { this->error_code_sensor_ = value; }
  void set_display_level_sensor(Sensor *value) { this->display_level_sensor_ = value; }

  void set_electrified_hours_sensor(Sensor *value) { this->electrified_hours_sensor_ = value; }
  void set_total_operating_hours_sensor(Sensor *value) { this->total_operating_hours_sensor_ = value; }
  void set_current_operating_hours_sensor(Sensor *value) { this->current_operating_hours_sensor_ = value; }
  void set_compressor_frequency_sensor(Sensor *value) { this->compressor_frequency_sensor_ = value; }
  void set_target_compressor_frequency_sensor(Sensor *value) { this->target_compressor_frequency_sensor_ = value; }
  void set_compressor_current_sensor(Sensor *value) { this->compressor_current_sensor_ = value; }
  void set_compressor_voltage_sensor(Sensor *value) { this->compressor_voltage_sensor_ = value; }
  void set_indoor_coil_temperature_sensor(Sensor *value) { this->indoor_coil_temperature_sensor_ = value; }
  void set_outdoor_coil_temperature_sensor(Sensor *value) { this->outdoor_coil_temperature_sensor_ = value; }
  void set_discharge_pipe_temperature_sensor(Sensor *value) { this->discharge_pipe_temperature_sensor_ = value; }
  void set_target_indoor_fan_speed_sensor(Sensor *value) { this->target_indoor_fan_speed_sensor_ = value; }
  void set_indoor_fan_speed_sensor(Sensor *value) { this->indoor_fan_speed_sensor_ = value; }
  void set_outdoor_fan_speed_sensor(Sensor *value) { this->outdoor_fan_speed_sensor_ = value; }
  void set_indoor_unit_fault_flags_sensor(Sensor *value) { this->indoor_unit_fault_flags_sensor_ = value; }
  void set_indoor_unit_protection_flags_sensor(Sensor *value) {
    this->indoor_unit_protection_flags_sensor_ = value;
  }
  void set_total_energy_sensor(Sensor *value) { this->total_energy_sensor_ = value; }
  void set_total_operating_energy_sensor(Sensor *value) { this->total_operating_energy_sensor_ = value; }
  void set_current_energy_sensor(Sensor *value) { this->current_energy_sensor_ = value; }
  void set_outdoor_unit_power_sensor(Sensor *value) { this->outdoor_unit_power_sensor_ = value; }
  void set_horizontal_louver_angle_sensor(Sensor *value) { this->horizontal_louver_angle_sensor_ = value; }
  void set_vertical_louver_angle_sensor(Sensor *value) { this->vertical_louver_angle_sensor_ = value; }

  void set_breeze_control_sensor(Sensor *value) { this->breeze_control_sensor_ = value; }
  void set_rate_select_sensor(Sensor *value) { this->rate_select_sensor_ = value; }
  void set_fresh_air_speed_sensor(Sensor *value) { this->fresh_air_speed_sensor_ = value; }
  void set_cascade_direction_sensor(Sensor *value) { this->cascade_direction_sensor_ = value; }

  void set_appliance_error_sensor(BinarySensor *value) { this->appliance_error_sensor_ = value; }
  void set_filter_alert_sensor(BinarySensor *value) { this->filter_alert_sensor_ = value; }
  void set_display_on_sensor(BinarySensor *value) { this->display_on_sensor_ = value; }
  void set_follow_me_sensor(BinarySensor *value) { this->follow_me_sensor_ = value; }
  void set_purifier_sensor(BinarySensor *value) { this->purifier_sensor_ = value; }
  void set_auxiliary_heat_sensor(BinarySensor *value) { this->auxiliary_heat_sensor_ = value; }
  void set_independent_auxiliary_heat_sensor(BinarySensor *value) {
    this->independent_auxiliary_heat_sensor_ = value;
  }
  void set_power_saving_sensor(BinarySensor *value) { this->power_saving_sensor_ = value; }
  void set_natural_wind_sensor(BinarySensor *value) { this->natural_wind_sensor_ = value; }
  void set_comfort_mode_sensor(BinarySensor *value) { this->comfort_mode_sensor_ = value; }
  void set_fahrenheit_sensor(BinarySensor *value) { this->fahrenheit_sensor_ = value; }
  void set_defrosting_sensor(BinarySensor *value) { this->defrosting_sensor_ = value; }
  void set_breezeless_sensor(BinarySensor *value) { this->breezeless_sensor_ = value; }
  void set_prompt_tone_sensor(BinarySensor *value) { this->prompt_tone_sensor_ = value; }
  void set_self_clean_sensor(BinarySensor *value) { this->self_clean_sensor_ = value; }
  void set_breeze_away_sensor(BinarySensor *value) { this->breeze_away_sensor_ = value; }
  void set_water_pump_sensor(BinarySensor *value) { this->water_pump_sensor_ = value; }
  void set_cascade_sensor(BinarySensor *value) { this->cascade_sensor_ = value; }
  void set_flash_sensor(BinarySensor *value) { this->flash_sensor_ = value; }
  void set_outdoor_silent_sensor(BinarySensor *value) { this->outdoor_silent_sensor_ = value; }
  void set_ieco_sensor(BinarySensor *value) { this->ieco_sensor_ = value; }
  void set_sound_sensor(BinarySensor *value) { this->sound_sensor_ = value; }

  void do_follow_me(float temperature, bool use_fahrenheit, bool beeper = false);
  void do_display_toggle();
  void do_swing_step();
  void do_beeper_on() { this->set_beeper_feedback(true); }
  void do_beeper_off() { this->set_beeper_feedback(false); }
  void do_power_on() { this->base_.setPowerState(true); }
  void do_power_off() { this->base_.setPowerState(false); }
  void do_power_toggle() { this->base_.setPowerState(this->mode == ClimateMode::CLIMATE_MODE_OFF); }
  void set_supported_modes(ClimateModeMask modes) { this->supported_modes_ = modes; }
  void set_supported_swing_modes(ClimateSwingModeMask modes) { this->supported_swing_modes_ = modes; }
  void set_supported_presets(ClimatePresetMask presets) { this->supported_presets_ = presets; }
  void set_custom_presets(std::initializer_list<const char *> presets) {
    this->set_supported_custom_presets(presets);
  }
  void set_custom_fan_modes(std::initializer_list<const char *> modes) {
    this->set_supported_custom_fan_modes(modes);
  }

 protected:
  void control(const ClimateCall &call) override;
  ClimateTraits traits() override;
  void query_next_group_();
  void query_properties_();
  bool wants_group_(dudanov::midea::ac::GroupId group) const;
  bool wants_properties_() const;

  ClimateModeMask supported_modes_{};
  ClimateSwingModeMask supported_swing_modes_{};
  ClimatePresetMask supported_presets_{};
  bool frost_protection_set_{false};
  uint32_t diagnostic_interval_{10000};
  uint32_t property_interval_{30000};
  uint8_t next_group_{};
  uint8_t group_failures_[12]{};

  Sensor *outdoor_temperature_sensor_{};
  Sensor *power_usage_sensor_{};
  Sensor *humidity_setpoint_sensor_{};
  Sensor *indoor_humidity_sensor_{};
  Sensor *error_code_sensor_{};
  Sensor *display_level_sensor_{};
  Sensor *electrified_hours_sensor_{};
  Sensor *total_operating_hours_sensor_{};
  Sensor *current_operating_hours_sensor_{};
  Sensor *compressor_frequency_sensor_{};
  Sensor *target_compressor_frequency_sensor_{};
  Sensor *compressor_current_sensor_{};
  Sensor *compressor_voltage_sensor_{};
  Sensor *indoor_coil_temperature_sensor_{};
  Sensor *outdoor_coil_temperature_sensor_{};
  Sensor *discharge_pipe_temperature_sensor_{};
  Sensor *target_indoor_fan_speed_sensor_{};
  Sensor *indoor_fan_speed_sensor_{};
  Sensor *outdoor_fan_speed_sensor_{};
  Sensor *indoor_unit_fault_flags_sensor_{};
  Sensor *indoor_unit_protection_flags_sensor_{};
  Sensor *total_energy_sensor_{};
  Sensor *total_operating_energy_sensor_{};
  Sensor *current_energy_sensor_{};
  Sensor *outdoor_unit_power_sensor_{};
  Sensor *horizontal_louver_angle_sensor_{};
  Sensor *vertical_louver_angle_sensor_{};
  Sensor *breeze_control_sensor_{};
  Sensor *rate_select_sensor_{};
  Sensor *fresh_air_speed_sensor_{};
  Sensor *cascade_direction_sensor_{};

  BinarySensor *appliance_error_sensor_{};
  BinarySensor *filter_alert_sensor_{};
  BinarySensor *display_on_sensor_{};
  BinarySensor *follow_me_sensor_{};
  BinarySensor *purifier_sensor_{};
  BinarySensor *auxiliary_heat_sensor_{};
  BinarySensor *independent_auxiliary_heat_sensor_{};
  BinarySensor *power_saving_sensor_{};
  BinarySensor *natural_wind_sensor_{};
  BinarySensor *comfort_mode_sensor_{};
  BinarySensor *fahrenheit_sensor_{};
  BinarySensor *defrosting_sensor_{};
  BinarySensor *breezeless_sensor_{};
  BinarySensor *prompt_tone_sensor_{};
  BinarySensor *self_clean_sensor_{};
  BinarySensor *breeze_away_sensor_{};
  BinarySensor *water_pump_sensor_{};
  BinarySensor *cascade_sensor_{};
  BinarySensor *flash_sensor_{};
  BinarySensor *outdoor_silent_sensor_{};
  BinarySensor *ieco_sensor_{};
  BinarySensor *sound_sensor_{};
};

}  // namespace esphome::midea::ac

#endif  // USE_ARDUINO || USE_ESP_IDF
