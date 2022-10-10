#include "esphome.h"
#include "IRremoteESP8266.h"
#include "IRsend.h"
#include "ir_Daikin.h"

///This code is relevant for cases where the IR control for an AC is available in IRremoteESP8266, but isn't supported yet in Esphome

const uint16_t kIrLed = 0; // ESP8266 GPIO pin to use. Recommended: 0 (D3).
IRDaikin128 ac(kIrLed);

// Setup files. This is the equivalent of the code written in the setup loop of Arduino
class DaikinAC : public Component, public Climate {
  public:
    sensor::Sensor *sensor_{nullptr};

    void set_sensor(sensor::Sensor *sensor) { this->sensor_ = sensor; }

    void setup() override
    {
      if (this->sensor_) {
        this->sensor_->add_on_state_callback([this](float state) {
          this->current_temperature = state;
          this->publish_state();
        });
        this->current_temperature = this->sensor_->state;
      } else {
        this->current_temperature = NAN;
      }

      auto restore = this->restore_state_();
      if (restore.has_value()) {
        restore->apply(this);
      } else {
        this->mode = climate::CLIMATE_MODE_OFF;
        this->target_temperature = roundf(clamp(this->current_temperature, 18.0f, 30.0f));
        this->fan_mode = climate::CLIMATE_FAN_AUTO;
        this->swing_mode = climate::CLIMATE_SWING_OFF;
      }

      if (isnan(this->target_temperature)) {
        this->target_temperature = 23;
      }

      ac.begin();
      // ac.on();
      if (this->mode == CLIMATE_MODE_OFF) {
        ac.setPowerToggle(true);
      } else if (this->mode == CLIMATE_MODE_AUTO) {
        ac.setMode(kDaikin128Auto);
      } else if (this->mode == CLIMATE_MODE_COOL) {
        ac.setMode(kDaikin128Cool);
      } else if (this->mode == CLIMATE_MODE_HEAT) {
        ac.setMode(kDaikin128Heat);
      } else if (this->mode == CLIMATE_MODE_FAN_ONLY) {
        ac.setMode(kDaikin128Fan);
      } else if (this->mode == CLIMATE_MODE_DRY) {
        ac.setMode(kDaikin128Dry);
      }
      ac.setTemp(this->target_temperature);
      if (this->fan_mode == CLIMATE_FAN_AUTO) {
        ac.setFan(kDaikin128FanAuto);
      } else if (this->fan_mode == CLIMATE_FAN_LOW) {
        ac.setFan(kDaikin128FanLow);
      } else if (this->fan_mode == CLIMATE_FAN_MEDIUM) {
        ac.setFan(kDaikin128FanMed);
      } else if (this->fan_mode == CLIMATE_FAN_HIGH) {
        ac.setFan(kDaikin128FanHigh);
      } else if (this->fan_mode == CLIMATE_FAN_FOCUS) {
        ac.setFan(kDaikin128FanPowerful);
      } else if (this->fan_mode == CLIMATE_FAN_DIFFUSE) {
        ac.setFan(kDaikin128FanQuiet);
      }
      if (this->swing_mode == CLIMATE_SWING_OFF) {
        ac.setSwingVertical(false);
      } else if (this->swing_mode == CLIMATE_SWING_VERTICAL) {
        ac.setSwingVertical(true);
      }
      ac.send();

      ESP_LOGD("DEBUG", "Daikin A/C remote is in the following state:");
      ESP_LOGD("DEBUG", "  %s\n", ac.toString().c_str());
    }
// Traits: This tells home assistant what "traits" are supported by AC in terms of heating/cooling/fan speeds/swing modes. These are used by Home Assistant to customize the AC card on the dashboard
    climate::ClimateTraits traits() {
      auto traits = climate::ClimateTraits();
      traits.set_supported_modes({
          climate::CLIMATE_MODE_AUTO,
          climate::CLIMATE_MODE_COOL,
          climate::CLIMATE_MODE_HEAT,
          climate::CLIMATE_MODE_FAN_ONLY,
          climate::CLIMATE_MODE_DRY,
      });
      traits.set_supported_fan_modes({
          climate::CLIMATE_FAN_AUTO,
          climate::CLIMATE_FAN_HIGH,
          climate::CLIMATE_FAN_LOW,
          climate::CLIMATE_FAN_MEDIUM,
          climate::CLIMATE_FAN_DIFFUSE,
          climate::CLIMATE_FAN_FOCUS,
      });
      traits.set_supported_swing_modes({
          climate::CLIMATE_SWING_OFF,
          climate::CLIMATE_SWING_VERTICAL,
      });
      traits.set_supports_current_temperature(this->sensor_ != nullptr);
      traits.set_supports_two_point_target_temperature(false);
      traits.set_visual_max_temperature(30);
      traits.set_visual_min_temperature(18);
      traits.set_visual_temperature_step(1);

      return traits;
    }
  // Power Toggle function - testing
  void setPowerState() {
    if(ac.getPowerToggle() == true){
       ac.setPowerToggle(false);
    }
    else ac.setPowerToggle(true);
  }
//Code for what to do when the mode of the AC is changed on the dashboard
  void control(const ClimateCall &call) override {
    if (call.get_mode().has_value()) {
      ClimateMode mode = *call.get_mode();
//For each mode, need to find the relevant mode from the list of constants. This list can be found in the relevant .h library from IRremoteESP8266 library. In this case the file is "ir_Hitachi.h". Typically the function should be the same - .setMode. However, best check the relevant .h library.       
      if (mode == CLIMATE_MODE_OFF) {
        ac.setPowerToggle(true);
      } else if (mode == CLIMATE_MODE_AUTO) {
        setPowerState();
        ac.setMode(kDaikin128Auto);
      } else if (mode == CLIMATE_MODE_COOL) {
        setPowerState();
        ac.setMode(kDaikin128Cool);
      } else if (mode == CLIMATE_MODE_HEAT) {
        setPowerState();
        ac.setMode(kDaikin128Heat);
      } else if (mode == CLIMATE_MODE_FAN_ONLY) {
        setPowerState();
        ac.setMode(kDaikin128Fan);
      } else if (mode == CLIMATE_MODE_DRY) {
        setPowerState();
        ac.setMode(kDaikin128Dry);
      }
      this->mode = mode;
    }

    if (call.get_target_temperature().has_value()) {
      float temp = *call.get_target_temperature();
      ac.setTemp(temp);
      this->target_temperature = temp;
    }

    if (call.get_fan_mode().has_value()) {
      ClimateFanMode fan_mode = *call.get_fan_mode();
      if (fan_mode == CLIMATE_FAN_AUTO) {
        ac.setFan(kDaikin128FanAuto);
      } else if (fan_mode == CLIMATE_FAN_LOW) {
        ac.setFan(kDaikin128FanLow);
      } else if (fan_mode == CLIMATE_FAN_MEDIUM) {
        ac.setFan(kDaikin128FanMed);
      } else if (fan_mode == CLIMATE_FAN_HIGH) {
        ac.setFan(kDaikin128FanHigh);
      } else if (fan_mode == CLIMATE_FAN_FOCUS) {
        ac.setFan(kDaikin128FanPowerful);
      } else if (fan_mode == CLIMATE_FAN_DIFFUSE) {
        ac.setFan(kDaikin128FanQuiet);
      }
      this->fan_mode = fan_mode;
    }

    if (call.get_swing_mode().has_value()) {
      ClimateSwingMode swing_mode = *call.get_swing_mode();
      if (swing_mode == CLIMATE_SWING_OFF) {
        ac.setSwingVertical(false);
      } else if (swing_mode == CLIMATE_SWING_VERTICAL) {
        ac.setSwingVertical(true);
      }
      this->swing_mode = swing_mode;
    }

    ac.send();

    this->publish_state();

    ESP_LOGD("DEBUG", "Daikin A/C remote is in the following state:");
    ESP_LOGD("DEBUG", "  %s\n", ac.toString().c_str());
  }
};