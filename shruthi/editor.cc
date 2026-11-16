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
// Patch editor.
//
// I really hate this code. Work on a "UI" framework to avoid such horrible
// things in progress...

#include "shruthi/editor.h"

#include "avrlib/deprecated/devices/switch_array.h"
#include "shruthi/display.h"
#include "shruthi/midi_dispatcher.h"
#include "shruthi/parameter.h"
#include "shruthi/storage.h"
#include "shruthi/part.h"
#include "shruthi/ui.h"
#include "avrlib/string.h"

using namespace avrlib;
using namespace avrlib;

using avrlib::kLcdNoCursor;

namespace shruthi {

/* extern */
Editor editor;

/* static */
const UiHandler Editor::ui_handler_[] = {
  { &Editor::DisplayEditOverviewPage, &Editor::DisplayEditOverviewPage,
    &Editor::OnEditInput, &Editor::OnEditIncrement, NULL },
  { &Editor::DisplayLoadSavePage, &Editor::DisplayLoadSavePage,
    &Editor::OnEditInput, &Editor::OnLoadSaveIncrement,
    &Editor::OnLoadSaveClick },
  { &Editor::DisplayConfirmPage, &Editor::DisplayConfirmPage,
    &Editor::OnConfirmInput, &Editor::OnConfirmIncrement,
    &Editor::OnConfirmClick },
  { &Editor::DisplayJamPage, &Editor::DisplayJamPage,
    &Editor::OnJamPageInput, &Editor::OnJamPageIncrement,
    &Editor::OnJamPageClick },
};

// LEDs: S1 = LED_1/LED_2, S2 = LED_3, S3 = LED_4/LED_5, S4 = LED_6, S5 = MODE,
// S6 = WRITE. FUNC pages add the S6 (FUNC) LED.
PageDefinition Editor::page_definition_[] = {
  /* PAGE_OSC_OSC_1 */ { PAGE_OSC_OSC_2, GROUP_SYN, PARAMETER_EDITOR, LED_1_MASK },
  /* PAGE_OSC_OSC_2 */ { PAGE_OSC_OSC_MIX, GROUP_SYN, PARAMETER_EDITOR, LED_2_MASK },
  /* PAGE_OSC_OSC_MIX */ { PAGE_OSC_OSC_1, GROUP_SYN, PARAMETER_EDITOR,
    LED_1_MASK | LED_2_MASK },
  /* PAGE_FILTER_FILTER */ { PAGE_MOD_ENV_1, GROUP_FLTR, PARAMETER_EDITOR, LED_3_MASK },
  /* PAGE_MOD_ENV_1 */ { PAGE_FILTER_FILTER, GROUP_FLTR, PARAMETER_EDITOR, LED_3_MASK },
  /* PAGE_MOD_ENV_2 */ { PAGE_MOD_ENV_2, GROUP_AMP, PARAMETER_EDITOR, LED_4_MASK },
  /* PAGE_MOD_LFO_1 */ { PAGE_MOD_LFO_2, GROUP_MOD, PARAMETER_EDITOR, LED_6_MASK },
  /* PAGE_MOD_LFO_2 */ { PAGE_MOD_LFO_1, GROUP_MOD, PARAMETER_EDITOR, LED_6_MASK },
  /* PAGE_FX */ { PAGE_FX, GROUP_FX, PARAMETER_EDITOR, LED_1_MASK | LED_2_MASK | LED_WRITE_MASK },
  /* PAGE_FILTER_MULTIMODE */ { PAGE_FILTER_MULTIMODE, GROUP_VCF,
    PARAMETER_EDITOR, LED_3_MASK | LED_WRITE_MASK },
  /* PAGE_VOICE */ { PAGE_VOICE, GROUP_VOICE, PARAMETER_EDITOR, LED_5_MASK | LED_WRITE_MASK },
  /* PAGE_MOD_MATRIX */ { PAGE_MOD_OPERATORS, GROUP_MOD_MATRIX,
    PARAMETER_EDITOR, LED_6_MASK | LED_WRITE_MASK },
  /* PAGE_MOD_OPERATORS */ { PAGE_MOD_MATRIX, GROUP_MOD_MATRIX,
    PARAMETER_EDITOR, LED_6_MASK | LED_WRITE_MASK },
  /* PAGE_SYS_KBD */ { PAGE_SYS_MIDI, GROUP_SYS, PARAMETER_EDITOR,
    LED_MODE_MASK | LED_WRITE_MASK },
  /* PAGE_SYS_MIDI */ { PAGE_SYS_DISPLAY, GROUP_SYS, PARAMETER_EDITOR,
    LED_MODE_MASK | LED_WRITE_MASK },
  /* PAGE_SYS_DISPLAY */ { PAGE_SYS_KBD, GROUP_SYS, PARAMETER_EDITOR,
    LED_MODE_MASK | LED_WRITE_MASK },
  /* PAGE_LOAD_SAVE */ { PAGE_LOAD_SAVE, GROUP_LOAD_SAVE, LOAD_SAVE, LED_MODE_MASK },
  /* PAGE_CONFIRM */ { PAGE_CONFIRM, GROUP_CONFIRM, CONFIRM, 0x55 },
  /* PAGE_JAM */ { PAGE_JAM, GROUP_JAM, JAM, 0x00 },
};

// Parameter ids for the 4 pots of each parameter page. 0xff = unused;
// 0x80 | k = k-th parameter of the expansion filter board block.
static const prog_uint8_t page_parameters[][4] PROGMEM = {
  { 0, 1, 2, 3 },  // OSC 1
  { 4, 5, 6, 7 },  // OSC 2
  { 9, 10, 11, 0xff },  // MIXER
  { 12, 13, 14, 15 },  // FILTER
  { 16, 17, 18, 19 },  // FLT ENV
  { 20, 21, 22, 23 },  // AMP ENV
  { 24, 25, 26, 27 },  // LFO 1
  { 28, 29, 30, 31 },  // LFO 2
  { 8, 9, 0xff, 0xff },  // FX
  { 0x81, 0x82, 0x83, 0xff },  // POLIVOKS
  { 46, 47, 44, 0xff },  // VOICE
  { 32, 33, 34, 35 },  // MATRIX
  { 58, 59, 60, 61 },  // MATH
  { 44, 45, 46, 47 },  // SYS KBD
  { 48, 49, 50, 0xff },  // SYS MIDI
  { 52, 53, 0xff, 55 },  // SYS DISPLAY (filter board fixed: PVK)
};

static const prog_char page_labels[] PROGMEM =
  "shaprmrngsub"  // 0
  "shaprmrngtun"  // 1
  "mixsubnoi   "  // 2
  "cutresenvlfo"  // 3
  "atkdecsusrel"  // 4
  "atkdecsusrel"  // 5
  "wavspdmulmod"  // 6
  "wavspdmulmod"  // 7
  "op mix      "  // 8
  "modovdfm    "  // 9
  "prtlegoct   "  // 10
  "sltsrcdstamt"  // 11
  "op in1in2out"  // 12
  "octragprtleg"  // 13
  "tunchnout   "  // 14
  "pausnpfltsta";  // 15

static const prog_char page_titles[] PROGMEM =
  "SYN  OSC 1   1/3"
  "SYN  OSC 2   2/3"
  "SYN  MIXER   3/3"
  "FLTR FILTER  1/2"
  "FLTR FLT ENV 2/2"
  "AMP  AMP ENV    "
  "MOD  LFO 1   1/2"
  "MOD  LFO 2   2/2"
  "FX              "
  "POLIVOKS        "
  "VOICE           "
  "MATRIX       1/2"
  "MATH         2/2"
  "SYS  KBD     1/3"
  "SYS  MIDI    2/3"
  "SYS  DISPLAY 3/3";

static const prog_char lfo_mode_names[] PROGMEM = "FRETRGHLDONEHLF";

static const prog_char osc_names[] PROGMEM = "OFFSAWSQRTRIZSWZRSZTRZPLZSYPAD FMWAVTMPDIGMETBOWSLPORGMALUSR8BTCRUPWMNOIVOWBELPOLCELCLPFEMFMTFRMRESELEVIB";

static const prog_uint8_t icons[] PROGMEM = {
  0x00, 0x01, 0x03, 0x05, 0x09, 0x11, 0x00, 0x00,  // saw
  0x00, 0x1d, 0x15, 0x15, 0x15, 0x17, 0x00, 0x00,  // sqr
  0x00, 0x09, 0x09, 0x09, 0x09, 0x1f, 0x00, 0x00,  // pul
  0x00, 0x00, 0x04, 0x0a, 0x11, 0x00, 0x00, 0x00,  // tri
  0x00, 0x18, 0x08, 0x0b, 0x0a, 0x0e, 0x00, 0x00,  // rnd
  0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x00,  // off
  0x00, 0x06, 0x09, 0x09, 0x11, 0x11, 0x00, 0x00,  // zsw
  0x00, 0x08, 0x14, 0x15, 0x02, 0x00, 0x00, 0x00,  // zrs
  0x00, 0x00, 0x02, 0x05, 0x08, 0x10, 0x00, 0x00,  // ztr
  0x00, 0x10, 0x1d, 0x15, 0x17, 0x00, 0x00, 0x00,  // zpl
  0x00, 0x01, 0x03, 0x0b, 0x1d, 0x09, 0x00, 0x00,  // zsy
  0x00, 0x08, 0x15, 0x02, 0x08, 0x15, 0x02, 0x00,  // pad
  0x0e, 0x0a, 0x0e, 0x04, 0x0e, 0x0a, 0x0e, 0x00,  // fm
  0x00, 0x03, 0x02, 0x0e, 0x08, 0x18, 0x00, 0x00,  // bit
  0x00, 0x1d, 0x11, 0x15, 0x11, 0x17, 0x00, 0x00,  // pwm
  0x00, 0x12, 0x05, 0x08, 0x12, 0x05, 0x00, 0x00,  // noi
  0x00, 0x00, 0x0e, 0x11, 0x0e, 0x00, 0x00, 0x00,  // vow
  0x00, 0x08, 0x14, 0x15, 0x05, 0x02, 0x00, 0x00,  // wtb
  0x00, 0x04, 0x04, 0x04, 0x04, 0x1f, 0x00, 0x00,  // clk
  0x00, 0x18, 0x06, 0x13, 0x0c, 0x03, 0x00, 0x00,  // glt
  0x00, 0x06, 0x01, 0x1e, 0x10, 0x0f, 0x00, 0x00,  // blw
  0x00, 0x04, 0x0e, 0x0e, 0x1f, 0x04, 0x00, 0x00,  // mtl
  0x00, 0x15, 0x0e, 0x1b, 0x0e, 0x15, 0x00, 0x00,  // pop
  0x00, 0x07, 0x04, 0x04, 0x04, 0x1c, 0x00, 0x00,  // fuzz
  0x00, 0x0c, 0x0a, 0x1a, 0x03, 0x01, 0x00, 0x00,  // cru
  0x00, 0x04, 0x04, 0x1f, 0x04, 0x04, 0x00, 0x00,  // sum
  0x00, 0x0e, 0x1b, 0x15, 0x1b, 0x0e, 0x00, 0x00,  // rng
  0x00, 0x0e, 0x15, 0x1f, 0x15, 0x0e, 0x00, 0x00,  // xor
  0x00, 0x0a, 0x15, 0x11, 0x00, 0x00, 0x00, 0x00,  // fld
  0x00, 0x0f, 0x09, 0x09, 0x1b, 0x1b, 0x00, 0x00,  // duo
  0x00, 0x02, 0x02, 0x0a, 0x0a, 0x0a, 0x00, 0x00,  // st2
  0x00, 0x08, 0x0a, 0x1a, 0x1e, 0x1e, 0x00, 0x00,  // st4
  0x00, 0x11, 0x15, 0x15, 0x1f, 0x00, 0x00, 0x00,  // st8
  0x00, 0x1c, 0x02, 0x02, 0x01, 0x01, 0x00, 0x00,  // lpf
  0x00, 0x0e, 0x11, 0x11, 0x11, 0x11, 0x00, 0x00,  // bpf
};

// Icon index for each value of the units that have icons. 0xff = no icon.
static const prog_uint8_t icon_map[] PROGMEM = {
  5, 0, 1, 3, 6, 7, 8, 9, 10, 11, 12, 17, 17, 17, 17, 17, 17, 17, 17, 17, 13, 24, 14, 15, 16, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
  1, 3, 2, 1, 3, 2, 18, 19, 20, 21, 22,
  25, 10, 26, 27, 23, 24, 24, 28, 13, 29, 30, 31, 32, 0xff,
  3, 1, 4, 0, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
  33, 34,
};

// Custom glyphs: allocated per screen, uploaded to CGRAM by UploadGlyphs().
static uint8_t glyph_[8][8];
static uint8_t glyph_dirty_ = 0xff;
static uint8_t glyph_used_;  // Slots used by the screen being drawn.
static uint8_t glyph_previous_;  // Slots used by the previous screen.

// A glyph keeps its slot while it stays on screen, and a new one goes to a
// slot the LCD is not showing, so a redraw never changes a visible cell.
static char GlyphGet(const uint8_t* bits) {
  for (uint8_t i = 0; i < 8; ++i) {
    if (!memcmp(glyph_[i], bits, 8)) {
      glyph_used_ |= 1 << i;
      return 8 + i;
    }
  }
  uint8_t slot = 0xff;
  for (uint8_t i = 0; i < 8; ++i) {
    uint8_t mask = 1 << i;
    if (!(glyph_used_ & mask) &&
        (slot == 0xff || !(glyph_previous_ & mask))) {
      slot = i;
      if (!(glyph_previous_ & mask)) {
        break;
      }
    }
  }
  if (slot == 0xff) {  // Pages are laid out to need at most 8.
    return ' ';
  }
  memcpy(glyph_[slot], bits, 8);
  glyph_dirty_ |= 1 << slot;
  glyph_used_ |= 1 << slot;
  return 8 + slot;
}

static char Icon(uint8_t unit, uint8_t value) {
  uint8_t base, count;
  switch (unit) {
    case UNIT_WAVEFORM: base = 0; count = 35; break;
    case UNIT_SUB_OSC_WAVEFORM: base = 35; count = 11; break;
    case UNIT_OPERATOR: base = 46; count = 14; break;
    case UNIT_LFO_WAVEFORM: base = 60; count = 20; break;
    case UNIT_FILTER_1_MODE: base = 80; count = 2; break;
    default: return ' ';
  }
  uint8_t icon = value < count ? pgm_read_byte(icon_map + base + value) : 0xff;
  if (icon == 0xff) {
    return ' ';
  }
  uint8_t bits[8];
  memcpy_P(bits, icons + 8 * icon, 8);
  return GlyphGet(bits);
}

/* <static> */
ParameterPage Editor::current_page_ = PAGE_FILTER_FILTER;
ParameterPage Editor::last_visited_page_[] = {
    PAGE_OSC_OSC_1,
    PAGE_FILTER_FILTER,
    PAGE_MOD_ENV_2,
    PAGE_MOD_LFO_1,
    PAGE_FX,
    PAGE_FILTER_MULTIMODE,
    PAGE_VOICE,
    PAGE_MOD_MATRIX,
    PAGE_SYS_KBD,
    PAGE_LOAD_SAVE,
    PAGE_CONFIRM,
    PAGE_JAM
};
uint8_t Editor::last_visited_subpage_ = 0;
uint8_t Editor::display_mode_ = DISPLAY_MODE_OVERVIEW;
uint8_t Editor::editor_mode_ = EDITOR_MODE_PATCH;
uint8_t Editor::last_visited_group_[] = {
    GROUP_FLTR,
    GROUP_SYS
};

char Editor::line_buffer_[kLcdWidth * kLcdHeight + 1];

uint8_t Editor::cursor_;
uint8_t Editor::last_knob_;
uint8_t Editor::subpage_;
uint8_t Editor::action_;
uint16_t Editor::current_patch_number_ = 0;

int8_t Editor::jam_note_ = 0;
int8_t Editor::jam_note_root_ = 60;
int8_t Editor::jam_mode_shifts_[4] = { 2, 5, 7, 4 };
uint8_t Editor::jam_mode_previous_page_;

ConfirmPageSettings Editor::confirm_page_settings_;
bool Editor::snapped_[4];
bool Editor::title_flash_;
bool Editor::show_numbers_;
bool Editor::empty_patch_;
uint16_t Editor::deferred_load_;
/* </static> */

/* static */
void Editor::Init() {
  parameter_manager.Init();
  
  line_buffer_[kLcdWidth] = '\0';
  deferred_load_ = 0xffff;
}

/* static */
void Editor::PageChange() {
  if (current_page_ == PAGE_MOD_MATRIX) {
    subpage_ = last_visited_subpage_;
  }
  uint8_t group = page_definition_[current_page_].group;
  last_visited_page_[group] = current_page_;
  // Remembered for FUNC tap = back to the main pages.
  if (group < GROUP_FX) {
    last_visited_group_[0] = group;
  }
  title_flash_ = current_page_ <= PAGE_SYS_DISPLAY;
  
  if (page_definition_[current_page_].ui_type == PARAMETER_EDITOR) {
    memset(snapped_, false, sizeof(snapped_));
  }
  display.set_cursor_position(kLcdNoCursor);
}

/* static */
void Editor::JumpToPageGroup(uint8_t group) {
  cursor_ = 0;
  display_mode_ = DISPLAY_MODE_OVERVIEW;
  subpage_ = 0;
  // If we move to another group, go to the last visited page in this group.
  if (group != page_definition_[current_page_].group) {
    current_page_ = last_visited_page_[group];
  } else {
    current_page_ = page_definition_[current_page_].next;
  }
  // When switching to the modulation matrix page, go back to the previously
  // edited modulation.
  PageChange();
}

/* static */
bool Editor::Relax() {
  if (title_flash_) {
    title_flash_ = false;
    return true;
  }
  // Disable the "get back to overview page" thing in the sequencer pages or
  // Load/save pages - in short, all pages where there's a moving cursor.
  if (display_mode_ == DISPLAY_MODE_EDIT_TEMPORARY &&
      display.cursor_position() == kLcdNoCursor) {
    display_mode_ = DISPLAY_MODE_OVERVIEW;
    return true;
  } else {
    return false;
  }
}

/* static */
void Editor::ShowEditCursor() {
  display.set_cursor_position(kLcdWidth + cursor_);
  display.set_cursor_character(
      display_mode_ == DISPLAY_MODE_EDIT ? kLcdEditCursor : kLcdCursor);
}

/* static */
void Editor::RandomizeParameter(uint8_t subpage, uint8_t parameter_index) {
  const Parameter& parameter = parameter_manager.parameter(parameter_index);
  uint8_t value = parameter.RandomValue();
  part.SetParameter(
      parameter_index,
      parameter.offset + subpage * 3,
      value,
      true);
}

/* static */
void Editor::RandomizePatch() {
  // Randomize all the main parameters
  for (uint8_t parameter = 0; parameter < 32; ++parameter) {
    RandomizeParameter(0, parameter);
  }
  // Randomize the first 8th modulation slots. Keep the last one to avoid
  // silent patches in which a bogus modulation source is routed to the VCA.
  for (uint8_t slot = 0; slot < kModulationMatrixSize - 4; ++slot) {
    for (uint8_t parameter = 33; parameter < 36; ++parameter) {
      RandomizeParameter(slot, parameter);
    }
  }
  part.Touch(true);
}

/* static */
void Editor::RandomizeSequence() {
  for (uint8_t i = 0; i < kNumSteps; ++i) {
    SequencerSettings* settings = part.mutable_sequencer_settings();
    settings->steps[i].set_raw(
        Random::GetByte(),
        Random::GetByte());
    settings->steps[i].set_note(
        (settings->steps[i].note() & 0x3f) + 0x20);
  }
}

/* static */
void Editor::SaveSystemSettings() {
  part.mutable_system_settings()->EepromSave();
}

/* static */
void Editor::StartMidiBackup() {
  Storage::SysExBulkDump();
}

/* static */
void Editor::Confirm(ConfirmPageSettings confirm_page_settings) {
  confirm_page_settings_ = confirm_page_settings;
  current_page_ = PAGE_CONFIRM;
  display_mode_ = DISPLAY_MODE_OVERVIEW;
  Refresh();
}

/* static */
void Editor::OnSwitch(const Event& event) {
  uint8_t id = event.control_id;
  if (event.value == 0xff) {
    // FUNC + key.
    if (current_page_ == PAGE_LOAD_SAVE && id < SWITCH_MODE) {
      switch (id) {
        case SWITCH_1:
          display.set_status('x');
          part.ResetPatch();
          part.ResetSequence();
          break;

        case SWITCH_2:
          display.set_status('?');
          RandomizePatch();
          RandomizeSequence();
          break;

        case SWITCH_3:
          display.set_status('>');
          Storage::SysExDump(part.mutable_patch());
          Storage::SysExDump(part.mutable_sequencer_settings());
          break;

        case SWITCH_4:
          {
            ConfirmPageSettings confirm_midi_backup;
            confirm_midi_backup.text = STR_RES_START_FULL_MIDI;
            confirm_midi_backup.return_group = GROUP_SYN;
            confirm_midi_backup.callback = &StartMidiBackup;
            Confirm(confirm_midi_backup);
          }
          break;
      }
    } else if (id == SWITCH_MODE) {
      JumpToPageGroup(GROUP_SYS);
    } else {
      JumpToPageGroup(id + GROUP_FX);
    }
  } else if (id == SWITCH_MODE) {
    ToggleLoadSaveAction();
  } else if (id == SWITCH_LOAD_SAVE) {
    // FUNC tap: back to the last main page.
    if (page_definition_[current_page_].group != last_visited_group_[0]) {
      JumpToPageGroup(last_visited_group_[0]);
    }
  } else {
    JumpToPageGroup(id + GROUP_SYN);
  }
}

/* static */
void Editor::OnInput(uint8_t knob_index, uint8_t value) {
  (*ui_handler_[page_definition_[current_page_].ui_type].input_handler)(
      knob_index, value);
}

/* static */
void Editor::OnIncrement(int8_t increment) {
  (*ui_handler_[page_definition_[current_page_].ui_type].increment_handler)(
      increment);
}

/* static */
void Editor::OnLongClick() {
  if (!part.latched()) {
    if (part.num_notes() == 0) {
      UpdateJamNote();
      jam_mode_previous_page_ = current_page_;
      display_mode_ = DISPLAY_MODE_OVERVIEW;
      current_page_ = PAGE_JAM;
    }
    part.Latch();
  } else {
    if (jam_note_ && current_page_ != PAGE_JAM) {
      display_mode_ = DISPLAY_MODE_OVERVIEW;
      current_page_ = PAGE_JAM;
    } else {
      part.Unlatch();
      if (jam_note_) {
        part.NoteOn(0, jam_note_, 0);
        jam_note_ = 0;
        if (current_page_ == PAGE_JAM) {
          current_page_ = jam_mode_previous_page_;
        }
      }
    }
  }
}

/* static */
void Editor::OnClick() {
  if (current_page_ >= PAGE_SYS_KBD && current_page_ <= PAGE_SYS_DISPLAY) {
    ConfirmPageSettings confirm_save_system_settings;
    confirm_save_system_settings.text = STR_RES_SAVE_MIDI_KBD;
    confirm_save_system_settings.return_group = GROUP_SYS;
    confirm_save_system_settings.callback = &SaveSystemSettings;
    Confirm(confirm_save_system_settings);
    return;
  }
  title_flash_ = false;
  if (display_mode_ != DISPLAY_MODE_EDIT) {
    display_mode_ = DISPLAY_MODE_EDIT;
  } else {
    display_mode_ = DISPLAY_MODE_OVERVIEW;
  }
  if (ui_handler_[page_definition_[current_page_].ui_type].click_handler) {
    (*ui_handler_[page_definition_[current_page_].ui_type].click_handler)();
  }
}

/* static */
uint8_t Editor::OnNoteOn(uint8_t note, uint16_t velocity) {
  return 0;
}

/* static */
void Editor::Refresh() {
  if (display_mode_ == DISPLAY_MODE_OVERVIEW) {
    (*ui_handler_[page_definition_[current_page_].ui_type].overview_page)();
  } else {
    (*ui_handler_[page_definition_[current_page_].ui_type].edit_page)();
  }
}

/* static */
void Editor::ScreenSaver() {
  memset(line_buffer_, ' ', kLcdWidth);
  display.Print(0, line_buffer_);
  display.Print(1, line_buffer_);
}

/* static */
void Editor::RestoreEditBuffer() {
  Storage::Restore(part.mutable_patch());
  part.Touch(true);
  Storage::Restore(part.mutable_sequencer_settings());
}

/* static */
void Editor::BackupEditBuffer() {
  Storage::Backup(part.mutable_patch());
  Storage::Backup(part.mutable_sequencer_settings());
}

/* static */
void Editor::ToggleLoadSaveAction() {
  empty_patch_ = false;
  if (current_page_ != PAGE_LOAD_SAVE) {
    BackupEditBuffer();
    action_ = ACTION_LOAD;
  } else {
    if (action_ != ACTION_SAVE) {
      action_ = ACTION_SAVE;
      set_edited_item_number(edited_item_number());
    } else {
      action_ = ACTION_COMPARE;
    }
  }
  cursor_ = 0;
  display.set_cursor_position(kLcdNoCursor);
  display_mode_ = DISPLAY_MODE_OVERVIEW;
  current_page_ = PAGE_LOAD_SAVE;
}

/* static */
uint16_t Editor::edited_item_number() {
  return current_patch_number_;
}

const prog_uint8_t default_name[] PROGMEM = "user    ";

/* static */
void Editor::set_edited_item_number(int16_t value) {
  if (value >= static_cast<int16_t>(Storage::size<Patch>())) {
    value = 0;
  } else if (value < 0) {
    value = Storage::size<Patch>() - 1;
  }
  current_patch_number_ = value;
  
  uint8_t name[8];
  Storage::LoadPatchName(name, edited_item_number());
  empty_patch_ = true;
  for (uint8_t i = 0; i < 8; ++i) {
    if (name[i] != pgm_read_byte(default_name + i)) {
      empty_patch_ = false;
    }
  }
}

/* static */
uint8_t Editor::is_cursor_at_valid_position() {
  uint16_t allowed_cursor_positions = 0x0ff7;
  return ((1 << cursor_) & allowed_cursor_positions) != 0;
}

/* static */
void Editor::OnLoadSaveClick() {
  if (action_ == ACTION_LOAD) {
    action_ = ACTION_COMPARE;
    RestoreEditBuffer();
  } else if (action_ == ACTION_COMPARE) {
    action_ = ACTION_LOAD;
    OnLoadSaveIncrement(0);
  } else {
    if (!is_cursor_at_valid_position()) {
      display_mode_ = DISPLAY_MODE_OVERVIEW;
    }
    if (cursor_ >= kLcdWidth - 4) {
      Storage::WritePatch(edited_item_number());
      Storage::WriteSequence(edited_item_number());
      action_ = ACTION_LOAD;
    }
  }
}

/* static */
bool Editor::FinishLoadingPatch() {
  if (deferred_load_ == 0xffff) {
    return false;
  }
  uint16_t n = deferred_load_;
  Storage::LoadPatch(n);
  midi_dispatcher.OnProgramChange(n);
  part.Touch(true);
  if (part.system_settings().start_page == START_PAGE_LAST_PATCH) {
    part.mutable_system_settings()->last_patch = n;
    part.system_settings().EepromSave();
  }
  // When we are not playing, load the sequence parameters.
  if (!part.running()) {
    Storage::LoadSequence(edited_item_number());
  }
  // Reset the snap position of all pots.
  memset(snapped_, false, sizeof(snapped_));
  deferred_load_ = 0xffff;
  return true;
}

/* static */
void Editor::OnLoadSaveIncrement(int8_t increment) {
  if (action_ == ACTION_SAVE) {
    if (display_mode_ == DISPLAY_MODE_OVERVIEW) {
      int8_t new_cursor = static_cast<int8_t>(cursor_) + increment;
      if (new_cursor >= 0 && new_cursor < kLcdWidth) {
        cursor_ = new_cursor;
      }
    } else {
      if (cursor_ <= 2) {
        set_edited_item_number(edited_item_number() + increment);
      } else if (cursor_ >= 4 && cursor_ < 4 + kPatchNameSize) {
        uint8_t value = part.patch().name[cursor_ - 4];
        value += increment;
        if (value >= 32 && value <= 128) {
          part.mutable_patch()->name[cursor_ - 4] = value;
        }
      }
    }
  } else {
    set_edited_item_number(edited_item_number() + increment);
    if (action_ == ACTION_LOAD) {
      deferred_load_ = edited_item_number();
    }
  }
}

/* static */
void Editor::DisplayLoadSavePage() {
  if (display_mode_ == DISPLAY_MODE_EDIT_TEMPORARY
      && action_ != ACTION_SAVE
      && (part.system_settings().display_delay & 0xf)) {
    DisplayEditOverviewPage();
    return;
  }
  ResourcesManager::LoadStringResource(
      STR_RES_BROWSE + action_,
      line_buffer_,
      8);
  ResourcesManager::LoadStringResource(
      STR_RES_PATCH,
      line_buffer_ + 8,
      kLcdWidth - 8);
  AlignLeft(line_buffer_, 8);
  AlignLeft(line_buffer_ + 8, kLcdWidth - 8);
  line_buffer_[7] = ':';
  display.Print(0, line_buffer_);

  UnsafeItoa<int16_t>(edited_item_number() + 1, 3, line_buffer_);
  AlignRight(line_buffer_, 3);
  memcpy(line_buffer_ + 4, part.patch().name, kPatchNameSize);
  memset(
        line_buffer_ + 4 + kPatchNameSize,
        ' ',
        kLcdWidth - kPatchNameSize - 4);
  line_buffer_[3] = empty_patch_ ? '*' : ' ';
  if (action_ == ACTION_SAVE) {
    line_buffer_[kLcdWidth - 3] = 'o';
    line_buffer_[kLcdWidth - 2] = 'k';
    if (cursor_ >= kLcdWidth - 4) {
      line_buffer_[kLcdWidth - 4] = '[';
      line_buffer_[kLcdWidth - 1] = ']';
    }
  }
  if (action_ == ACTION_SAVE && is_cursor_at_valid_position()) {
    ShowEditCursor();
  } else {
    display.set_cursor_position(kLcdNoCursor);
  }
  display.Print(1, line_buffer_);
}

/* static */
void Editor::UploadGlyphs() {
  if (!glyph_dirty_ || lcd.writable() < 18) {
    return;
  }
  uint8_t slot = 0;
  while (!(glyph_dirty_ & (1 << slot))) {
    ++slot;
  }
  glyph_dirty_ &= ~(1 << slot);
  lcd.WriteCommand(0x40 | (slot << 3));
  for (uint8_t i = 0; i < 8; ++i) {
    lcd.WriteData(glyph_[slot][i]);
  }
  // The LCD address counter now points to CGRAM.
  display.InvalidateCursor();
}

/* static */
char Editor::PrintParameter(
    uint8_t id, char* out, uint8_t width, char* label, bool touched) {
  const Parameter& p = parameter_manager.parameter(id);
  uint8_t value = GetParameterValue(p.offset);
  char text[8];
  memset(text, 0, sizeof(text));
  char* t = text;
  bool is_signed = p.unit == UNIT_INT8 &&
      static_cast<int8_t>(p.min_value) < 0;
  if (is_signed && static_cast<int8_t>(value) > 0 && value < 100) {
    *t++ = '+';
  }
  if (p.unit == UNIT_WAVEFORM) {
    memcpy_P(t, osc_names + 3 * value, 3);
  } else if (p.unit == UNIT_LFO_RETRIGGER_MODE) {
    memcpy_P(t, lfo_mode_names + 3 * value, 3);
  } else if (p.unit == UNIT_LFO_RATE) {  // LFO multiplier: x1..x2k.
    *t++ = 'x';  // x128/x256/x512 spill into the meter column.
    if (value >= 10) {
      *t++ = value == 10 ? '1' : '2';
      *t = 'k';
    } else {
      UnsafeItoa<int16_t>(1 << value, 4, t);
    }
  } else {
    p.PrintValue(value, t, width - (t - text));
  }
  uint8_t len = 0;
  while (len < width && text[len]) {
    ++len;
  }
  if (p.unit == UNIT_LFO_RATE && text[width]) {
    memcpy(out, text, width + 1);
    return text[width];
  }
  if (p.unit > UNIT_INT8) {
    memset(out, ' ', width);
    memcpy(out + width - len, text, len);
    return Icon(p.unit, value);
  }
  if (id == 2 || id == 6) {  // Osc range: plain number, keeps glyphs for bars.
    memset(out, ' ', 4);
    memcpy(out, text, len);
    return out[3];
  }
  // Numeric: 19 px box. Unipolar: inset fill; bipolar: 2 px marker, 2 dots
  // at 0. The number replaces the label while touched or FUNC is held.
  if (touched || show_numbers_) {
    while (text[len]) {
      ++len;
    }
    memset(label, ' ', 4);
    memcpy(label, text, len);
  }
  int16_t num = value - p.min_value;
  int16_t den = p.max_value - p.min_value;
  if (is_signed) {
    num = static_cast<int8_t>(value) - static_cast<int8_t>(p.min_value);
    den = static_cast<int8_t>(p.max_value) - static_cast<int8_t>(p.min_value);
  }
  if (den <= 0) {
    den = 1;
  }
  bool dots = false;
  uint8_t from = 2, to;  // Lit pixel columns on rows 2-5; empty if to < from.
  if (!is_signed && id != 9) {  // 9 = osc mix: 32 is the equal mix.
    to = 1 + (15 * num + den / 2) / den;
  } else if (num == (den + 1) / 2) {
    dots = true;
    to = 0;
  } else {
    if (id != 9) {
      from = 2 + (13 * num + den / 2) / den;
    } else {
      // Mix shares MIXER with two fills: 11 positions that skip px 4, 9, 14
      // keep the marker inside one cell, so the page fits in 8 glyphs.
      for (uint8_t j = (10 * num + den / 2) / den; j; --j) {
        if (++from % 5 == 4) {
          ++from;
        }
      }
    }
    to = from + 1;
  }
  // Built row-wise: the full per-pixel version was slow enough to glitch audio.
  for (uint8_t k = 0; k < 4; ++k) {
    uint8_t fill = 0, dot = 0;
    for (uint8_t x = 0; x < 5; ++x) {
      uint8_t px = 5 * k + x;
      fill = (fill << 1) | (px >= from && px <= to);
      dot = (dot << 1) | (dots && px == 9);
    }
    uint8_t edge = (k == 0 ? 0x10 : 0) | (k == 3 ? 0x02 : 0);
    uint8_t bits[8];
    bits[0] = bits[7] = k == 3 ? 0x1e : 0x1f;  // px 19 stays empty.
    bits[1] = bits[6] = edge;
    bits[2] = bits[5] = edge | fill | dot;
    bits[3] = bits[4] = edge | fill;
    out[k] = GlyphGet(bits);
  }
  return out[3];
}

/* static */
void Editor::DisplayEditOverviewPage() {
  // SHA PRM RNG SUB
  // SAW#  0# +0#SQ1#
  char* row_0 = line_buffer_;
  char* row_1 = line_buffer_ + kLcdWidth + 1;
  memset(line_buffer_, ' ', sizeof(line_buffer_));
  row_0[kLcdWidth] = '\0';
  glyph_previous_ = glyph_used_;
  glyph_used_ = 0;
  // Encoder target: steady arrow when browsing, blinking when editing.
  uint8_t marker = cursor_ * kColumnWidth + 3;
  if (title_flash_) {
    marker = kLcdNoCursor;
    memcpy_P(row_0, page_titles + kLcdWidth * current_page_, kLcdWidth);
  } else {
    uint8_t previous = 0xff;
    for (uint8_t i = 0; i < kNumEditingPots; ++i) {
      uint8_t id = KnobIndexToParameterId(i);
      if (id == 0xff) {
        continue;
      }
      uint8_t offset = parameter_manager.parameter(id).offset;
      if (offset == previous) {
        continue;
      }
      previous = offset;
      char* label = row_0 + i * kColumnWidth;
      memcpy_P(label, page_labels + 12 * current_page_ + 3 * i, 3);
      char* value = row_1 + i * kColumnWidth;
      value[3] = PrintParameter(id, value, 3, label,
          i == cursor_ && display_mode_ != DISPLAY_MODE_OVERVIEW);
    }
    // If we are in latched mode, show it.
    if (part.latched()) {
      for (uint8_t i = 1; i < kLcdWidth; ++i) {
        if (row_0[i] == ' ') {
          row_0[i] = '-';
        }
      }
    }
  }
  for (uint8_t i = 0; i < 2 * kLcdWidth + 1; ++i) {
    char c = line_buffer_[i];
    if (c == '\0' && i != kLcdWidth) {
      line_buffer_[i] = ' ';
    } else if (c >= 'a' && c <= 'z' && i > kLcdWidth) {  // Labels: Title case.
      line_buffer_[i] = c - 32;
    }
  }
  if (display_mode_ == DISPLAY_MODE_EDIT) {
    display.set_cursor_character(0x7f);
  } else {
    if (marker != kLcdNoCursor) {
      line_buffer_[marker + (marker >= kLcdWidth)] = 0x7f;  // HD44780 left arrow.
    }
    marker = kLcdNoCursor;
  }
  display.set_cursor_position(marker);
  display.Print(0, row_0);
  display.Print(1, row_1);
}

/* static */
uint8_t Editor::KnobIndexToParameterId(uint8_t knob_index) {
  if (current_page_ == PAGE_LOAD_SAVE) {
    if (action_ != ACTION_LOAD) {
      return 0xff;
    }
    switch (knob_index) {
      case 0:
        return PRM_OSC_PARAMETER_1;
      case 1:
        return PRM_FILTER_CUTOFF;
      case 2:
        return PRM_FILTER_RESONANCE;
      default:
        return PRM_FILTER_ENV;
    }
  }
  if (current_page_ > PAGE_SYS_DISPLAY) {
    return 0xff;
  }
  uint8_t id = pgm_read_byte(&page_parameters[current_page_][knob_index]);
  if (id != 0xff && (id & 0x80)) {
    // Expansion board n has its 4 parameters at 58 + 4 * n.
    id = 58 + 4 * FILTER_BOARD_PVK + (id & 0x7f);
  }
  return id;
}

/* static */
void Editor::OnEditInput(uint8_t knob_index, uint8_t value) {
  title_flash_ = false;
  uint8_t index = KnobIndexToParameterId(knob_index);
  if (index == 0xff) {
    return;
  }
  cursor_ = knob_index;
  const Parameter& parameter = parameter_manager.parameter(index);
  
  if (part.system_settings().display_snap) {
    if (!snapped_[knob_index]) {
      uint8_t current_value = GetParameterValue(parameter.offset);
      if (parameter.is_snapped(current_value, value)) {
        snapped_[knob_index] = true;
      } else {
        return;
      }
    }
  }
  if (display_mode_ != DISPLAY_MODE_EDIT) {
    display_mode_ = DISPLAY_MODE_EDIT_TEMPORARY;
  }
  SetParameterValue(index, parameter.offset, parameter.Scale(value));
}

/* static */
void Editor::OnEditIncrement(int8_t increment) {
  title_flash_ = false;
  if (display_mode_ == DISPLAY_MODE_EDIT) {
    uint8_t id = KnobIndexToParameterId(cursor_);
    if (id != 0xff) {
      IncrementParameterValue(id, increment);
    }
    return;
  }
  // Browse: move to the next used column of this page, no page change.
  int8_t c = cursor_;
  do {
    c += increment > 0 ? 1 : -1;
  } while (c >= 0 && c < 4 && KnobIndexToParameterId(c) == 0xff);
  if (c >= 0 && c < 4) {
    cursor_ = c;
  }
  display_mode_ = DISPLAY_MODE_OVERVIEW;
}

/* static */
void Editor::IncrementParameterValue(uint8_t index, int8_t increment) {
  const Parameter& parameter = parameter_manager.parameter(index);
  uint8_t old_value = GetParameterValue(parameter.offset);
  uint8_t new_value = parameter.Increment(old_value, increment);
  if (new_value != old_value) {
    SetParameterValue(index, parameter.offset, new_value);
  }
}

const prog_uint8_t modulation_destination_map[] PROGMEM = {
  MOD_DST_PWM_1,
  MOD_DST_PWM_2,
  MOD_DST_VCO_1,
  MOD_DST_VCO_2,
  MOD_DST_VCO_1_2_COARSE,
  MOD_DST_VCO_1_2_FINE,
  MOD_DST_MIX_BALANCE,
  MOD_DST_MIX_NOISE,
  MOD_DST_MIX_SUB_OSC,
  MOD_DST_FILTER_CUTOFF,
  MOD_DST_FILTER_RESONANCE,
  MOD_DST_VCA,
  MOD_DST_CV_1,
  MOD_DST_CV_2,
  MOD_DST_TRIGGER_ENV_1,
  MOD_DST_ATTACK_1,
  MOD_DST_DECAY_1,
  MOD_DST_SUSTAIN_1,
  MOD_DST_RELEASE_1,
  MOD_DST_TRIGGER_ENV_2,
  MOD_DST_ATTACK_2,
  MOD_DST_DECAY_2,
  MOD_DST_SUSTAIN_2,
  MOD_DST_RELEASE_2,
  MOD_DST_ATTACK,
  MOD_DST_LFO_1,
  MOD_DST_LFO_2,
};

/* static */
void Editor::SetParameterValue(
    uint8_t index,
    uint8_t offset,
    uint8_t value) {
  // Dirty hack for the modulation page.
  if (current_page_ == PAGE_MOD_MATRIX && offset == PRM_MOD_ROW) {
    subpage_ = value;
    last_visited_subpage_ = value;
  } else if (current_page_ == PAGE_MOD_OPERATORS && offset == PRM_OP_ROW ) {
    subpage_ = value;
  } else {
    if (offset == PRM_MOD_DESTINATION) {
      value = pgm_read_byte(modulation_destination_map + value);
    }
    if ((offset >= PRM_OP_OP1 && offset <= PRM_OP_OPERATOR) ||
        (offset >= PRM_MOD_SOURCE && offset <= PRM_MOD_AMOUNT)) {
      offset += subpage_ * 3;
    }
    part.SetParameter(index, offset, value, true);
  }
}

/* static */
uint8_t Editor::GetParameterValue(uint8_t offset) {
  uint8_t value;
  if (current_page_ == PAGE_MOD_MATRIX && offset == PRM_MOD_ROW) {
    value = subpage_;
  } else if (current_page_ == PAGE_MOD_OPERATORS && offset == PRM_OP_ROW ) {
    value = subpage_;
  } else {
    uint8_t actual_offset = offset;
    if ((actual_offset >= PRM_OP_OP1 && actual_offset <= PRM_OP_OPERATOR) ||
        (actual_offset >= PRM_MOD_SOURCE && actual_offset <= PRM_MOD_AMOUNT)) {
      actual_offset += subpage_ * 3;
    }
    value = part.GetParameter(actual_offset);
    if (offset == PRM_MOD_DESTINATION) {
      for (uint8_t i = 0; i < MOD_DST_LAST; ++i) {
        if (pgm_read_byte(modulation_destination_map + i) == value) {
          value = i;
          break;
        }
      }
    }
  }
  return value;
}

/* static */
void Editor::DisplaySplashScreen(ResourceId first_line) {
  // 0123456789abcdef
  // mutable
  // instruments sh-1
  for (uint8_t i = 0; i < 2; ++i) {
    ResourcesManager::LoadStringResource(
        first_line + i,
        line_buffer_,
        kLcdWidth);
    AlignLeft(line_buffer_, kLcdWidth);
    display.Print(i, line_buffer_);
  }
}

/* static */
void Editor::OnConfirmClick() {
  if (cursor_) {
    (*confirm_page_settings_.callback)();
  }
  JumpToPageGroup(confirm_page_settings_.return_group);
  Refresh();
}

/* static */
void Editor::OnConfirmIncrement(int8_t increment) {
  cursor_ = !cursor_;
}

/* static */
void Editor::OnConfirmInput(uint8_t knob_index, uint8_t value) {
}

/* static */
void Editor::DisplayConfirmPage() {
  display.set_cursor_position(kLcdNoCursor);
  for (uint8_t i = 0; i < 2; ++i) {
    ResourcesManager::LoadStringResource(
        confirm_page_settings_.text + i,
        line_buffer_,
        kLcdWidth);
    AlignLeft(line_buffer_, kLcdWidth);
    if (i == 1) {
      if (cursor_) {
        line_buffer_[kLcdWidth - 2] = 'o';
        line_buffer_[kLcdWidth - 1] = 'k';
      } else {
        line_buffer_[kLcdWidth - 2] = 'n';
        line_buffer_[kLcdWidth - 1] = 'o';
      }
    }
    display.Print(i, line_buffer_);
  }
}

/* static */
void Editor::DisplayJamPage() {
  ResourcesManager::LoadStringResource(
      STR_RES_LETS_JAM_,
      line_buffer_,
      kLcdWidth);
  AlignLeft(line_buffer_, kLcdWidth);
  display.Print(0, line_buffer_);
  line_buffer_[0] = 'r';
  line_buffer_[1] = 'o';
  line_buffer_[2] = 'o';
  line_buffer_[3] = 't';
  line_buffer_[4] = ' ';
  SequencerSettings::PrintNote(jam_note_, &line_buffer_[5]);
  line_buffer_[8] = '\0';
  AlignLeft(line_buffer_, kLcdWidth);
  display.Print(1, line_buffer_);
}


const prog_uint8_t octave_shift[] PROGMEM = {
  -24, -12, 0, 12, 24
};

const prog_uint8_t pentatonic_shift[] PROGMEM = {
  -12, -10, -8, -5, -3, 0, 2, 4, 7, 9, 12
};

const prog_uint8_t bhairav_shift[] PROGMEM = {
  -12, -11, -8, -7, -5, -4, -1, 0, 1, 4, 5, 7, 8, 11, 12
};

const prog_uint8_t chromatic_shift[] PROGMEM = {
  -12, -5 , -2, -1, 0, 1, 2, 7, 12
};

const prog_uint8_t ranges[] PROGMEM = {
  5,
  sizeof(pentatonic_shift),
  sizeof(bhairav_shift),
  sizeof(chromatic_shift)
};

void Editor::OnJamPageInput(uint8_t knob_index, uint8_t value) {
  value = U8U8MulShift8(pgm_read_byte(ranges + knob_index), value << 1);
  jam_mode_shifts_[knob_index] = value;
  UpdateJamNote();
}

void Editor::UpdateJamNote() {
  int16_t note = jam_note_root_;
  note += static_cast<int8_t>(pgm_read_byte(octave_shift + jam_mode_shifts_[0]));
  note += static_cast<int8_t>(pgm_read_byte(pentatonic_shift + jam_mode_shifts_[1]));
  note += static_cast<int8_t>(pgm_read_byte(bhairav_shift + jam_mode_shifts_[2]));
  note += static_cast<int8_t>(pgm_read_byte(chromatic_shift + jam_mode_shifts_[3]));
  while (note < 12) {
    note += 12;
  }
  while (note > 96) {
    note -= 12;
  }
  if (note != jam_note_) {
    if (jam_note_) {
      part.NoteOn(0, jam_note_, 0);
    }
    part.NoteOn(0, note, 100);
    jam_note_ = note;
  }
}

void Editor::OnJamPageIncrement(int8_t increment) {
  int16_t new_root = jam_note_root_ + increment;
  if (new_root < 0) {
    new_root = 0;
  } else if (new_root > 127) {
    new_root = 127;
  }
  jam_note_root_ = new_root;
  UpdateJamNote();
}

void Editor::OnJamPageClick() {
  OnLongClick();
}

/* static */
void Editor::BootOnPatchBrowsePage(uint16_t index) {
  if (index >= Storage::size<Patch>()) {
    index = Storage::size<Patch>() - 1;
  } 
  current_patch_number_ = index;
  current_page_ = 0;
  ToggleLoadSaveAction();
  OnLoadSaveIncrement(0);
  Refresh();
}

}  // namespace shruthi
