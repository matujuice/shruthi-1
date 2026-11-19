// Copyright 2010 Emilie Gillet.
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
// This class dispatches received MIDI events to the different components of the
// synth (synthesis engine, display, editor) that might react to them.

#ifndef SHRUTHI_MIDI_DISPATCHER_H_
#define SHRUTHI_MIDI_DISPATCHER_H_

#include "avrlib/base.h"
#include "avrlib/ring_buffer.h"
#include "midi/midi.h"
#include "shruthi/display.h"
#include "shruthi/editor.h"
#include "shruthi/parameter.h"
#include "shruthi/part.h"
#include "shruthi/storage.h"

namespace shruthi {

const uint8_t kDataEntryResendRate = 32;


struct LowPriorityBufferSpecs {
  enum {
    buffer_size = 128,
    data_size = 8,
  };
  typedef avrlib::DataTypeForSize<data_size>::Type Value;
};

class MidiDispatcher : public midi::MidiDevice {
 public:
  typedef avrlib::RingBuffer<LowPriorityBufferSpecs> OutputBuffer;
  MidiDispatcher() { }

  // ------ MIDI in handling ---------------------------------------------------

  // Forwarded to the controller.
  static inline void NoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    display.set_status('\xa5');
    if (!editor.OnNoteOn(note, velocity)) {
      part.NoteOn(channel, note, velocity);
    }
  }
  static inline void NoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
    part.NoteOff(channel, note);
  }

  // Handled.
  static inline void ControlChange(
      uint8_t channel,
      uint8_t controller,
      uint8_t value) {
    if (controller == midi::kBankMsb) {
      current_bank_ = value;
    } else {
      display.set_status('\xdf');
      part.ControlChange(controller, value);
    }
  }
  static inline void PitchBend(uint8_t channel, uint16_t pitch_bend) {
    display.set_status('\xa5');
    part.PitchBend(pitch_bend);
  }
  static void Aftertouch(uint8_t channel, uint8_t note, uint8_t velocity) {
    part.Aftertouch(note, velocity);
  }
  static void Aftertouch(uint8_t channel, uint8_t velocity) {
    part.Aftertouch(velocity);
  }
  static void AllSoundOff(uint8_t channel) {
    part.AllSoundOff();
  }
  static void ResetAllControllers(uint8_t channel) {
    part.ResetAllControllers();
  }
  static void AllNotesOff(uint8_t channel) {
    part.AllNotesOff();
  }
  static void OmniModeOff(uint8_t channel) {
    part.OmniModeOff(channel);
  }
  static void OmniModeOn(uint8_t channel) {
    part.OmniModeOn();
  }
  
  static void ProgramChange(uint8_t channel, uint8_t program) {
    uint16_t n = program + (current_bank_ << 7);
    if (n < Storage::size<Patch>()) {
      editor.set_current_patch_number(n);
    }
  }
  
  static void Reset() { part.Reset(); }
  static void Clock() { part.Clock(); }
  static void Start() { part.Start(); }
  static void Continue() { part.Start(); }
  static void Stop() { part.Stop(); }
  
  static void SysExStart() {
    ProcessSysEx(0xf0);
  }
  static void SysExByte(uint8_t sysex_byte) {
    ProcessSysEx(sysex_byte);
  }
  static void SysExEnd() {
    ProcessSysEx(0xf7);
    if (Storage::sysex_rx_state() == RECEPTION_OK) {
      display.set_status('+');
      part.Touch();
      editor.Refresh();
    } else {
      display.set_status('#');
    }
  }
  
  static uint8_t CheckChannel(uint8_t channel) {
    const SystemSettings& settings = part.system_settings();
    return settings.midi_channel == 0 ||
           settings.midi_channel == (channel + 1);
  }
  
  static void RawMidiData(
      uint8_t status,
      uint8_t* data,
      uint8_t data_size,
      uint8_t accepted_channel) { }  // MIDI out is off or soft thru only.
  
  static void RawByte(uint8_t byte) {
    // Report that some data has been received. The MIDI Out filter might
    // propagate it directly to the output if "Soft Thru" is enabled.
    if (mode() == MIDI_OUT_SOFT_THRU) {
      OutputBuffer::Overwrite(byte);
    }
  }
  
  static uint8_t readable() {
    return OutputBuffer::readable();
  }

  static uint8_t ImmediateRead() {
    return OutputBuffer::ImmediateRead();
  }
  
  // ------ MIDI out handling (patch dumps only) -----------------------------
  static void Flush() {
    running_status_ = 0;
    while (OutputBuffer::readable());
  }
  
  static void Send3(uint8_t a, uint8_t b, uint8_t c);
  static uint8_t channel() {
    return part.system_settings().midi_channel == 0
        ? 0
        : part.system_settings().midi_channel - 1;
  }
  
 private:
  
  static void ProcessSysEx(uint8_t byte) {
    Storage::SysExReceive(byte);
  }
  
  static uint8_t mode() { return part.system_settings().midi_out_mode; }
  
  static uint8_t current_bank_;
  static uint8_t running_status_;
  
  DISALLOW_COPY_AND_ASSIGN(MidiDispatcher);
};

extern MidiDispatcher midi_dispatcher;

}  // namespace shruthi

#endif // SHRUTHI_MIDI_DISPATCHER_H_
