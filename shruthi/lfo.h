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
// LFO (cheap oscillator).
//
// Contrary to oscillators which are templatized "static singletons", to
// generate the fastest, most specialized code, LFOs are less
// performance-sensitive and are thus implemented as a traditional class.

#ifndef SHRUTHI_LFO_H_
#define SHRUTHI_LFO_H_

#include "avrlib/base.h"
#include "shruthi/envelope.h"
#include "shruthi/patch.h"
#include "shruthi/resources.h"
#include "shruthi/sequencer_settings.h"
#include "shruthi/shruthi.h"
#include "avrlib/random.h"
#include "avrlib/op.h"

using avrlib::Random;

namespace shruthi {

// Tempo-locked LFO. The phase is 16.16 fixed point; a negative increment runs
// the waveform backwards.
class Lfo {
 public:
  Lfo() { }

  uint8_t Render() {
    uint8_t value;
    uint16_t phase = phase_ >> 16;
    switch (shape_) {
      case LFO_WAVEFORM_RAMP:
        value = phase >> 8;
        break;

      case LFO_WAVEFORM_S_H:
        if (cycle_complete_) {
          value_ = Random::GetByte();
        }
        value = value_;
        break;

      case LFO_WAVEFORM_SMOOTH:
        if (cycle_complete_) {
          previous_ = value_;
          value_ = Random::GetByte();
        }
        {
          uint8_t t = phase >> 8;  // Time since the last value, either way.
          value = U8Mix(previous_, value_, phase_increment_ < 0 ? ~t : t);
        }
        break;

      case LFO_WAVEFORM_EXP:
        value = Exp(phase);
        break;

      case LFO_WAVEFORM_TRIANGLE:
        value = (phase & 0x8000) ?
            phase >> 7 :
            ~static_cast<uint8_t>(phase >> 7);
        break;

      case LFO_WAVEFORM_SQUARE:
        value = (phase & 0x8000) ? 255 : 0;
        break;

      default:
        {
          uint8_t shape_offset = shape_ - LFO_WAVEFORM_WAVE_1;
          if (shape_offset == 0) {
            shape_offset = 3;
          } else {
            shape_offset += 16;
          }
          value = ResourcesManager::Lookup<uint8_t, uint8_t>(
              wav_res_waves + U8U8Mul(shape_offset, 129),
              phase >> 9);
        }
        break;
    }

    // Advance. ONE stops after a full cycle, HLF after half a cycle (the
    // phase MSB flips); the phase stays on the last value before the end.
    uint32_t next = phase_ + phase_increment_;
    bool wrapped = phase_increment_ < 0 ? next > phase_ : next < phase_;
    cycle_complete_ = false;
    if (running_ || mode_ < LFO_MODE_ONE_SHOT) {
      if ((mode_ == LFO_MODE_ONE_SHOT && wrapped) ||
          (mode_ == LFO_MODE_HALF && ((next ^ phase_) & 0x80000000))) {
        running_ = false;
      } else {
        phase_ = next;
        cycle_complete_ = wrapped;
      }
    }
    last_value_ = value;
    return mode_ == LFO_MODE_HOLD ? held_value_ : value;
  }

  // 1 - t, to the 4th power: close to e^(-5.5 t).
  static inline uint8_t Exp(uint16_t phase) {
    uint8_t p = ~(phase >> 8);
    p = U8U8MulShift8(p, p);
    return U8U8MulShift8(p, p);
  }

  void Reset() {
    ResetPhase();
    Trigger();
  }

  void ResetPhase() {
    phase_ = phase_increment_ < 0 ? 0xffffffff : 0;
    cycle_complete_ = true;
    running_ = true;
  }

  // Note on.
  void Trigger() {
    if (mode_ == LFO_MODE_HOLD) {
      held_value_ = last_value_;
    } else if (mode_ != LFO_MODE_FREE) {
      ResetPhase();
    }
  }

  void Update(uint8_t shape, int32_t phase_increment, uint8_t mode) {
    shape_ = shape;
    phase_increment_ = phase_increment;
    mode_ = mode;
  }

 private:
  int32_t phase_increment_;
  uint32_t phase_;
  uint8_t shape_;
  uint8_t mode_;
  bool cycle_complete_;
  bool running_;
  uint8_t value_;  // S&H.
  uint8_t previous_;  // Smooth random: the value it glides from.
  uint8_t last_value_;
  uint8_t held_value_;

  DISALLOW_COPY_AND_ASSIGN(Lfo);
};

}  // namespace shruthi

#endif // SHRUTHI_LFO_H_
