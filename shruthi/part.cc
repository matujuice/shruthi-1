// Copyright 2009 Emilie Gillet.
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
// Main synthesis engine.

#include "shruthi/part.h"

#include <string.h>

#include "avrlib/op.h"
#include "avrlib/random.h"
#include "avrlib/resources_manager.h"

#include "shruthi/audio_out.h"
#include "shruthi/midi_dispatcher.h"
#include "shruthi/parameter.h"
#include "shruthi/storage.h"

using namespace avrlib;

namespace shruthi {

/* extern */
Part part;

/* <static> */
uint8_t Part::data_access_byte_[1];
Patch Part::patch_;
SequencerSettings Part::sequencer_settings_;
SystemSettings Part::system_settings_;
bool Part::dirty_;

Voice Part::voice_;
NoteStack Part::pressed_keys_;
NoteStack Part::mono_allocator_;
bool Part::release_latched_keys_on_next_note_on_;
bool Part::ignore_note_off_messages_;

uint8_t Part::data_entry_msb_;
uint8_t Part::nrpn_parameter_number_;
uint8_t Part::nrpn_parameter_number_msb_;

Lfo Part::lfo_[kNumLfos];
uint8_t Part::previous_lfo_fm_[kNumLfos];
uint16_t Part::blocks_per_beat_ = 490;  // 120 BPM until the first beat is measured.
uint16_t Part::block_count_;
uint8_t Part::beat_tick_;

/* </static> */

/* static */
void Part::Init() {
  ResetPatch();
  if (!system_settings_.EepromLoad()) {
    ResetSystemSettings();
    system_settings_.EepromSave();
  }
  if (system_settings_.midi_out_mode > MIDI_OUT_SOFT_THRU) {
    system_settings_.midi_out_mode = MIDI_OUT_SOFT_THRU;  // Old ctrl/full/chain.
  }
  Reset();
  pressed_keys_.Init();
  mono_allocator_.Init();
  voice_.Init();
  dirty_ = false;
  nrpn_parameter_number_ = 0xff;
}

// TORTURE TEST v2
// static const prog_char init_patch[] PROGMEM = {
//     // Oscillators
//     WAVEFORM_SAW, 0, 0, 4,
//     WAVEFORM_SAW, 0, -12, 12,
//     // Mixer
//     32, 8, 8, WAVEFORM_SUB_OSC_SQUARE_1,
//     // Filter
//     96, 0, 32, 0,
//     // ADSR
//     0, 50, 20, 60,
//     0, 40, 90, 30,
//     // LFO
//     LFO_WAVEFORM_TRIANGLE, 80, 0, 0,
//     LFO_WAVEFORM_TRIANGLE, 3, 0, 0,
//     // Routing
//     MOD_SRC_LFO_1, MOD_DST_VCO_1, 1,
//     MOD_SRC_ENV_1, MOD_DST_VCO_2, 1,
//     MOD_SRC_ENV_1, MOD_DST_PWM_1, 1,
//     MOD_SRC_LFO_1, MOD_DST_PWM_2, 1,
//     MOD_SRC_LFO_2, MOD_DST_MIX_BALANCE, 1,
//     MOD_SRC_SEQ, MOD_DST_MIX_BALANCE, 1,
//     MOD_SRC_CV_1, MOD_DST_PWM_1, 0,
//     MOD_SRC_CV_2, MOD_DST_PWM_2, 0,
//     MOD_SRC_ENV_2, MOD_DST_VCA, 63,
//     MOD_SRC_VELOCITY, MOD_DST_VCA, 16,
//     MOD_SRC_PITCH_BEND, MOD_DST_VCO_1_2_COARSE, 32,
//     MOD_SRC_LFO_1, MOD_DST_VCO_1_2_COARSE, 16,
//     // Name
//     'i', 'n', 'i', 't', ' ', ' ', ' ', ' ',
//     // Performance page assignments.
//     0, 0,
//     0, 0,
//     0, 0,
//     0, 0,
//     // Settings for second filter
//     0, 0, 0, 
//     0, 0, 0, 0,
//     '!',
//     0, 0, 0, 0, 0, 0, 0, 0,
// };

static const prog_char init_patch[] PROGMEM = {
    // Oscillators
    WAVEFORM_SAW, 0, 0, 0,
    WAVEFORM_SAW, 16, -12, 12,
    // Mixer
    32, 0, 0, WAVEFORM_SUB_OSC_SQUARE_1,
    // Filter
    96, 0, 32, 0,
    // ADSR
    0, 50, 20, 60,
    0, 40, 90, 30,
    // LFO
    LFO_WAVEFORM_TRIANGLE, 32, 4, 0,
    LFO_WAVEFORM_TRIANGLE, 32, 4, 0,
    // Routing
    MOD_SRC_LFO_1, MOD_DST_VCO_1, 0,
    MOD_SRC_ENV_1, MOD_DST_VCO_2, 0,
    MOD_SRC_ENV_1, MOD_DST_PWM_1, 0,
    MOD_SRC_LFO_1, MOD_DST_PWM_2, 0,
    MOD_SRC_LFO_2, MOD_DST_MIX_BALANCE, 0,
    MOD_SRC_LFO_2, MOD_DST_MIX_BALANCE, 0,
    MOD_SRC_VELOCITY, MOD_DST_PWM_1, 0,
    MOD_SRC_VELOCITY, MOD_DST_PWM_2, 0,
    MOD_SRC_ENV_2, MOD_DST_VCA, 63,
    MOD_SRC_VELOCITY, MOD_DST_VCA, 16,
    MOD_SRC_PITCH_BEND, MOD_DST_VCO_1_2_COARSE, 32,
    MOD_SRC_LFO_1, MOD_DST_VCO_1_2_COARSE, 16,
    // Name
    'i', 'n', 'i', 't', ' ', ' ', ' ', ' ',
    // Performance page assignments.
    0, 0,
    0, 0,
    0, 0,
    0, 0,
    // Settings for second filter
    0, 0, 0, 
    0, 0, 0, 0,
    '(',
    0, 0, 0, 0, 0, 0, 0, 0,
};

static const prog_char init_system_settings[] PROGMEM = {
    // System Settings,
    0, 0, 0, 0,
    0, 1, 1, 0,
    8, 0, 0, 0,
    0, 0, 0,
};

/* static */
void Part::ResetPatch() {
  ResourcesManager::Load(init_patch, 0, &patch_);
  Touch(true);
}

/* static */
void Part::Touch(bool cascade) {
  UpdateModulationRates();
  dirty_ = true;
}

/* static */
void Part::ResetSystemSettings() {
  ResourcesManager::Load(init_system_settings, 0, &system_settings_);
}

/* static */
void Part::NoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
  if (!velocity) {
    NoteOff(channel, note);
    return;
  }
  
  if (release_latched_keys_on_next_note_on_) {
    bool still_latched = ignore_note_off_messages_;
    
    // Releasing all latched key will generate "fake" NoteOff messages. We
    // should note ignore them.
    ignore_note_off_messages_ = false;
    ReleaseLatchedNotes();
    
    release_latched_keys_on_next_note_on_ = still_latched;
    ignore_note_off_messages_ = still_latched;
  }
  
  pressed_keys_.NoteOn(note, velocity);
  
  InternalNoteOn(note, velocity);
}

/* static */
void Part::NoteOff(uint8_t channel, uint8_t note) {
  if (ignore_note_off_messages_) {
    for (uint8_t i = 1; i <= pressed_keys_.max_size(); ++i) {
      // Flag the note so that it is removed once the sustain pedal is released.
      NoteEntry* e = pressed_keys_.mutable_note(i);
      if (e->note == note && e->velocity) {
        e->velocity |= 0x80;
      }
    }
  } else {
    pressed_keys_.NoteOff(note);
    InternalNoteOff(note);
  }
}

/* static */
void Part::ControlChange(uint8_t controller, uint8_t value) {
  switch (controller) {
    case midi::kHoldPedal:
      if (value >= 64) {
        ignore_note_off_messages_ = true;
      } else {
        ignore_note_off_messages_ = false;
        ReleaseLatchedNotes();
      }
      break;
      
    case midi::kDataEntryMsb:
      data_entry_msb_ = value << 7;
      break;
    
    case midi::kNrpnLsb:
      nrpn_parameter_number_ = value | nrpn_parameter_number_msb_;
      nrpn_parameter_number_msb_ = 0;
      data_entry_msb_ = 0;
      break;
    case midi::kNrpnMsb:
      nrpn_parameter_number_msb_ = value << 7;
      data_entry_msb_ = 0;
      break;
    
    case midi::kDataEntryLsb:
    case midi::kDataIncrement:
    case midi::kDataDecrement:
      value |= data_entry_msb_;
      if (nrpn_parameter_number_ != 0xff) {
        uint8_t parameter_index = parameter_manager.at_offset(
            nrpn_parameter_number_);
        if (parameter_index != 0xff) {
          dirty_ = true;
          const Parameter& p = parameter_manager.parameter(parameter_index);
          if (controller == midi::kDataEntryLsb) {
            if (p.unit == UNIT_INT8) {
              int8_t signed_value = static_cast<int8_t>(value);
              if (signed_value >= static_cast<int8_t>(p.min_value) &&
                  signed_value <= static_cast<int8_t>(p.max_value)) {
                SetParameter(parameter_index, nrpn_parameter_number_, value, 0);
              }
            } else {
              if (value >= p.min_value && value <= p.max_value) {
                SetParameter(parameter_index, nrpn_parameter_number_, value, 0);
              }
            }
          } else {
            const Parameter& p = parameter_manager.parameter(parameter_index);
            uint8_t old_value = GetParameter(nrpn_parameter_number_);
            uint8_t value = p.Increment(
                old_value,
                controller == midi::kDataIncrement ? 1 : -1);
            if (value != old_value) {
              SetParameter(parameter_index, nrpn_parameter_number_, value, 0);
            }
          }
        }
      }
      break;
      
    default:
      {
        uint8_t index = parameter_manager.for_cc(controller);
        if (index == 0xff) {
          voice_.ControlChange(controller, value);
        } else {
          SetScaledParameter(index, value, false);          
        }
      }
      break;
  }
}

/* static */
void Part::AllSoundOff() {
  ignore_note_off_messages_ = false;
  AllNotesOff();
}

/* static */
void Part::ResetAllControllers() {
  ignore_note_off_messages_ = false;
  voice_.ResetAllControllers();
}

// When in Omni mode, disable Omni and enable reception only on the channel on
// which this message has been received.
/* static */
void Part::OmniModeOff(uint8_t channel) {
  system_settings_.midi_channel = channel + 1;
}

// Enable Omni mode.
/* static */
void Part::OmniModeOn() {
  system_settings_.midi_channel = 0;
}

/* static */
void Part::Reset() {
  ResetAllControllers();
  for (uint8_t i = 0; i < kNumLfos; ++i) {
    lfo_[i].Reset();
  }
  AllNotesOff();
}

/* static */
void Part::Clock() {
  // Measure the beat (24 ticks) in control-rate blocks; LFO speeds lock to it.
  if (++beat_tick_ >= 24) {
    beat_tick_ = 0;
    if (block_count_) {
      blocks_per_beat_ = block_count_;
    }
    block_count_ = 0;
    UpdateModulationRates();
  }
}
/* static */
void Part::Start() {
  release_latched_keys_on_next_note_on_ = false;
  ignore_note_off_messages_ = false;
}

/* static */
void Part::Stop() {
  ignore_note_off_messages_ = false;
}

/* static */
void Part::SetName(uint8_t* name) {
  memcpy(patch_.name, name, kPatchNameSize);
  dirty_ = true;
}

/* static */
void Part::SetScaledParameter(
    uint8_t parameter_index,
    uint8_t value,
    bool user_initiated) {
  dirty_ = true;
  const Parameter& parameter = parameter_manager.parameter(parameter_index);
  SetParameter(
      parameter_index,
      parameter.offset,
      parameter.Scale(value),
      user_initiated);
}

/* static */
void Part::SetParameter(
    uint8_t parameter_index,
    uint8_t offset,
    uint8_t value,
    bool user_initiated) {
  if (data_access_byte_[offset + 1] == value) {
    return;
  }
  data_access_byte_[offset + 1] = value;
  if (offset >= PRM_ENV_ATTACK_1 && offset <= PRM_LFO_RETRIGGER_2) {
    UpdateModulationRates();
  }
}

/* static */
void Part::UpdateModulationRates() {
  // Update the LFO increments.
  for (uint8_t i = 0; i < kNumLfos; ++i) {
    UpdateLfoRate(i);
  }
}

/* static */
void Part::UpdateLfoRate(uint8_t i) {
  const LfoSettings& lfo = patch_.lfo[i];
  int16_t speed = lfo.speed;
  speed += voice_.modulation_destination(MOD_DST_LFO_1 + i);
  speed -= 32;
  // Clamp: old patches stored the fade time in this byte.
  uint8_t multiplier = lfo.multiplier > 11 ? 11 : lfo.multiplier;
  uint32_t p = static_cast<uint32_t>(speed < 0 ? -speed : speed) << multiplier;
  if (p > 131071) {
    p = 131071;  // Keeps p << 15 in 32 bits.
  }
  // Cycles per beat = p / 512, so the 16-bit phase moves 128 * p / beat
  // blocks per block; computed with 8 extra bits, then scaled to 16.16.
  uint32_t increment = (p << 15) / blocks_per_beat_;
  if (increment > 0x400000) {
    increment = 0x400000;  // A quarter cycle per block.
  }
  increment <<= 8;
  lfo_[i].Update(
      lfo.waveform,
      speed < 0 ? -static_cast<int32_t>(increment) : increment,
      lfo.mode);
}

/* static */
void Part::ProcessBlock() {
  if (block_count_ != 0xffff) {
    ++block_count_;
  }
  for (uint8_t i = 0; i < kNumLfos; ++i) {
    voice_.set_modulation_source(MOD_SRC_LFO_1 + i, lfo_[i].Render());
  }
  voice_.set_modulation_source(MOD_SRC_NOISE, Random::state_msb());

  // Update the modulation speed if some of the LFO FM parameters have changed.
  for (uint8_t i = 0; i < kNumLfos; ++i) {
    if (previous_lfo_fm_[i] !=
       voice_.modulation_destination(MOD_DST_LFO_1 + i)) {
      previous_lfo_fm_[i] = voice_.modulation_destination(MOD_DST_LFO_1 + i);
      UpdateLfoRate(i);
    }
  }

  voice_.ProcessBlock();
}

/* static */
void Part::AllNotesOff() {
  if (ignore_note_off_messages_) {
    // This is a fix for the Roland machines that send the all notes off
    // message every time all keys in a chord are released.
    return;
  }
  mono_allocator_.Clear();
  pressed_keys_.Clear();
  voice_.NoteOff();
  release_latched_keys_on_next_note_on_ = false;
  ignore_note_off_messages_ = false;
}

/* static */
void Part::ReleaseLatchedNotes() {
  for (uint8_t i = 1; i <= pressed_keys_.max_size(); ++i) {
    NoteEntry* e = pressed_keys_.mutable_note(i);
    if (e->velocity & 0x80) {
      NoteOff(midi_dispatcher.channel(), e->note);
    }
  }
}

/* static */
uint16_t Part::Tune(uint8_t note) {
  return U8U8Mul(note, 128);
}

/* static */
void Part::InternalNoteOn(uint8_t note, uint8_t velocity) {
  const NoteEntry& before = mono_allocator_.most_recent_note();
  mono_allocator_.NoteOn(note, velocity);
  const NoteEntry& after = mono_allocator_.most_recent_note();
  if (before.note != after.note) {
    bool legato = mono_allocator_.size() > 1;
    voice_.NoteOn(
        Tune(after.note),
        after.velocity,
        !system_settings_.legato || legato ? system_settings_.portamento : 0,
        !system_settings_.legato || !legato);
    uint8_t bass_note = mono_allocator_.least_recent_note().note;
    voice_.set_bass_note(bass_note != after.note ? Tune(bass_note) : 0);
  }
}

/* static */
void Part::InternalNoteOff(uint8_t note) {
  const NoteEntry& before = mono_allocator_.most_recent_note();
  mono_allocator_.NoteOff(note);
  const NoteEntry& after = mono_allocator_.most_recent_note();
  if (mono_allocator_.size() == 0) {
    voice_.NoteOff();
  } else if (before.note != after.note && patch_.osc[0].option != OP_DUO) {
    voice_.NoteOn(
        Tune(after.note),
        after.velocity,
        system_settings_.portamento,
        !system_settings_.legato);
  }
}

}  // namespace shruthi
