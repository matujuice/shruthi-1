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
  version_ = '+';  // '&': tempo-locked LFOs; '\'': no SEQ/CV sources; '(': FX; ')': volume; '*': no CC7; '+': FX page (8 FX, sub switch).
  // PVK: any mode but LP plays as BP (voice.cc), and storage keeps 1 bit of it.
  filter_topology_ = ((filter_1_mode_ != FILTER_MODE_LP) << 4) | filter_2_mode_;
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
  if (extra_data_[1] & 4) {
    op_data_[0] |= 0x20;
  }
  if (extra_data_[5]) {
    op_data_[2] |= 0x20;
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
  // Older formats are converted offline by shruthi/patch_translator.py.
  if (buffer[91] != '+') {
    memset(name, '-', 8);
    return 0;
  }
  return 1;
}

void Patch::Update() {
  ops_[0].operands[0] = op_data_[0] & 0x1f;
  ops_[0].operands[1] = op_data_[1] >> 3;
  ops_[0].op = op_data_[1] & 0x7;
  if (op_data_[0] & 0x80) {
    ops_[0].op |= 0x8;
  }
  if (op_data_[0] & 0x40) {
    ops_[0].operands[1] |= 0x20;
  }
  
  ops_[1].operands[0] = op_data_[2] & 0x1f;
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

  // Bit 5 of op_data_[0] / [2]: FX type bit 2 / sub through the FX.
  extra_data_[1] = (extra_data_[1] & 3) | ((op_data_[0] & 0x20) >> 3);
  extra_data_[5] = op_data_[2] & 0x20 ? 1 : 0;
  // Out-of-range indices would make the voice read/write past its arrays.
  for (uint8_t i = 0; i < kModulationMatrixSize; ++i) {
    Modulation* m = &modulation_matrix.modulation[i];
    if (m->source >= kNumModulationSources ||
        m->destination >= kNumModulationDestinations) {
      m->source = m->destination = m->amount = 0;
    }
  }
  for (uint8_t i = 0; i < 4; ++i) {
    uint8_t* operand = &ops_[i >> 1].operands[i & 1];
    if (*operand >= kNumModulationSources) {
      *operand = 0;
    }
  }
  for (uint8_t i = 0; i < 2; ++i) {
    if (osc[i].shape >= WAVEFORM_LAST) {
      osc[i].shape = 0;  // Else the wavetable lookup reads past its table.
    }
  }
  
  SystemSettings* system_settings = part.mutable_system_settings();
  system_settings->legato = extra_data_[0] & 0x40 ? 1 : 0;
  system_settings->portamento = extra_data_[0] & 0x3f;
  system_settings->retrigger = extra_data_[0] & 0x80 ? 1 : 0;
}

}  // shruthi
