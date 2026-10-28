#if (defined(USE_ARDUINO) && !defined(USE_RP2) && !defined(USE_LIBRETINY)) || defined(USE_ESP_IDF)

#include "air_conditioner.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

#include "ac_adapter.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome::midea::ac {

template<typename T> static void publish_sensor(Sensor *sensor, const dudanov::Optional<T> &value) {
  if (sensor != nullptr && value.hasValue()) {
    const float next = static_cast<float>(value.value());
    if (!sensor->has_state() || sensor->get_raw_state() != next)
      sensor->publish_state(next);
  }
}

static void publish_binary(BinarySensor *sensor, const dudanov::Optional<bool> &value) {
  if (sensor != nullptr && value.hasValue() && (!sensor->has_state() || sensor->state != value.value()))
    sensor->publish_state(value.value());
}

template<typename T> static void update_property(T &property, const T &value, bool &changed) {
  if (property != value) {
    property = value;
    changed = true;
  }
}

void AirConditioner::setup() {
  // All C1 traffic is consumer-driven by the configured entities below.
  this->base_.setPowerQuery(false);
  ApplianceBase<dudanov::midea::ac::AirConditioner>::setup();
  this->configure_property_queries_();

  bool wants_diagnostics = false;
  for (const auto group : {dudanov::midea::ac::GroupId::RUNTIME,
                           dudanov::midea::ac::GroupId::PERFORMANCE,
                           dudanov::midea::ac::GroupId::INDOOR_FAN,
                           dudanov::midea::ac::GroupId::OUTDOOR_FAN,
                           dudanov::midea::ac::GroupId::ENERGY,
                           dudanov::midea::ac::GroupId::ENVIRONMENT,
                           dudanov::midea::ac::GroupId::OUTDOOR_POWER,
                           dudanov::midea::ac::GroupId::LOUVERS})
    wants_diagnostics |= this->wants_group_(group);

  if (wants_diagnostics) {
    this->set_timeout("midea_diagnostic_start", 15000, [this]() {
      this->set_interval("midea_diagnostics", this->diagnostic_interval_, [this]() { this->query_next_group_(); });
    });
  }
  if (this->wants_properties_()) {
    this->set_timeout("midea_property_start", 20000, [this]() {
      this->set_interval("midea_properties", this->property_interval_, [this]() { this->query_properties_(); });
    });
  }
}

void AirConditioner::on_status_change() {
  const auto &state = this->base_.getState();
  if (this->base_.getAutoconfStatus() == dudanov::midea::AUTOCONF_OK &&
      this->base_.getCapabilities().supportFrostProtectionPreset() && !this->frost_protection_set_) {
    const auto traits = this->get_traits();
    const auto &existing = traits.get_supported_custom_presets();
    bool found = false;
    for (const char *preset : existing)
      found |= strcmp(preset, Constants::FREEZE_PROTECTION) == 0;
    if (!found) {
      std::vector<const char *> merged(existing.begin(), existing.end());
      merged.push_back(Constants::FREEZE_PROTECTION);
      this->set_supported_custom_presets(merged);
    }
    this->frost_protection_set_ = true;
  }

  bool climate_changed = false;
  if (state.target_temperature.hasValue())
    update_property(this->target_temperature, state.target_temperature.value(), climate_changed);
  if (state.indoor_temperature.hasValue())
    update_property(this->current_temperature, state.indoor_temperature.value(), climate_changed);
  if (state.mode.hasValue())
    update_property(this->mode, Converters::to_climate_mode(state.mode.value()), climate_changed);
  if (state.swing_mode.hasValue())
    update_property(this->swing_mode, Converters::to_climate_swing_mode(state.swing_mode.value()), climate_changed);
  if (state.preset.hasValue()) {
    const auto preset = state.preset.value();
    if (Converters::is_custom_midea_preset(preset)) {
      if (this->set_custom_preset_(Converters::to_custom_climate_preset(preset)))
        climate_changed = true;
    } else if (this->set_preset_(Converters::to_climate_preset(preset))) {
      climate_changed = true;
    }
  }
  if (state.fan_mode.hasValue()) {
    const auto fan_mode = state.fan_mode.value();
    if (Converters::is_custom_midea_fan_mode(fan_mode)) {
      if (this->set_custom_fan_mode_(Converters::to_custom_climate_fan_mode(fan_mode)))
        climate_changed = true;
    } else if (this->set_fan_mode_(Converters::to_climate_fan_mode(fan_mode))) {
      climate_changed = true;
    }
  }
  if (climate_changed)
    this->publish_state();

  publish_sensor(this->outdoor_temperature_sensor_, state.outdoor_temperature);
  publish_sensor(this->power_usage_sensor_, state.realtime_power);
  publish_sensor(this->humidity_setpoint_sensor_, state.target_humidity);
  publish_sensor(this->indoor_humidity_sensor_, state.indoor_humidity);
  publish_sensor(this->error_code_sensor_, state.error_code);
  publish_sensor(this->display_level_sensor_, state.display_level);
  publish_sensor(this->pmv_sensor_, state.pmv);
  publish_sensor(this->fresh_filter_total_sensor_, state.fresh_filter_total);
  publish_sensor(this->fresh_filter_used_sensor_, state.fresh_filter_used);
  publish_binary(this->appliance_error_sensor_, state.appliance_error);
  publish_binary(this->filter_alert_sensor_, state.filter_alert);
  publish_binary(this->display_on_sensor_, state.display_on);
  publish_binary(this->follow_me_sensor_, state.follow_me);
  publish_binary(this->purifier_sensor_, state.purifier);
  publish_binary(this->auxiliary_heat_sensor_, state.aux_heat);
  publish_binary(this->power_saving_sensor_, state.power_saving);
  publish_binary(this->smart_eye_sensor_, state.smart_eye);
  publish_binary(this->natural_wind_sensor_, state.natural_wind);
  publish_binary(this->dry_sensor_, state.dry);
  publish_binary(this->eco_mode_sensor_, state.eco_mode);
  publish_binary(this->sleep_mode_sensor_, state.sleep_mode);
  publish_binary(this->prevent_cold_sensor_, state.prevent_cold);
  publish_binary(this->frost_protection_sensor_, state.frost_protection);
  publish_binary(this->comfort_mode_sensor_, state.comfort_mode);
  publish_binary(this->fresh_filter_timeout_sensor_, state.fresh_filter_timeout);
  publish_binary(this->fahrenheit_sensor_, state.fahrenheit);

  publish_sensor(this->electrified_hours_sensor_, state.electrified_hours);
  publish_sensor(this->total_operating_hours_sensor_, state.total_operating_hours);
  publish_sensor(this->current_operating_hours_sensor_, state.current_operating_hours);
  publish_sensor(this->compressor_frequency_sensor_, state.compressor_frequency);
  publish_sensor(this->target_compressor_frequency_sensor_, state.target_compressor_frequency);
  publish_sensor(this->compressor_current_sensor_, state.compressor_current);
  publish_sensor(this->compressor_voltage_sensor_, state.compressor_voltage);
  publish_sensor(this->indoor_coil_temperature_sensor_, state.indoor_coil_temperature);
  publish_sensor(this->outdoor_coil_temperature_sensor_, state.outdoor_coil_temperature);
  publish_sensor(this->discharge_pipe_temperature_sensor_, state.discharge_pipe_temperature);
  publish_sensor(this->target_indoor_fan_speed_sensor_, state.target_indoor_fan_speed);
  publish_sensor(this->indoor_fan_speed_sensor_, state.indoor_fan_speed);
  publish_sensor(this->outdoor_fan_speed_sensor_, state.outdoor_fan_speed);
  publish_sensor(this->indoor_unit_fault_flags_sensor_, state.indoor_unit_fault_flags);
  publish_sensor(this->indoor_unit_protection_flags_sensor_, state.indoor_unit_protection_flags);
  publish_sensor(this->total_energy_sensor_, state.total_energy);
  publish_sensor(this->total_operating_energy_sensor_, state.total_operating_energy);
  publish_sensor(this->current_energy_sensor_, state.current_energy);
  publish_binary(this->defrosting_sensor_, state.defrosting);
  publish_sensor(this->outdoor_unit_power_sensor_, state.outdoor_unit_power);
  publish_sensor(this->horizontal_louver_angle_sensor_, state.horizontal_louver_angle);
  publish_sensor(this->vertical_louver_angle_sensor_, state.vertical_louver_angle);

  publish_binary(this->breezeless_sensor_, state.breezeless);
  publish_binary(this->prompt_tone_sensor_, state.prompt_tone);
  publish_binary(this->self_clean_sensor_, state.self_clean);
  publish_binary(this->breeze_away_sensor_, state.breeze_away);
  publish_sensor(this->breeze_control_sensor_, state.breeze_control);
  publish_sensor(this->rate_select_sensor_, state.rate_select);
  publish_sensor(this->fresh_air_speed_sensor_, state.fresh_air_speed);
  publish_binary(this->water_pump_sensor_, state.water_pump_running);
  publish_binary(this->cascade_sensor_, state.cascade);
  publish_sensor(this->cascade_direction_sensor_, state.cascade_direction);
  publish_binary(this->flash_sensor_, state.flash);
  publish_binary(this->outdoor_silent_sensor_, state.outdoor_silent);
  publish_binary(this->ieco_sensor_, state.ieco);
  publish_binary(this->sound_sensor_, state.sound);
}

bool AirConditioner::wants_group_(dudanov::midea::ac::GroupId group) const {
  using dudanov::midea::ac::GroupId;
  switch (group) {
    case GroupId::RUNTIME:
      return this->electrified_hours_sensor_ != nullptr || this->total_operating_hours_sensor_ != nullptr ||
             this->current_operating_hours_sensor_ != nullptr;
    case GroupId::PERFORMANCE:
      return this->compressor_frequency_sensor_ != nullptr || this->target_compressor_frequency_sensor_ != nullptr ||
             this->compressor_current_sensor_ != nullptr || this->compressor_voltage_sensor_ != nullptr ||
             this->indoor_coil_temperature_sensor_ != nullptr || this->outdoor_coil_temperature_sensor_ != nullptr ||
             this->discharge_pipe_temperature_sensor_ != nullptr;
    case GroupId::INDOOR_FAN:
      return this->target_indoor_fan_speed_sensor_ != nullptr || this->indoor_fan_speed_sensor_ != nullptr ||
             this->water_pump_sensor_ != nullptr ||
             this->indoor_unit_fault_flags_sensor_ != nullptr ||
             this->indoor_unit_protection_flags_sensor_ != nullptr;
    case GroupId::OUTDOOR_FAN:
      return this->outdoor_fan_speed_sensor_ != nullptr && this->indoor_humidity_sensor_ == nullptr &&
             this->defrosting_sensor_ == nullptr;
    case GroupId::ENERGY:
      return this->power_usage_sensor_ != nullptr || this->total_energy_sensor_ != nullptr ||
             this->total_operating_energy_sensor_ != nullptr || this->current_energy_sensor_ != nullptr;
    case GroupId::ENVIRONMENT:
      return this->indoor_humidity_sensor_ != nullptr || this->defrosting_sensor_ != nullptr;
    case GroupId::OUTDOOR_POWER:
      return this->outdoor_unit_power_sensor_ != nullptr;
    case GroupId::LOUVERS:
      return this->horizontal_louver_angle_sensor_ != nullptr || this->vertical_louver_angle_sensor_ != nullptr;
  }
  return false;
}

void AirConditioner::query_next_group_() {
  static const dudanov::midea::ac::GroupId groups[] = {
      dudanov::midea::ac::GroupId::RUNTIME,       dudanov::midea::ac::GroupId::PERFORMANCE,
      dudanov::midea::ac::GroupId::INDOOR_FAN,    dudanov::midea::ac::GroupId::OUTDOOR_FAN,
      dudanov::midea::ac::GroupId::ENERGY,        dudanov::midea::ac::GroupId::ENVIRONMENT,
      dudanov::midea::ac::GroupId::OUTDOOR_POWER, dudanov::midea::ac::GroupId::LOUVERS,
  };
  for (uint8_t count = 0; count < sizeof(groups) / sizeof(groups[0]); ++count) {
    const auto group = groups[this->next_group_++ % (sizeof(groups) / sizeof(groups[0]))];
    const uint8_t index = static_cast<uint8_t>(group);
    if (!this->wants_group_(group) || this->group_failures_[index] >= 3)
      continue;
    const bool queued = this->base_.requestGroup(
        group, [this, index](const dudanov::midea::ac::GroupData &) { this->group_failures_[index] = 0; },
        [this, index]() {
          if (++this->group_failures_[index] == 3)
            ESP_LOGW(Constants::TAG, "C1 group %u did not answer; stopping this query until reboot", index);
        });
    if (queued)
      return;
  }
}

bool AirConditioner::wants_properties_() const {
  return !this->property_queries_.empty();
}

void AirConditioner::configure_property_queries_() {
  using dudanov::midea::ac::PropertyId;
  this->property_queries_.clear();
  auto add = [this](bool configured, PropertyId id) {
    if (configured)
      this->property_queries_.push_back(PropertyPoll{static_cast<uint16_t>(id), 0});
  };
  add(this->breezeless_sensor_ != nullptr, PropertyId::BREEZELESS);
  add(this->prompt_tone_sensor_ != nullptr, PropertyId::PROMPT_TONE);
  add(this->self_clean_sensor_ != nullptr, PropertyId::SELF_CLEAN);
  add(this->breeze_away_sensor_ != nullptr, PropertyId::BREEZE_AWAY);
  add(this->breeze_control_sensor_ != nullptr, PropertyId::BREEZE_CONTROL);
  add(this->rate_select_sensor_ != nullptr, PropertyId::RATE_SELECT);
  add(this->fresh_air_speed_sensor_ != nullptr, PropertyId::FRESH_AIR);
  add(this->water_pump_sensor_ != nullptr, PropertyId::WATER_PUMP);
  add(this->cascade_sensor_ != nullptr || this->cascade_direction_sensor_ != nullptr, PropertyId::CASCADE);
  add(this->flash_sensor_ != nullptr, PropertyId::FLASH);
  add(this->outdoor_silent_sensor_ != nullptr, PropertyId::OUTDOOR_SILENT);
  add(this->ieco_sensor_ != nullptr, PropertyId::IECO);
  add(this->sound_sensor_ != nullptr, PropertyId::SOUND);
}

void AirConditioner::query_properties_() {
  for (size_t count = 0; count < this->property_queries_.size(); ++count) {
    const size_t index = this->next_property_++ % this->property_queries_.size();
    auto &poll = this->property_queries_[index];
    if (poll.failures >= 3)
      continue;
    const uint16_t id = poll.id;
    const bool queued = this->base_.requestProperties(
        {id},
        [this, index, id](const std::vector<dudanov::midea::ac::PropertyValue> &values) {
          const auto value = std::find_if(values.begin(), values.end(),
                                          [id](const dudanov::midea::ac::PropertyValue &item) {
                                            return item.id == id && item.successful();
                                          });
          if (value != values.end()) {
            this->property_queries_[index].failures = 0;
            return;
          }
          auto &failed = this->property_queries_[index].failures;
          if (++failed == 3)
            ESP_LOGW(Constants::TAG,
                     "B1 property 0x%04X returned no usable value; stopping this query until reboot", id);
        },
        [this, index, id]() {
          auto &failed = this->property_queries_[index].failures;
          if (++failed == 3)
            ESP_LOGW(Constants::TAG, "B1 property 0x%04X did not answer; stopping this query until reboot", id);
        });
    if (queued)
      return;
  }
}

void AirConditioner::control(const ClimateCall &call) {
  dudanov::midea::ac::Control control{};
  if (const auto value = call.get_target_temperature(); value.has_value())
    control.targetTemp = *value;
  if (const auto value = call.get_swing_mode(); value.has_value())
    control.swingMode = Converters::to_midea_swing_mode(*value);
  if (const auto value = call.get_mode(); value.has_value())
    control.mode = Converters::to_midea_mode(*value);
  if (const auto value = call.get_preset(); value.has_value())
    control.preset = Converters::to_midea_preset(*value);
  else if (call.has_custom_preset())
    control.preset = Converters::to_midea_preset(call.get_custom_preset().c_str());
  if (const auto value = call.get_fan_mode(); value.has_value())
    control.fanMode = Converters::to_midea_fan_mode(*value);
  else if (call.has_custom_fan_mode())
    control.fanMode = Converters::to_midea_fan_mode(call.get_custom_fan_mode().c_str());
  this->base_.control(control);
}

ClimateTraits AirConditioner::traits() {
  ClimateTraits traits;
  traits.add_feature_flags(climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE);
  traits.set_visual_min_temperature(17);
  traits.set_visual_max_temperature(30);
  traits.set_visual_temperature_step(0.5);
  traits.set_supported_modes(this->supported_modes_);
  traits.set_supported_swing_modes(this->supported_swing_modes_);
  traits.set_supported_presets(this->supported_presets_);
  traits.add_supported_fan_mode(ClimateFanMode::CLIMATE_FAN_AUTO);
  traits.add_supported_fan_mode(ClimateFanMode::CLIMATE_FAN_LOW);
  traits.add_supported_fan_mode(ClimateFanMode::CLIMATE_FAN_MEDIUM);
  traits.add_supported_fan_mode(ClimateFanMode::CLIMATE_FAN_HIGH);
  if (this->base_.getAutoconfStatus() == dudanov::midea::AUTOCONF_OK) {
    const auto &capabilities = this->base_.getCapabilities();
    Converters::to_climate_traits(traits, capabilities);
    float minimum = 100;
    float maximum = -100;
    if (capabilities.supportAutoMode()) {
      minimum = std::min(minimum, capabilities.minTempAuto());
      maximum = std::max(maximum, capabilities.maxTempAuto());
    }
    if (capabilities.supportCoolMode()) {
      minimum = std::min(minimum, capabilities.minTempCool());
      maximum = std::max(maximum, capabilities.maxTempCool());
    }
    if (capabilities.supportHeatMode()) {
      minimum = std::min(minimum, capabilities.minTempHeat());
      maximum = std::max(maximum, capabilities.maxTempHeat());
    }
    if (minimum <= maximum) {
      traits.set_visual_min_temperature(minimum);
      traits.set_visual_max_temperature(maximum);
    }
    traits.set_visual_temperature_step(capabilities.decimals() ? 0.5F : 1.0F);
  }
  if (!traits.get_supported_modes().empty())
    traits.add_supported_mode(ClimateMode::CLIMATE_MODE_OFF);
  if (!traits.get_supported_swing_modes().empty())
    traits.add_supported_swing_mode(ClimateSwingMode::CLIMATE_SWING_OFF);
  if (!traits.get_supported_presets().empty())
    traits.add_supported_preset(ClimatePreset::CLIMATE_PRESET_NONE);
  return traits;
}

void AirConditioner::dump_config() {
  ESP_LOGCONFIG(Constants::TAG,
                "Midea UART:\n"
                "  Period: %" PRIu32 "ms\n"
                "  Response timeout: %" PRIu32 "ms\n"
                "  Request attempts: %d\n"
                "  Diagnostic interval: %" PRIu32 "ms\n"
                "  Property interval: %" PRIu32 "ms",
                this->base_.getPeriod(), this->base_.getTimeout(), this->base_.getNumAttempts(),
                this->diagnostic_interval_, this->property_interval_);
  if (this->base_.getAutoconfStatus() == dudanov::midea::AUTOCONF_OK)
    this->base_.getCapabilities().dump();
  else if (this->base_.getAutoconfStatus() == dudanov::midea::AUTOCONF_ERROR)
    ESP_LOGW(Constants::TAG, "The B5 capabilities report was not available");
  this->dump_traits_(Constants::TAG);
}

void AirConditioner::do_follow_me(float temperature, bool use_fahrenheit, bool beeper) {
#ifdef USE_REMOTE_TRANSMITTER
  if (!std::isfinite(temperature)) {
    ESP_LOGW(Constants::TAG, "Follow-me requires a finite temperature");
    return;
  }
  const uint8_t rounded = static_cast<uint8_t>(
      esphome::clamp<long>(std::lroundf(temperature), 0L, static_cast<long>(UINT8_MAX)));
  IrFollowMeData data(rounded, use_fahrenheit, beeper);
  this->transmitter_.transmit(data);
#else
  ESP_LOGW(Constants::TAG, "Follow-me requires remote_transmitter");
#endif
}

void AirConditioner::do_swing_step() {
#ifdef USE_REMOTE_TRANSMITTER
  IrSpecialData data(0x01);
  this->transmitter_.transmit(data);
#else
  ESP_LOGW(Constants::TAG, "Swing-step requires remote_transmitter");
#endif
}

void AirConditioner::do_display_toggle() {
  if (this->base_.getCapabilities().supportLightControl())
    this->base_.displayToggle();
  else {
#ifdef USE_REMOTE_TRANSMITTER
    IrSpecialData data(0x08);
    this->transmitter_.transmit(data);
#else
    ESP_LOGW(Constants::TAG, "Display control is unavailable without remote_transmitter");
#endif
  }
}

}  // namespace esphome::midea::ac

#endif  // USE_ARDUINO || USE_ESP_IDF
