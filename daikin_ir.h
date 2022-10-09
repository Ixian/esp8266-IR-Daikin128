#include "esphome.h"
#include "IRremoteESP8266.h"
#include "IRsend.h"
#include "ir_Daikin.h"


const uint16_t kIrLed = 0;  // ESP8266 GPIO pin to use.
IRDaikin128 ac(kIrLed);
// bool currentPowerState = false; // Starting state for power toggle

class DaikinAC : public Component, public Climate {
 public:
  sensor::Sensor *sensor_{nullptr};

  void set_sensor(sensor::Sensor *sensor) { this->sensor_ = sensor; }

  void setup() override {

  if (this->sensor_) {
        this->sensor_->add_on_state_callback([this](float state) {
          this->current_temperature = state;
          this->publish_state();
        });
        this->current_temperature = this->sensor_->state;
      } else {
        this->current_temperature = NAN;
      }

    ac.begin();
    // ac.setPowerToggle(false);
    ac.setTemp(21);
    ac.setFan(kDaikinFanAuto);
    ac.setMode(kDaikinCool);
    ac.setSwingVertical(false);
 	ac.setSleep(false);
 	ac.setQuiet(false);
 	ac.setPowerful(false);
 	ac.setEcono(false);
  }

  climate::ClimateTraits traits() {
    auto traits = climate::ClimateTraits();
    traits.set_supports_current_temperature(true);
    traits.set_supports_auto_mode(true);
    traits.set_supports_cool_mode(true);
    traits.set_supports_heat_mode(true);
	traits.set_supports_fan_only_mode(true);
	traits.set_supports_dry_mode(true);
	traits.set_supports_fan_mode_auto(true);
    traits.set_supports_fan_mode_high(true);
    traits.set_supports_fan_mode_low(true);
    traits.set_supports_fan_mode_medium(true);
	traits.set_supports_swing_mode_off(true);
	traits.set_supports_fan_mode_diffuse(true);
    traits.set_supports_two_point_target_temperature(false);
    traits.set_supports_away(false);
    traits.set_visual_min_temperature(16);
    traits.set_visual_max_temperature(30);
    traits.set_visual_temperature_step(1.f);
    return traits;
  }
  // Capture Power state
  void setPowerState() {
    if(ac.getPowerToggle() == true){
       ac.setPowerToggle(false);
    }
    else ac.setPowerToggle(true);
  }
  void control(const ClimateCall &call) override {
    if (call.get_mode().has_value()) {
      // User requested mode change
      ClimateMode mode = *call.get_mode();
      // Send mode to hardware
      if (mode == CLIMATE_MODE_HEAT) {
        // ac.setPowerToggle(true);
        setPowerState();
        ac.setMode(kDaikin128Heat);
      }
      else if (mode == CLIMATE_MODE_COOL) {
        setPowerState();
        ac.setMode(kDaikin128Cool);
      }
      else if (mode == CLIMATE_MODE_AUTO) {
        setPowerState();
        ac.setMode(kDaikin128Auto);
      }
	  else if (mode == CLIMATE_MODE_FAN_ONLY) {
        setPowerState();
        ac.setMode(kDaikin128Fan);
      }
	  else if (mode == CLIMATE_MODE_DRY) {
        setPowerState();
        ac.setMode(kDaikin128Dry);
      }
      else if (mode == CLIMATE_MODE_OFF) {
        ac.setPowerToggle(true);
        }
	

      // Publish updated state
    this->mode = mode;
    this->publish_state();
    }
    if (call.get_target_temperature().has_value()) {
      // User requested target temperature change
      float temp = *call.get_target_temperature();
      // Send target temp to climate
      ac.setTemp(temp);
      this->target_temperature = temp;
     this->publish_state();
    }

    if (call.get_fan_mode().has_value()) {
      ClimateFanMode fan_mode = *call.get_fan_mode();
      if (fan_mode == CLIMATE_FAN_AUTO) {
        setPowerState();
        ac.setFan(kDaikin128FanAuto);
      } else if (fan_mode == CLIMATE_FAN_LOW) {
        setPowerState();
        ac.setFan(kDaikin128FanLow);
      } else if (fan_mode == CLIMATE_FAN_MEDIUM) {
        setPowerState();
        ac.setFan(kDaikin128FanMed);
      } else if (fan_mode == CLIMATE_FAN_HIGH) {
        setPowerState();
        ac.setFan(kDaikin128FanHigh);
      } else if (fan_mode == CLIMATE_FAN_DIFFUSE) {
        setPowerState();
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
  }
};
