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
// Patch data.

#include "shruthi/patch.h"
#include "shruthi/part.h"

#include <string.h>

namespace shruthi {

void Patch::PrepareForWrite() {
  version_ = '*';  // '&': tempo-locked LFOs; '\'': no SEQ/CV sources; '(': FX; ')': volume; '*': no env trig destinations.
  filter_topology_ = (filter_1_mode_ << 4) | filter_2_mode_;
  op_data_[0] = ops_[0].operands[0];
  op_data_[1] = (ops_[0].operands[1] << 3) | (ops_[0].op & 0x7);
  op_data_[2] = ops_[1].operands[0];
  op_data_[3] = (ops_[1].operands[1] << 3) | (ops_[1].op & 0x7);
  if (ops_[0].op & 0x8) {
    op_data_[0] |= 0x80;
  }
  if (ops_[1].op & 0x8) {
    op_data_[2] |= 0x80;
  }
  if (ops_[0].operands[1] & 0x20) {
    op_data_[0] |= 0x40;
  }
  if (ops_[1].operands[1] & 0x20) {
    op_data_[2] |= 0x40;
  }
  
  const SystemSettings system_settings = part.system_settings();
  
  extra_data_[0] = system_settings.legato ? 0x40 : 0x00;
  extra_data_[0] |= system_settings.portamento;
  extra_data_[0] |= system_settings.retrigger ? 0x80 : 0x00;
}

uint8_t Patch::CheckBuffer(uint8_t* buffer) {
  for (uint8_t i = 8; i < 24; ++i) {
    if (buffer[i] >= 128) {
      memset(name, '-', 8);
      return 0;
    }
  }
  if (buffer[91] != '!' && buffer[91] != '%' && buffer[91] != '&' &&
      buffer[91] != '\'' && buffer[91] != '(' && buffer[91] != ')' &&
      buffer[91] != '*') {
    memset(name, '-', 8);
    return 0;
  }
  return 1;
}

// Source number before version '\'' (SEQ, SEQ 1-2, STEP, CV 1-4 removed);
// 0xff for a removed source.
static uint8_t RemapSource(uint8_t s) {
  if (s >= 14) {
    return s - 8;
  }
  if (s >= 10) {
    return 0xff;
  }
  if (s >= 6) {
    return s - 4;
  }
  return s >= 2 ? 0xff : s;
}

void Patch::Update() {
  // Horrible hack to set the HPF to 0 on patches saved with previous
  // versions of the firmware.
  if (op_data_[2] == 'n' && op_data_[3] == 'g') {
    memset(saved_data() + 84, 0, 8);
  }
  ops_[0].operands[0] = op_data_[0] & 0x3f;
  ops_[0].operands[1] = op_data_[1] >> 3;
  ops_[0].op = op_data_[1] & 0x7;
  if (op_data_[0] & 0x80) {
    ops_[0].op |= 0x8;
  }
  if (op_data_[0] & 0x40) {
    ops_[0].operands[1] |= 0x20;
  }
  
  ops_[1].operands[0] = op_data_[2] & 0x3f;
  ops_[1].operands[1] = op_data_[3] >> 3;
  ops_[1].op = op_data_[3] & 0x7;
  if (op_data_[2] & 0x80) {
    ops_[1].op |= 0x8;
  }
  if (op_data_[2] & 0x40) {
    ops_[1].operands[1] |= 0x20;
  }
    
  filter_2_mode_ = filter_topology_ & 0xf;
  filter_1_mode_ = filter_topology_ >> 4;

  // ponytail: older LFO settings ('!', '%') are converted by shruthi/patch_translator.py.
  if (version_ != '*' && version_ != ')' && version_ != '(' &&
      version_ != '\'') {
    for (uint8_t i = 0; i < kModulationMatrixSize; ++i) {
      Modulation* m = &modulation_matrix.modulation[i];
      uint8_t s = RemapSource(m->source);
      if (s == 0xff) {
        s = MOD_SRC_LFO_1;  // The removed sources were always 0 here.
        m->amount = 0;
      }
      m->source = s;
    }
    for (uint8_t i = 0; i < 4; ++i) {
      uint8_t* operand = &ops_[i >> 1].operands[i & 1];
      uint8_t s = RemapSource(*operand);
      *operand = s == 0xff ? MOD_SRC_LFO_1 : s;
    }
  }
  if (version_ != '*' && version_ != ')' && version_ != '(') {
    extra_data_[1] = extra_data_[2] = extra_data_[3] = 0;  // FX off.
  }
  if (version_ != '*' && version_ != ')') {
    extra_data_[4] = 127;  // Full volume.
  }
  if (version_ != '*') {
    // Env retrigger left the matrix.
    for (uint8_t i = 0; i < kModulationMatrixSize; ++i) {
      Modulation* m = &modulation_matrix.modulation[i];
      if (m->destination == MOD_DST_TRIGGER_ENV_1 ||
          m->destination == MOD_DST_TRIGGER_ENV_2) {
        m->destination = MOD_DST_PWM_1;
        m->amount = 0;
      }
    }
    version_ = '*';
  }
  
  if (version_ != '!') {
    SystemSettings* system_settings = part.mutable_system_settings();

    system_settings->legato = extra_data_[0] & 0x40 ? 1 : 0;
    system_settings->portamento = extra_data_[0] & 0x3f;
    system_settings->retrigger = extra_data_[0] & 0x80 ? 1 : 0;
  }
}

}  // shruthi
