// Copyright 2013 Emilie Gillet.
//
// Author: Emilie Gillet (emilie.o.gillet@gmail.com)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// -----------------------------------------------------------------------------
//
// User interface handler.

#include "shruthi/ui.h"

#include "shruthi/editor.h"
#include "shruthi/part.h"

namespace shruthi {

using namespace avrlib;

/* static */
bool Ui::inhibit_encoder_release_;
uint8_t Ui::switch_state_[SWITCH_LAST];
uint32_t Ui::switch_delay_[SWITCH_LAST];
uint8_t Ui::sub_clock_;
uint8_t Ui::sub_clock_2_;
uint8_t Ui::progress_bar_;
uint8_t Ui::led_pattern_;
uint8_t Ui::led_pwm_counter_;
uint8_t Ui::pots_multiplexer_address_;
int16_t Ui::adc_values_[4];
int8_t Ui::adc_thresholds_[4];
uint8_t Ui::adc_warm_up_cycles_;
uint8_t Ui::adc_hot_address_;
uint8_t Ui::adc_resume_scan_to_;
uint32_t Ui::idle_time_;
bool Ui::freeze_display_;
Adc Ui::adc_;
EventQueue<> Ui::queue_;
SpiMaster<IOEnableLine, MSB_FIRST, 4> Ui::io_;
SpiMaster<IOEnableLineAux, MSB_FIRST, 4> Ui::aux_io_;
RotaryEncoder<EncoderALine, EncoderBLine, EncoderClickLine> Ui::encoder_;

static const int16_t kAdcThresholdLocked = 32;
static const int16_t kAdcThresholdUnlocked = 8;

/* static */
void Ui::Init() {
  io_.Init();
  aux_io_.Init();
  encoder_.Init();

  lcd.Init();
  display.Init();

  editor.Init();
  queue_.Flush();

  progress_bar_ = 0;
  pots_multiplexer_address_ = 0;

  memset(switch_state_, 0xff, sizeof(switch_state_));
  
  adc_.Init();
  adc_.set_reference(ADC_DEFAULT);
  adc_.set_alignment(ADC_RIGHT_ALIGNED);
  adc_.StartConversion(0);
  pots_multiplexer_address_ = 0;
    
  for (uint8_t i = 0; i < 4; ++i) {
    adc_values_[i] = -1;
    adc_thresholds_[i] = kAdcThresholdUnlocked;
  }
  adc_warm_up_cycles_ = 8;
  adc_hot_address_ = 0;
  adc_resume_scan_to_ = 0;
}

/* static */
void Ui::ShowSplash() {
  if (part.system_settings().start_page == START_PAGE_SPLASH) {
    editor.DisplaySplashScreen(STR_RES_V + 1);
  } else if (part.system_settings().start_page == START_PAGE_LAST_PATCH) {
    editor.BootOnPatchBrowsePage(part.system_settings().last_patch);
  } else {
    editor.Refresh();
  }
}

/* static */
void Ui::LockPotentiometers() {
  adc_thresholds_[0] = kAdcThresholdLocked;
  adc_thresholds_[1] = kAdcThresholdLocked;
  adc_thresholds_[2] = kAdcThresholdLocked;
  adc_thresholds_[3] = kAdcThresholdLocked;
}

/* static */
void Ui::ScanPotentiometers() {
  adc_.Wait();
  uint8_t address = pots_multiplexer_address_;
  uint8_t physical_address = address;
  int16_t value = adc_.ReadOut();
  
  *PortA::Mode::ptr() = 0x00;
  pots_multiplexer_address_ = (pots_multiplexer_address_ + 1) & 3;
  if (adc_resume_scan_to_ == 0) {
    adc_resume_scan_to_ = pots_multiplexer_address_ + 1;
    pots_multiplexer_address_ = adc_hot_address_;
  } else {
    pots_multiplexer_address_ = adc_resume_scan_to_ - 1;
    adc_resume_scan_to_ = 0;
  }
  adc_.StartConversion(pots_multiplexer_address_);
  
  if (address != 0xff) {
    if (adc_warm_up_cycles_) {
      // Register the value and move on.
      adc_values_[address] = value;
      if (address == 0) {
        --adc_warm_up_cycles_;
      }
    } else {
      int16_t previous_value = adc_values_[address];
      int16_t delta = value - previous_value;
      int16_t threshold = adc_thresholds_[address];
      if (delta > threshold || delta < -threshold) {
        adc_hot_address_ = physical_address;
        adc_thresholds_[address] = kAdcThresholdUnlocked;
        queue_.AddEvent(CONTROL_POT, address, value >> 3);
        adc_values_[address] = value;
      }
    }
  }
}

/* static */
void Ui::WriteShiftRegister() {
  led_pwm_counter_ += 16;
  io_.Begin();
  io_.Send(part.pvk_routing_byte());

  io_.Send(led_pattern_);
  io_.End();
}

/* static */
void Ui::DebounceSwitches() {
  io_.Begin();
  io_.End();
  
  uint8_t data = io_.Receive();
  uint8_t mask = 0x20;
  for (uint8_t i = 0; i <= SWITCH_ENCODER; ++i) {
    if (i == SWITCH_ENCODER) {
      switch_state_[i] = encoder_.ReadSwitch();
      continue;
    }
    switch_state_[i] <<= 1;
    if (data & mask) {
      switch_state_[i] |= 1;
    }
    mask >>= 1;
  }
  
  // Handle basic switches. Page keys (S1-S4) act on release, so a hold can
  // show the page's numbers instead; switch_delay_ = 0 means no tap pending.
  for (uint8_t i = 0; i <= SWITCH_MODE; ++i) {
    if (switch_state_[i] == 0x80) {
      if (switch_state_[SWITCH_SHIFT] == 0x00) {
        queue_.AddEvent(CONTROL_SWITCH, i, 0xff);
        switch_delay_[i] = 0;
      } else if (i == SWITCH_MODE) {
        queue_.AddEvent(CONTROL_SWITCH, i, 0);
      } else {
        switch_delay_[i] = milliseconds() | 1;
      }
      if (i == SWITCH_MODE) {
        switch_delay_[i] = milliseconds() | 1;  // Hold timer for perform mode.
      }
    } else if (switch_state_[i] == 0x7f && i != SWITCH_MODE) {
      if (switch_delay_[i] && milliseconds() - switch_delay_[i] <= 300) {
        queue_.AddEvent(CONTROL_SWITCH, i, 0);
      }
      switch_delay_[i] = 0;
    }
  }
  
  // Handle encoder
  if (switch_state_[SWITCH_ENCODER] == 0x80) {
    switch_delay_[SWITCH_ENCODER] = milliseconds() + 1000;
    inhibit_encoder_release_ = false;
  } else if (switch_state_[SWITCH_ENCODER] == 0x00) {
    uint32_t now = milliseconds();
    int32_t delta = now - switch_delay_[SWITCH_ENCODER];
    if (!inhibit_encoder_release_ && delta > 0) {
      queue_.AddEvent(CONTROL_SWITCH, SWITCH_ENCODER, 6);
      inhibit_encoder_release_ = true;
    }
  } else if (switch_state_[SWITCH_ENCODER] == 0x7f) {
    if (!inhibit_encoder_release_) {
      if (switch_state_[SWITCH_SHIFT] == 0x00) {
        // FUNC + click.
        queue_.AddEvent(CONTROL_SWITCH, SWITCH_ENCODER, 1);
      } else {
        queue_.AddEvent(CONTROL_SWITCH, SWITCH_ENCODER, 0);
      }
    }
    inhibit_encoder_release_ = false;
  }
}

/* static */
void Ui::RefreshLeds() {
  // The progress bar overrides the default pattern.
  if (progress_bar_) {
    led_pattern_ = progress_bar_;
    return;
  }

  uint8_t led_pattern = editor.leds_pattern();
  if (editor.current_mode() == EDITOR_MODE_SEQUENCE) {
    if (part.running()) {
      if (!(part.step() & 3)) {
        led_pattern |= LED_MODE_MASK;
      }
    }
  } else {
    if (editor.current_page() == PAGE_MOD_MATRIX) {
      uint8_t current_modulation_source_value = part.modulation_source(0,
          part.patch().modulation_matrix.modulation[
              editor.subpage()].source);
      if (current_modulation_source_value > led_pwm_counter_) {
        led_pattern |= LED_6_MASK;
      }
    } else if (editor.current_page() == PAGE_MOD_OPERATORS) {
      uint8_t current_operator_value = part.modulation_source(0,
          MOD_SRC_OP_1 + editor.subpage());
      if (current_operator_value > led_pwm_counter_) {
        led_pattern |= LED_6_MASK;
      }
    }
  }
  led_pattern_ = led_pattern;
}

/* static */
void Ui::DoEvents() {
  bool refresh_display = false;

  uint8_t previous_page = editor.current_page();
  editor.set_func(switch_state_[SWITCH_SHIFT] == 0x00);

  while (queue_.available()) {
    Event e = queue_.PullEvent();
    queue_.Touch();
    switch (e.control_type) {
      case CONTROL_ENCODER:
        editor.OnIncrement(e.value);
        break;

      case CONTROL_SWITCH:
        if (e.control_id <= SWITCH_LOAD_SAVE) {
          editor.OnSwitch(e);
        } else if (e.control_id == SWITCH_ENCODER) {
          if (e.value == 1) {
            editor.ReloadPatch();
          } else if (e.value < 6) {
            editor.OnClick();  // Long press (6): unused.
          }
        }
        break;

      case CONTROL_POT:
        editor.OnInput(e.control_id, e.value);
        break;
    }
    idle_time_ = 0;
    refresh_display = true;
  }

  // Perform mode: hold SYS (S5) for 1 s to leave.
  if (editor.current_page() == PAGE_PERFORM &&
      switch_state_[SWITCH_MODE] == 0x00 && switch_delay_[SWITCH_MODE] &&
      milliseconds() - switch_delay_[SWITCH_MODE] > 1000) {
    switch_delay_[SWITCH_MODE] = 0;
    editor.ExitPerform();
    refresh_display = true;
  }

  if (editor.current_page() != previous_page) {
    LockPotentiometers();
  }

  uint16_t pause_ms = ((part.system_settings().display_delay & 0xf) << 7) + 100;
  if (queue_.idle_time_ms() > pause_ms) {
    idle_time_ += queue_.idle_time_ms();
    queue_.Touch();
    refresh_display |= editor.Relax();
    bool show_screen_saver = idle_time_ >= 100000 && \
        (part.system_settings().display_delay & 0x10);
    if (show_screen_saver) {
      idle_time_ = 100000;
      editor.ScreenSaver();
    }
    if (part.dirty()) {
      refresh_display = true;
    }
  }
  
  if (queue_.idle_time_ms() > 80 &&
      (editor.FinishLoadingPatch() || part.dirty())) {
    queue_.Touch();
    refresh_display = true;
    idle_time_ = 0;
  }
  
  // Holding a page key shows the page's values as numbers (Elektron style).
  bool numbers = false;
  for (uint8_t i = 0; i < SWITCH_MODE; ++i) {
    numbers |= switch_state_[i] == 0x00 && switch_delay_[i] &&
        milliseconds() - switch_delay_[i] > 300;
  }
  if (numbers != editor.show_numbers()) {
    editor.set_show_numbers(numbers);
    refresh_display = true;
  }

  refresh_display |= editor.PopupDone();

  if (!encoder_.immediate_value() && freeze_display_) {
    refresh_display = false;
  }
  
  // Redraw only once the LCD shows the previous screen: glyph slots are then
  // reused only when no cell displays them (no flicker under fast knob moves).
  static bool refresh_pending;
  refresh_pending |= refresh_display;
  if (refresh_pending && editor.display_idle()) {
    refresh_pending = false;
    editor.Refresh();
  }

  RefreshLeds();
  editor.UploadGlyphs();
  display.Tick();
}

/* extern */
Ui ui;

}  // namespace shruthi
