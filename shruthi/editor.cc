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

#include <avr/eeprom.h>

#include "avrlib/deprecated/devices/switch_array.h"
#include "shruthi/display.h"
#include "shruthi/midi_dispatcher.h"
#include "shruthi/oscillator.h"
#include "shruthi/parameter.h"
#include "shruthi/storage.h"
#include "shruthi/part.h"
#include "shruthi/ui.h"
#include "avrlib/string.h"
#include "avrlib/time.h"

using namespace avrlib;
using namespace avrlib;

using avrlib::kLcdNoCursor;

// In shruthi.cc. Called between overview cells so a full redraw does not
// starve the audio buffer (the '!' glitch indicator).
void FillAudioBuffer();

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
  { &Editor::DisplaySysPage, &Editor::DisplaySysPage,
    &Editor::OnSysInput, &Editor::OnSysIncrement, &Editor::OnSysClick },
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
  /* PAGE_FILTER_MULTIMODE */ { PAGE_FX_2, GROUP_VCF,
    PARAMETER_EDITOR, LED_3_MASK | LED_WRITE_MASK },
  /* PAGE_FX_2 */ { PAGE_FILTER_MULTIMODE, GROUP_VCF,
    PARAMETER_EDITOR, LED_3_MASK | LED_WRITE_MASK },
  /* PAGE_VOICE */ { PAGE_VOICE, GROUP_VOICE, PARAMETER_EDITOR, LED_5_MASK | LED_WRITE_MASK },
  /* PAGE_MOD_MATRIX */ { PAGE_MOD_OPERATORS, GROUP_MOD_MATRIX,
    PARAMETER_EDITOR, LED_6_MASK | LED_WRITE_MASK },
  /* PAGE_MOD_OPERATORS */ { PAGE_MOD_MATRIX, GROUP_MOD_MATRIX,
    PARAMETER_EDITOR, LED_6_MASK | LED_WRITE_MASK },
  /* PAGE_SYS_MIDI */ { PAGE_SYS_DISPLAY, GROUP_SYS, SYS_MENU,
    LED_MODE_MASK | LED_WRITE_MASK },
  /* PAGE_SYS_DISPLAY */ { PAGE_SYS_PERF, GROUP_SYS, SYS_MENU,
    LED_MODE_MASK | LED_WRITE_MASK },
  /* PAGE_SYS_PERF */ { PAGE_SYS_MIDI, GROUP_SYS, SYS_MENU,
    LED_MODE_MASK | LED_WRITE_MASK },
  /* PAGE_LOAD_SAVE */ { PAGE_LOAD_SAVE, GROUP_LOAD_SAVE, LOAD_SAVE, LED_MODE_MASK },
  /* PAGE_CONFIRM */ { PAGE_CONFIRM, GROUP_CONFIRM, CONFIRM, 0x55 },
  /* PAGE_PERFORM */ { PAGE_PERFORM, GROUP_PERFORM, PARAMETER_EDITOR, 0x00 },
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
  { 8, 9, 0xff, 0xff },  // OPERATOR
  { 0x81, 0x82, 0x83, 0xff },  // POLIVOKS
  { 94, 95, 96, 0xff },  // FX
  { 46, 47, 45, 97 },  // VOICE (octave is global: SYS MIDI)
  { 32, 33, 34, 35 },  // MATRIX
  { 58, 59, 60, 61 },  // MATH
  { 48, 49, 50, 44 },  // SYS MIDI (octave stays global)
  { 52, 53, 0xff, 0xff },  // SYS DISPLAY (filter board fixed: PVK)
  { 90, 91, 92, 93 },  // SYS PERF: preset-page knob targets
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
  "typamtprm   "  // 10
  "prtlegrstvol"  // 10
  "sltsrcdstamt"  // 11
  "op in1in2out";  // 12 (SYS pages are menus: no labels)

static const prog_char page_titles[] PROGMEM =
  "SYN  OSC 1   1/3"
  "SYN  OSC 2   2/3"
  "SYN  MIXER   3/3"
  "FLTR FILTER  1/2"
  "FLTR FLT ENV 2/2"
  "AMP  AMP ENV    "
  "MOD  LFO 1   1/2"
  "MOD  LFO 2   2/2"
  "OPERATOR        "
  "POLIVOKS     1/2"
  "FX           2/2"
  "VOICE           "
  "MATRIX       1/2"
  "MATH         2/2"
  "MIDI            "
  "DISPLAY         "
  "PERFORM MODE    ";

static const prog_char lfo_mode_names[] PROGMEM = "fretrghldonehlf";

static const prog_char osc_names[] PROGMEM = "offsawsqrtrizswzrsztrzplzsypad fmwavtmpdigmetbowslporgmalusr8btcrupwmnoivowbelpolcelclpfemfmtfrmreselevibplk";

// Full lowercase values for the top row, '\0'-separated in value order.
static const prog_char long_wave[] PROGMEM =
  "off\0"
  "saw\0"
  "square\0"
  "triangle\0"
  "cz saw\0"
  "cz reso\0"
  "cz tri\0"
  "cz pulse\0"
  "cz sync\0"
  "pad\0"
  "fm\0"
  "waves\0"
  "tampura\0"
  "digital\0"
  "metallic\0"
  "bowed\0"
  "slap\0"
  "organ\0"
  "male\0"
  "user\0"
  "8 bits\0"
  "crush\0"
  "pwm\0"
  "noise\0"
  "vowel\0"
  "bellish\0"
  "polated\0"
  "cello\0"
  "clip swp\0"
  "female\0"
  "fmtvoice\0"
  "formant2\0"
  "reso 3hp\0"
  "e.piano\0"
  "vibes\0"
  "pluck\0";
static const prog_char long_sub[] PROGMEM =
  "square-1\0"
  "tri-1\0"
  "pulse-1\0"
  "square-2\0"
  "tri-2\0"
  "pulse-2\0"
  "click\0"
  "glitch\0"
  "blow\0"
  "metal\0"
  "pop\0";
static const prog_char long_op[] PROGMEM =
  "sum\0"
  "sync\0"
  "ring mod\0"
  "xor\0"
  "fuzz\0"
  "crush 4\0"
  "crush 8\0"
  "fold\0"
  "bits\0"
  "duo\0"
  "2 steps\0"
  "4 steps\0"
  "8 steps\0"
  "cc mix\0";
static const prog_char long_lfo_wave[] PROGMEM =
  "triangle\0"
  "square\0"
  "s&h\0"
  "ramp\0";
static const prog_char long_lfo_mode[] PROGMEM =
  "free\0"
  "retrig\0"
  "hold\0"
  "one shot\0"
  "half\0";
static const prog_char long_src[] PROGMEM =
  "lfo 1\0"
  "lfo 2\0"
  "modwheel\0"
  "pressure\0"
  "bender\0"
  "offset\0"
  "cc 16\0"
  "cc 17\0"
  "breath\0"
  "pedal\0"
  "noise\0"
  "env 1\0"
  "env 2\0"
  "velocity\0"
  "random\0"
  "note\0"
  "gate\0"
  "audio\0"
  "op 1\0"
  "op 2\0"
  "= 4\0"
  "= 8\0"
  "= 16\0"
  "= 32\0";
static const prog_char long_dst[] PROGMEM =
  "pwm 1\0"
  "pwm 2\0"
  "pitch 1\0"
  "pitch 2\0"
  "osc 1+2\0"
  "finetune\0"
  "osc mix\0"
  "noise\0"
  "sub osc\0"
  "cutoff\0"
  "reso\0"
  "filt env\0"
  "filt lfo\0"
  "vca\0"
  "drive\0"
  "fm\0"
  "pvk mode\0"
  "env1trig\0"
  "attack1\0"
  "decay1\0"
  "sustain1\0"
  "release1\0"
  "env2trig\0"
  "attack2\0"
  "decay2\0"
  "sustain2\0"
  "release2\0"
  "attacks\0"
  "lfo 1\0"
  "lfo 2\0"
  "fx amt\0";
static const prog_char long_cv_op[] PROGMEM =
  "off\0"
  "sum\0"
  "product\0"
  "max\0"
  "min\0"
  "xor\0"
  ">=\0"
  "<=\0"
  "quantize\0"
  "lag\0";
static const prog_char long_midi[] PROGMEM =
  "off\0"
  "thru\0";
static const prog_char long_fx[] PROGMEM =
  "off\0"
  "ring\0"
  "delay\0";
static const char fx_names[] PROGMEM = "offrngdly";
// FX param name, per FX type.
static const prog_char fx_param_names[] PROGMEM =
  "param\0"
  "dry\0"
  "fdbk\0";
static const prog_char long_f1[] PROGMEM =
  "low pass\0"
  "bandpass\0"
  "highpass\0";
// Top-row names that replace the stock long names.
static const prog_uint8_t long_name_ids[] PROGMEM = {
  0, 4, 3, 8, 24, 28, 27, 31, 32, 33, 34, 50, 55, 58, 59, 60, 61, 75, 76, 77, 45,
  94, 95, 97
};
static const prog_char long_names[] PROGMEM =
  "osc1\0"
  "osc2\0"
  "sub osc\0"
  "combine\0"
  "lfo1\0"
  "lfo2\0"
  "lfo1 md\0"
  "lfo2 md\0"
  "slot\0"
  "source\0"
  "dest\0"
  "midi out\0"
  "startup\0"
  "op\0"
  "in 1\0"
  "in 2\0"
  "type\0"
  "filter\0"
  "drive\0"
  "fm fdbk\0"
  "retrig\0"
  "fx\0"
  "amount\0"
  "volume\0";

// Copies the n-th '\0'-separated string of a PROGMEM list.
static void CopyNth(const prog_char* list, uint8_t n, char* out) {
  while (n--) {
    list += strlen_P(list) + 1;
  }
  strcpy_P(out, list);
}

static const prog_char* LongValues(uint8_t unit) {
  switch (unit) {
    case UNIT_WAVEFORM: return long_wave;
    case UNIT_SUB_OSC_WAVEFORM: return long_sub;
    case UNIT_OPERATOR: return long_op;
    case UNIT_LFO_WAVEFORM: return long_lfo_wave;
    case UNIT_LFO_RETRIGGER_MODE: return long_lfo_mode;
    case UNIT_MODULATION_SOURCE: return long_src;
    case UNIT_MODULATION_DESTINATION: return long_dst;
    case UNIT_CV_OPERATOR: return long_cv_op;
    case UNIT_MIDI_MODE: return long_midi;
    case UNIT_FILTER_1_MODE: return long_f1;
    case UNIT_FX_PROGRAM: return long_fx;
  }
  return NULL;
}

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
  0x00, 0x06, 0x09, 0x10, 0x00, 0x00, 0x00, 0x00,  // smooth random
  0x00, 0x10, 0x10, 0x08, 0x06, 0x01, 0x00, 0x00,  // exp decay
};

// Icon index for each value of the units that have icons. 0xff = no icon.
static const prog_uint8_t icon_map[] PROGMEM = {
  5, 0, 1, 3, 6, 7, 8, 9, 10, 11, 12, 17, 17, 17, 17, 17, 17, 17, 17, 17, 13, 24, 14, 15, 16, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 16,
  1, 3, 2, 1, 3, 2, 18, 19, 20, 21, 22,
  25, 10, 26, 27, 23, 24, 24, 28, 13, 29, 30, 31, 32, 0xff,
  3, 1, 4, 0, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 35, 36,
  33, 34,
};

// Custom glyphs: allocated per screen, uploaded to CGRAM by UploadGlyphs().
static uint8_t glyph_[8][8];
static uint8_t glyph_dirty_ = 0xff;
static uint8_t glyph_used_;  // Slots used by the screen being drawn.
static uint8_t glyph_previous_;  // Slots used by the previous screen.
static bool glyph_probe_;  // First pass: only claim slots that still match.

// A glyph keeps its slot while it stays on screen, and a new one goes to a
// slot the LCD is not showing, so a redraw never changes a visible cell.
static char GlyphGet(const uint8_t* bits) {
  // Masks are shifted, never computed as 1 << i: avr-gcc 4.3.5 clobbered i
  // doing so and every reused glyph came back as char 7.
  uint8_t mask = 1;
  for (uint8_t i = 0; i < 8; ++i, mask <<= 1) {
    if (!memcmp(glyph_[i], bits, 8)) {
      glyph_used_ |= mask;
      return 8 + i;
    }
  }
  if (glyph_probe_) {
    return ' ';
  }
  uint8_t slot = 0xff;
  uint8_t slot_mask = 0;
  mask = 1;
  for (uint8_t i = 0; i < 8; ++i, mask <<= 1) {
    if (!(glyph_used_ & mask) &&
        (slot == 0xff || !(glyph_previous_ & mask))) {
      slot = i;
      slot_mask = mask;
      if (!(glyph_previous_ & mask)) {
        break;
      }
    }
  }
  if (slot == 0xff) {  // Pages are laid out to need at most 8.
    return ' ';
  }
  memcpy(glyph_[slot], bits, 8);
  glyph_dirty_ |= slot_mask;
  glyph_used_ |= slot_mask;
  return 8 + slot;
}

// Close-up: tapping the page key again on an oscillator, envelope or LFO
// page draws its shape over the whole screen (80x16 px); the knobs still
// edit it.
static bool closeup_;
static uint8_t scope_[kAudioBlockSize];  // One oscillator cycle.

static const Modulation& MatrixRow() {
  return part.patch().modulation_matrix.modulation[Editor::subpage()];
}

// Page whose shape the current matrix row sends (LFO or env), or 0xff.
static uint8_t MatrixShapePage() {
  uint8_t s = MatrixRow().source;
  if (s <= MOD_SRC_LFO_2) {
    return PAGE_MOD_LFO_1 + s;
  }
  if (s == MOD_SRC_ENV_1 || s == MOD_SRC_ENV_2) {
    return PAGE_MOD_ENV_1 + s - MOD_SRC_ENV_1;
  }
  return 0xff;
}

// Matrix: rows from an LFO or env only (drawn flipped if amount < 0).
static bool HasCloseup(uint8_t page) {
  return page <= PAGE_OSC_OSC_2 ||
      (page >= PAGE_MOD_ENV_1 && page <= PAGE_MOD_LFO_2) ||
      (page == PAGE_MOD_MATRIX && MatrixShapePage() != 0xff);
}

// exp(-4 i / 16) * 255.
static const prog_uint8_t exp_decay[] PROGMEM = {
  255, 199, 155, 121, 94, 73, 57, 44, 35, 27, 21, 16, 13, 10, 8, 6 };

static uint8_t Decay(uint8_t t, uint8_t n) {  // t < n.
  return pgm_read_byte(exp_decay + t * 16 / n);
}

// Level (0-255) of pixel column x (0-79) of an envelope or LFO page.
static uint8_t CloseupLevel(uint8_t page, uint8_t x) {
  if (page == PAGE_MOD_MATRIX) {
    uint8_t level = CloseupLevel(MatrixShapePage(), x);
    return MatrixRow().amount < 0 ? ~level : level;
  }
  if (page <= PAGE_OSC_OSC_2) {  // 2 px per sample, odd columns in between.
    uint8_t i = x >> 1;
    if (!(x & 1)) {
      return scope_[i];
    }
    return (scope_[i] + scope_[i + 1 == kAudioBlockSize ? 0 : i + 1]) >> 1;
  }
  if (page <= PAGE_MOD_ENV_2) {
    const EnvelopeSettings& e = part.patch().env[page - PAGE_MOD_ENV_1];
    uint8_t a = (e.attack * 23 >> 7) + 1;
    uint8_t d = (e.decay * 23 >> 7) + 1;
    uint8_t r = (e.release * 23 >> 7) + 1;
    uint8_t s = e.sustain << 1;
    if (x < a) {
      return x * 255 / a;
    }
    x -= a;
    if (x < d) {
      return s + U8U8MulShift8(255 - s, Decay(x, d));
    }
    x -= d;
    uint8_t hold = 80 - a - d - r;  // At least 11.
    if (x < hold) {
      return s;
    }
    x -= hold;
    return U8U8MulShift8(s, Decay(x, r)) * (r - x) / r;
  }
  const LfoSettings& l = part.patch().lfo[page - PAGE_MOD_LFO_1];
  uint16_t phase = static_cast<uint16_t>(x) * 819U;  // One cycle.
  if (l.speed < 0) {
    phase = ~phase;
  }
  switch (l.waveform) {
    case LFO_WAVEFORM_RAMP: return phase >> 8;
    case LFO_WAVEFORM_S_H: return (phase >> 13) * 97 + 40;  // Fixed steps.
    case LFO_WAVEFORM_TRIANGLE:
      return (phase & 0x8000) ? phase >> 7 : ~static_cast<uint8_t>(phase >> 7);
    case LFO_WAVEFORM_SQUARE: return (phase & 0x8000) ? 255 : 0;
    case LFO_WAVEFORM_SMOOTH: {  // The S&H steps, joined.
      uint8_t k = phase >> 13;
      return U8Mix(k * 97 + 40, (k + 1) * 97 + 40, phase >> 5);
    }
    case LFO_WAVEFORM_EXP: return Lfo::Exp(phase);
  }
  uint8_t shape = l.waveform - LFO_WAVEFORM_WAVE_1;  // As in Lfo::Render().
  shape = shape ? shape + 16 : 3;
  return ResourcesManager::Lookup<uint8_t, uint8_t>(
      wav_res_waves + U8U8Mul(shape, 129), phase >> 9);
}

// Filled shape, cells from the bottom; full cells use the ROM block 0xff. If
// more than 8 glyphs are needed, each cell gets one height (7 glyphs at most).
void Editor::DrawCloseup(uint8_t page, char* row_0) {
  if (page <= PAGE_OSC_OSC_2) {
    // Live capture: the patch's oscillator code renders one cycle in one
    // audio block, from phase 0 (PWM and other modulations not applied).
    const OscillatorSettings& s = part.patch().osc[page];
    Oscillator osc;
    memset(&osc, 0, sizeof(osc));
    osc.set_parameter(s.parameter);
    osc.set_secondary_parameter(s.range + 24);
    osc.set_pluck_line(Oscillator::pluck_lines_[page]);  // Shows the live string.
    uint24_t increment;
    increment.integral = 65536 / kAudioBlockSize;
    increment.fractional = 102;  // 2^24 / 40 = 1638.4 x 256.
    osc.Render(s.shape, 60, increment, Voice::no_sync_,
        Voice::dummy_sync_state_, scope_);
  }
  glyph_previous_ = glyph_used_;
  for (uint8_t coarse = 0; coarse < 2; ++coarse) {
    bool overflow = false;
    glyph_used_ = 0;
    for (uint8_t pass = 0; pass < 2; ++pass) {
      glyph_probe_ = pass == 0;
      for (uint8_t c = 0; c < kLcdWidth; ++c) {
        uint8_t h[5];
        uint8_t sum = 0;
        for (uint8_t p = 0; p < 5; ++p) {
          h[p] = (CloseupLevel(page, c * 5 + p) + 8) >> 4;
          if (!h[p]) {
            h[p] = 1;  // Zero line.
          }
          sum += h[p];
        }
        if (coarse) {
          memset(h, (sum + 2) / 5, 5);
        }
        for (uint8_t row = 0; row < 2; ++row) {
          uint8_t bits[8];
          uint8_t all = 0x1f;
          uint8_t any = 0;
          for (uint8_t y = 0; y < 8; ++y) {
            uint8_t level = (row ? 8 : 16) - y;
            uint8_t b = 0;
            for (uint8_t p = 0; p < 5; ++p) {
              b <<= 1;
              if (h[p] >= level) {
                b |= 1;
              }
            }
            bits[y] = b;
            all &= b;
            any |= b;
          }
          char ch = all == 0x1f ? 0xff : (any ? GlyphGet(bits) : ' ');
          if (any && ch == ' ' && !glyph_probe_) {
            overflow = true;
          }
          row_0[row * (kLcdWidth + 1) + c] = ch;
        }
        ::FillAudioBuffer();
      }
    }
    if (!overflow) {
      return;
    }
  }
}

static char Icon(uint8_t unit, uint8_t value) {
  uint8_t base, count;
  switch (unit) {
    case UNIT_WAVEFORM: base = 0; count = 36; break;
    case UNIT_SUB_OSC_WAVEFORM: base = 36; count = 11; break;
    case UNIT_OPERATOR: base = 47; count = 14; break;
    case UNIT_LFO_WAVEFORM: base = 61; count = 22; break;
    case UNIT_FILTER_1_MODE: base = 83; count = 2; break;
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

// Tall font, 5x16 over both LCD rows: 0-9, S H R U T I, N E P L V K,
// then the 4 cells of a hammer and sickle (22-25).
static const prog_uint8_t tall_font[] PROGMEM = {
  0x0e, 0x1f, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1f, 0x0e,  // 0
  0x06, 0x0e, 0x1e, 0x16, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x1f, 0x1f,  // 1
  0x0e, 0x1f, 0x1b, 0x03, 0x03, 0x03, 0x07, 0x0e, 0x1c, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1f, 0x1f,  // 2
  0x1e, 0x1f, 0x03, 0x03, 0x03, 0x03, 0x0e, 0x0e, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x1f, 0x1e,  // 3
  0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1f, 0x1f, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,  // 4
  0x1f, 0x1f, 0x18, 0x18, 0x18, 0x18, 0x1e, 0x1f, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x1f, 0x1e,  // 5
  0x0e, 0x1e, 0x18, 0x18, 0x18, 0x18, 0x1e, 0x1f, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1f, 0x0e,  // 6
  0x1f, 0x1f, 0x03, 0x03, 0x03, 0x06, 0x06, 0x06, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,  // 7
  0x0e, 0x1f, 0x1b, 0x1b, 0x1b, 0x1b, 0x1f, 0x0e, 0x1f, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1f, 0x0e,  // 8
  0x0e, 0x1f, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1f, 0x0f, 0x03, 0x03, 0x03, 0x03, 0x03, 0x0f, 0x0e,  // 9
  0x0f, 0x1f, 0x18, 0x18, 0x18, 0x18, 0x1f, 0x0f, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x1f, 0x1e,  // S
  0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1f, 0x1f, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b,  // H
  0x1e, 0x1f, 0x1b, 0x1b, 0x1b, 0x1b, 0x1f, 0x1e, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b,  // R
  0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1f, 0x0e,  // U
  0x1f, 0x1f, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e,  // T
  0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e,  // I
  0x1e, 0x1f, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b,  // N
  0x1f, 0x1f, 0x18, 0x18, 0x18, 0x18, 0x1e, 0x1e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1f, 0x1f,  // E
  0x1e, 0x1f, 0x1b, 0x1b, 0x1b, 0x1b, 0x1f, 0x1e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,  // P
  0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1f, 0x1f,  // L
  0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x0e, 0x0e, 0x04, 0x04,  // V
  0x1b, 0x1b, 0x1b, 0x1b, 0x1a, 0x1e, 0x1c, 0x18, 0x1c, 0x1e, 0x1a, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b,  // K
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03, 0x03, 0x00,  // hammer & sickle 0
  0x00, 0x00, 0x00, 0x07, 0x0f, 0x1f, 0x1f, 0x1f, 0x09, 0x00, 0x18, 0x1c, 0x1f, 0x03, 0x00, 0x00,  // hammer & sickle 1
  0x00, 0x10, 0x0c, 0x16, 0x13, 0x01, 0x01, 0x11, 0x19, 0x1d, 0x0f, 0x07, 0x1f, 0x1d, 0x00, 0x00,  // hammer & sickle 2
  0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x10, 0x18, 0x18, 0x00,  // hammer & sickle 3
};

// Boot words as tall_font indices (O is the 0), 0xff ends a word.
static const prog_uint8_t boot_words[] PROGMEM = {
  10, 11, 12, 13, 14, 11, 15, 0xff,  // SHRUTHI
  0, 16, 17, 0xff,  // ONE
  18, 0, 19, 15, 0xff,  // POLI
  20, 0, 21, 10, 0xff,  // VOKS
  22, 23, 24, 25, 0xff,  // hammer and sickle (4 cells: centred)
};

// 0 = show the memory (INT/EXT), 1 = edited since load, 2 = just reloaded.
static uint8_t preset_status_;
static uint8_t browse_name_[8];  // Name of the slot shown on PRESET.
static const prog_char* popup_text_;  // 8 chars, second popup row.

// Save page: the selected number is in the off phase of its blink.
static uint8_t number_hidden_;

static uint8_t boot_step_;  // Boot screens 0-9. Even: blank, odd: a word.
static uint8_t boot_word_;  // Read position in boot_words.
static bool saver_;  // Screen saver: boot words loop until any input.

// Starts a new glyph screen and draws tall_font characters from column 0 of
// both rows. Two passes, as in DisplayEditOverviewPage.
static void PrintTall(const uint8_t* chars, uint8_t n, char* row_0) {
  glyph_previous_ = glyph_used_;
  glyph_used_ = 0;
  for (uint8_t pass = 0; pass < 2; ++pass) {
    glyph_probe_ = pass == 0;
    for (uint8_t i = 0; i < n; ++i) {
      uint8_t bits[8];
      memcpy_P(bits, tall_font + 16 * chars[i], 8);
      row_0[i] = GlyphGet(bits);
      memcpy_P(bits, tall_font + 16 * chars[i] + 8, 8);
      row_0[kLcdWidth + 1 + i] = GlyphGet(bits);
    }
  }
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
    PAGE_SYS_MIDI,
    PAGE_LOAD_SAVE,
    PAGE_CONFIRM,
    PAGE_PERFORM
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
uint8_t Editor::page_tool_;
uint8_t Editor::clip_page_ = 0xff;
uint8_t Editor::clip_[4];
uint8_t Editor::last_knob_;
uint8_t Editor::subpage_;
uint8_t Editor::action_;
uint16_t Editor::current_patch_number_ = 0;
uint16_t Editor::loaded_patch_ = 0;


ConfirmPageSettings Editor::confirm_page_settings_;
ParameterPage Editor::confirm_return_page_;
bool Editor::snapped_[4];
bool Editor::touched_[4];
bool Editor::title_flash_;
bool Editor::show_numbers_;
bool Editor::func_;
bool Editor::empty_patch_;
uint16_t Editor::deferred_load_;
uint32_t Editor::popup_until_;
uint8_t Editor::popup_phase_;
uint8_t Editor::perf_[4];
bool Editor::sys_open_;
uint32_t Editor::sys_save_at_;
bool Editor::perf_setup_;
/* </static> */

// Free bytes after the sequences (internal EEPROM ends at 2048).
static uint8_t* const kPerfEeprom = (uint8_t*)2044;
static const prog_uint8_t perf_defaults[4] PROGMEM = {
  PRM_OSC_PARAMETER_1, PRM_FILTER_CUTOFF, PRM_FILTER_RESONANCE, PRM_FILTER_ENV
};

/* static */
void Editor::Init() {
  parameter_manager.Init();
  eeprom_read_block(perf_, kPerfEeprom, 4);
  for (uint8_t i = 0; i < 4; ++i) {
    if (perf_[i] > 31) {  // 0xff = never saved.
      perf_[i] = pgm_read_byte(perf_defaults + i);
    }
  }

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
  title_flash_ = current_page_ <= PAGE_SYS_PERF;
  closeup_ = false;
  
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
  part.Touch();
}

/* static */
void Editor::SaveSystemSettings() {
  part.mutable_system_settings()->EepromSave();
  EepromUpdate(kPerfEeprom, perf_, 4);
}

// Labels of patch parameter ids 0-31 come from the synth pages 0-8.
static const prog_char perf_groups[] PROGMEM = "osc1osc2mix filtfenvaenvlfo1lfo2op  ";

/* static */
void Editor::PerfLabel(uint8_t id, char* out, bool long_name) {
  for (uint8_t page = 0; page <= PAGE_FX; ++page) {
    for (uint8_t k = 0; k < 4; ++k) {
      if (pgm_read_byte(&page_parameters[page][k]) == id) {
        if (long_name) {  // "filt cut": group, space, label.
          memcpy_P(out, perf_groups + 4 * page, 4);
          out[4] = ' ';
          memcpy_P(out + 5, page_labels + 12 * page + 3 * k, 3);
        } else {
          memcpy_P(out, page_labels + 12 * page + 3 * k, 3);
        }
        return;
      }
    }
  }
}

/* static */
void Editor::StartMidiBackup() {
  Storage::SysExBulkDump();
}

/* static */
void Editor::Confirm(ConfirmPageSettings confirm_page_settings) {
  confirm_page_settings_ = confirm_page_settings;
  confirm_return_page_ = current_page_;
  // Yes/no screens start on no; the page tools menu on copy.
  cursor_ = confirm_page_settings.callback == &PageTool;
  current_page_ = PAGE_CONFIRM;
  display_mode_ = DISPLAY_MODE_OVERVIEW;
  Refresh();
}

/* static */
void Editor::OnSwitch(const Event& event) {
  uint8_t id = event.control_id;
  if (current_page_ == PAGE_PERFORM) {
    return;  // Only holding SYS leaves perform mode.
  }
  if (event.value == 0xff) {
    // FUNC + key.
    if (id == SWITCH_MODE) {
      // FUNC + SYS: open the SETTINGS list, or go back to it from a sub-menu.
      if (page_definition_[current_page_].group != GROUP_SYS) {
        JumpToPageGroup(GROUP_SYS);
      }
      sys_open_ = false;
      cursor_ = current_page_ - PAGE_SYS_MIDI;
      display_mode_ = DISPLAY_MODE_OVERVIEW;
    } else if (!closeup_ && current_page_ == PAGE_MOD_MATRIX &&
        id + GROUP_FX == GROUP_MOD_MATRIX && HasCloseup(current_page_)) {
      closeup_ = true;  // LFO/env row: the next tap shows what it sends.
      title_flash_ = true;
    } else {
      JumpToPageGroup(id + GROUP_FX);
    }
  } else if (id == SWITCH_MODE) {
    ToggleLoadSaveAction();
  } else if (!closeup_ && HasCloseup(current_page_) &&
      page_definition_[current_page_].group == id + GROUP_SYN) {
    closeup_ = true;  // Osc/envelope/LFO: the next tap shows the shape.
    title_flash_ = true;
  } else {
    JumpToPageGroup(id + GROUP_SYN);
  }
}

/* static */
void Editor::OnInput(uint8_t knob_index, uint8_t value) {
  popup_until_ = 0;  // A knob move replaces the popup.
  (*ui_handler_[page_definition_[current_page_].ui_type].input_handler)(
      knob_index, value);
}

/* static */
void Editor::OnIncrement(int8_t increment) {
  if (popup_phase_ < 0xfe) {
    popup_until_ = 0;
  }
  (*ui_handler_[page_definition_[current_page_].ui_type].increment_handler)(
      increment);
}

/* static */
void Editor::OnClick() {
  title_flash_ = false;
  if (current_page_ == PAGE_PERFORM) {
    return;
  }
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
  if (popup_until_) {
    if (popup_phase_ >= 0xfe) {
      return;  // Boot screens stay up.
    }
    // [][][][]NAME....[][][][] / [][][][]RELOADED[][][][], blocks lit.
    memset(line_buffer_, popup_phase_ & 1 ? 0xff : ' ', sizeof(line_buffer_));
    memset(line_buffer_ + 4, ' ', 8);
    memcpy(line_buffer_ + 4, part.patch().name, kPatchNameSize);
    memcpy_P(line_buffer_ + kLcdWidth + 1 + 4, popup_text_, 8);
    display.set_cursor_position(kLcdNoCursor);
    display.Print(0, line_buffer_);
    display.Print(1, line_buffer_ + kLcdWidth + 1);
    return;
  }
  if (display_mode_ == DISPLAY_MODE_OVERVIEW) {
    (*ui_handler_[page_definition_[current_page_].ui_type].overview_page)();
  } else {
    (*ui_handler_[page_definition_[current_page_].ui_type].edit_page)();
  }
}

/* static */
void Editor::ReloadPatch() {
  set_edited_item_number(loaded_patch_);
  deferred_load_ = loaded_patch_;
  preset_status_ = 2;
  ShowPatchPopup(PSTR("RELOADED"));
}

/* static */
void Editor::ShowPatchPopup(const prog_char* text) {
  popup_text_ = text;
  // Shown once, as long as a page title (SYS display delay).
  popup_until_ = milliseconds() +
      ((part.system_settings().display_delay & 0xf) << 7) + 100;
  popup_phase_ = 1;
}

/* static */
bool Editor::PopupTick() {
  // Global settings auto-save 1 s after the last change (one EEPROM write per edit burst).
  if (sys_save_at_ && static_cast<int32_t>(milliseconds() - sys_save_at_) >= 0) {
    sys_save_at_ = 0;
    SaveSystemSettings();
  }
  if (!popup_until_) {
    // Save page: the selected number or OK blinks, faster while edited.
    uint8_t hidden = current_page_ == PAGE_LOAD_SAVE &&
        action_ == ACTION_SAVE && (cursor_ == 0 || cursor_ >= kLcdWidth - 4) &&
        (milliseconds() >> (display_mode_ == DISPLAY_MODE_EDIT ? 7 : 8)) & 1;
    if (hidden != number_hidden_) {
      number_hidden_ = hidden;
      return true;
    }
    return false;
  }
  int32_t left = popup_until_ - milliseconds();
  if (left <= 0) {
    if (popup_phase_ == 0xfe && (++boot_step_ < 10 || saver_)) {
      if (boot_step_ == 10) {
        boot_step_ = 0;
        boot_word_ = 0;
      }
      BootStep();
      return false;
    }
    popup_until_ = 0;
    return true;
  }
  if (popup_phase_ == 1) {
    popup_phase_ = 3;  // Draw once; no blinking.
    return true;
  }
  return false;
}

/* static */
void Editor::ScreenSaver() {
  if (saver_) {
    return;
  }
  // The boot words, looped by PopupTick until WakeUp.
  saver_ = true;
  display.set_cursor_position(kLcdNoCursor);
  boot_step_ = 0;
  boot_word_ = 0;
  popup_phase_ = 0xfe;
  BootStep();
}

/* static */
void Editor::WakeUp() {
  if (!saver_) {
    return;
  }
  saver_ = false;
  if (popup_phase_ >= 0xfe) {  // A patch popup that took over keeps its time.
    popup_until_ = 0;
  }
}

/* static */
void Editor::RestoreEditBuffer() {
  Storage::Restore(part.mutable_patch());
  part.Touch();
}

/* static */
void Editor::BackupEditBuffer() {
  Storage::Backup(part.mutable_patch());
}

/* static */
void Editor::ToggleLoadSaveAction() {
  empty_patch_ = false;
  if (current_page_ == PAGE_LOAD_SAVE && action_ == ACTION_LOAD) {
    set_edited_item_number(loaded_patch_);  // Browse back to the loaded slot.
    return;
  }
  if (action_ == ACTION_COMPARE) {
    RestoreEditBuffer();  // Leaving compare keeps the edits.
  }
  action_ = ACTION_LOAD;
  set_edited_item_number(loaded_patch_);
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
  
  Storage::LoadPatchName(browse_name_, edited_item_number());
  for (uint8_t i = 0; i < 8; ++i) {
    if (browse_name_[i] >= 0x80) {  // Blank EEPROM: dashes, as stock.
      memset(browse_name_, '-', 8);
      break;
    }
  }
  empty_patch_ = true;
  for (uint8_t i = 0; i < 8; ++i) {
    if (browse_name_[i] != pgm_read_byte(default_name + i)) {
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
    action_ = ACTION_MENU;
    cursor_ = 0;
    display_mode_ = DISPLAY_MODE_OVERVIEW;
  } else if (action_ == ACTION_MENU) {
    OnPatchMenuClick();
  } else if (action_ == ACTION_COMPARE) {
    RestoreEditBuffer();
    action_ = ACTION_LOAD;
  } else {
    if (!is_cursor_at_valid_position()) {
      display_mode_ = DISPLAY_MODE_OVERVIEW;
    }
    if (cursor_ >= kLcdWidth - 4) {
      Storage::WritePatch(edited_item_number());
      loaded_patch_ = edited_item_number();
      action_ = ACTION_LOAD;
      preset_status_ = 0;
    }
  }
}

// PATCH menu items, 10 chars each, in PatchItem order.
static const prog_char patch_items[] PROGMEM =
  "load      quick savesave as   save here reload    compare   "
  "init      random    send patchbackup all back      ";

enum PatchItem {
  PI_LOAD, PI_QUICK_SAVE, PI_SAVE_AS, PI_SAVE_HERE, PI_RELOAD, PI_COMPARE,
  PI_INIT, PI_RANDOM, PI_SEND, PI_BACKUP, PI_BACK
};

/* static */
uint8_t Editor::PatchMenu(uint8_t* items) {
  uint8_t n = 0;
  if (current_patch_number_ != loaded_patch_) {
    // Browsed slot: only what applies to a stored patch.
    items[n++] = PI_LOAD;
    items[n++] = PI_SAVE_HERE;
  } else {
    bool edited = preset_status_ == 1;
    if (edited) {
      items[n++] = PI_QUICK_SAVE;
    }
    items[n++] = PI_SAVE_AS;
    if (edited) {
      items[n++] = PI_RELOAD;
      items[n++] = PI_COMPARE;
    }
    items[n++] = PI_INIT;
    items[n++] = PI_RANDOM;
    items[n++] = PI_SEND;
  }
  items[n++] = PI_BACKUP;
  items[n++] = PI_BACK;
  if (cursor_ >= n) {
    cursor_ = n - 1;
  }
  return n;
}

/* static */
void Editor::QuickSave() {
  // Overwrite the loaded slot, keeping its stored name.
  uint8_t* name = part.mutable_patch()->name;
  Storage::LoadPatchName(name, loaded_patch_);
  for (uint8_t i = 0; i < kPatchNameSize; ++i) {
    if (name[i] >= 0x80) {  // Blank slot: the empty name.
      memcpy_P(name, default_name, kPatchNameSize);
      break;
    }
  }
  Storage::WritePatch(loaded_patch_);
  preset_status_ = 0;
  set_edited_item_number(current_patch_number_);
  ShowPatchPopup(PSTR(" SAVED  "));
}

/* static */
void Editor::QuickSaveConfirm() {
  if (current_page_ >= PAGE_CONFIRM) {
    return;  // Already confirming, or perform mode.
  }
  ConfirmPageSettings settings;
  settings.callback = &QuickSave;
  Confirm(settings);
}

/* static */
void Editor::PageToolsMenu() {
  if (current_page_ >= PAGE_SYS_MIDI) {
    return;  // Synth pages only.
  }
  ConfirmPageSettings settings;
  settings.callback = &PageTool;
  Confirm(settings);
}

// Knob k of the current page, or 0xff if empty or a row selector.
/* static */
uint8_t Editor::ToolKnob(uint8_t k) {
  uint8_t id = KnobIndexToParameterId(k);
  if (id != 0xff) {
    uint8_t offset = parameter_manager.parameter(id).offset;
    if (offset == PRM_MOD_ROW || offset == PRM_OP_ROW) {
      id = 0xff;
    }
  }
  return id;
}

/* static */
void Editor::RandomizePage() {
  for (uint8_t k = 0; k < 4; ++k) {
    uint8_t id = ToolKnob(k);
    if (id != 0xff && id != 97) {  // Never the patch volume.
      const Parameter& p = parameter_manager.parameter(id);
      SetParameterValue(id, p.offset, p.RandomValue());
    }
  }
  ShowPatchPopup(PSTR(" RANDOM "));
}

/* static */
void Editor::PageTool() {
  if (page_tool_ == 3) {
    ConfirmPageSettings settings;
    settings.callback = &RandomizePage;
    Confirm(settings);  // "sure?", defaults to no.
    return;
  }
  // Paste goes to knobs whose label matches the copied page's (osc 1 <->
  // osc 2, env 1 <-> env 2, lfo 1 <-> lfo 2, same page in another patch).
  uint8_t pasted = 0;
  for (uint8_t k = 0; k < 4; ++k) {
    uint8_t id = ToolKnob(k);
    if (id == 0xff) {
      continue;
    }
    uint8_t offset = parameter_manager.parameter(id).offset;
    if (page_tool_ == 1) {
      clip_[k] = GetParameterValue(offset);
      continue;
    }
    if (clip_page_ == 0xff) {
      break;
    }
    const prog_char* a = page_labels + 12 * clip_page_ + 3 * k;
    const prog_char* b = page_labels + 12 * current_page_ + 3 * k;
    if (pgm_read_byte(a) == pgm_read_byte(b) &&
        pgm_read_byte(a + 1) == pgm_read_byte(b + 1) &&
        pgm_read_byte(a + 2) == pgm_read_byte(b + 2)) {
      SetParameterValue(id, offset, clip_[k]);
      pasted = 1;
    }
  }
  if (page_tool_ == 1) {
    clip_page_ = current_page_;
    ShowPatchPopup(PSTR(" COPIED "));
  } else {
    ShowPatchPopup(pasted ? PSTR(" PASTED ") : PSTR("NO MATCH"));
  }
}

/* static */
void Editor::OnPatchMenuClick() {
  display_mode_ = DISPLAY_MODE_OVERVIEW;
  uint8_t items[11];
  PatchMenu(items);
  uint8_t item = items[cursor_];
  action_ = ACTION_LOAD;
  switch (item) {
    case PI_LOAD:
      deferred_load_ = current_patch_number_;
      preset_status_ = 0;
      break;
    case PI_QUICK_SAVE:
      QuickSave();
      break;
    case PI_SAVE_AS:
    case PI_SAVE_HERE:
      action_ = ACTION_SAVE;
      cursor_ = 0;
      set_edited_item_number(edited_item_number());
      break;
    case PI_RELOAD:
      ReloadPatch();
      break;
    case PI_COMPARE:
      // Hear the stored version of this patch; click again for the edit.
      BackupEditBuffer();
      deferred_load_ = loaded_patch_;
      action_ = ACTION_COMPARE;
      break;
    case PI_INIT:
    case PI_RANDOM:
      if (item == PI_INIT) {
        part.ResetPatch();
        ShowPatchPopup(PSTR("  INIT  "));
      } else {
        RandomizePatch();
        ShowPatchPopup(PSTR(" RANDOM "));
      }
      preset_status_ = 1;  // EDITED *
      break;
    case PI_SEND:
      Storage::SysExDump(part.mutable_patch());
      ShowPatchPopup(PSTR("  SENT  "));
      break;
    case PI_BACKUP:
      {
        action_ = ACTION_MENU;  // The confirm window returns to the menu.
        ConfirmPageSettings confirm_midi_backup;
        confirm_midi_backup.callback = &StartMidiBackup;
        Confirm(confirm_midi_backup);
      }
      break;
  }
}

/* static */
bool Editor::FinishLoadingPatch() {
  if (deferred_load_ == 0xffff) {
    return false;
  }
  uint16_t n = deferred_load_;
  Storage::LoadPatch(n);
  if (!popup_until_ && action_ != ACTION_COMPARE) {
    ShowPatchPopup(PSTR(" LOADED "));
  }
  if (action_ != ACTION_COMPARE && preset_status_ == 1) {
    preset_status_ = 0;
  }
  loaded_patch_ = n;
  if (part.system_settings().last_patch != n) {
    part.mutable_system_settings()->last_patch = n;  // Reloaded at power-up.
    sys_save_at_ = (milliseconds() + 1000) | 1;  // Deferred: notes may follow.
  }
  part.Touch();
  // Reset the snap position of all pots.
  memset(snapped_, false, sizeof(snapped_));
  deferred_load_ = 0xffff;
  return true;
}

/* static */
void Editor::OnLoadSaveIncrement(int8_t increment) {
  if (action_ == ACTION_MENU) {
    uint8_t items[11];
    if (increment > 0 && cursor_ < PatchMenu(items) - 1) {
      ++cursor_;
    } else if (increment < 0 && cursor_ > 0) {
      --cursor_;
    }
  } else if (action_ == ACTION_SAVE) {
    if (display_mode_ == DISPLAY_MODE_OVERVIEW) {
      int8_t new_cursor = static_cast<int8_t>(cursor_) + increment;
      if (new_cursor > 0 && new_cursor < 4) {
        new_cursor = increment > 0 ? 4 : 0;  // The number is one slot.
      }
      if (new_cursor >= 0 && new_cursor <= kLcdWidth - 4) {  // 12 = ok.
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
  } else if (action_ == ACTION_LOAD) {
    set_edited_item_number(edited_item_number() + increment);
  }
}

/* static */
void Editor::DisplayLoadSavePage() {
  // 012  FUNK WAH / 012  INT: number in the tall font (at most 6 glyphs),
  // name on top, then the memory, EDITED *, RELOADED, SAVE [ok] or COMPARE.
  memset(line_buffer_, ' ', sizeof(line_buffer_));
  char* row_1 = line_buffer_ + kLcdWidth + 1;
  if (action_ == ACTION_MENU) {
    // PATCH        1/9
    // >quick save
    uint8_t items[11];
    uint8_t count = PatchMenu(items);
    memcpy_P(line_buffer_, PSTR("PATCH"), 5);
    line_buffer_[kLcdWidth - 3] = '1' + cursor_;
    line_buffer_[kLcdWidth - 2] = '/';
    line_buffer_[kLcdWidth - 1] = '0' + count;
    line_buffer_[kLcdWidth] = '\0';
    row_1[0] = 0x7e;
    memcpy_P(row_1 + 1, patch_items + 10 * items[cursor_], 10);
    display.set_cursor_position(kLcdNoCursor);
    display.Print(0, line_buffer_);
    display.Print(1, row_1);
    return;
  }
  uint16_t n = edited_item_number();
  uint8_t digits[3] = { n / 100, n / 10 % 10, n % 10 };
  PrintTall(digits, 3, line_buffer_);
  if (number_hidden_ && action_ == ACTION_SAVE && cursor_ == 0) {
    memset(line_buffer_, ' ', 3);
    memset(line_buffer_ + kLcdWidth + 1, ' ', 3);
  }
  bool browsing = action_ == ACTION_LOAD && n != loaded_patch_;
  memcpy(line_buffer_ + 5, browsing ? browse_name_ : part.patch().name,
      kPatchNameSize);
  const prog_char* status =
      action_ == ACTION_SAVE ? PSTR("SAVE") :
      action_ == ACTION_COMPARE ? PSTR("COMPARE") :
      browsing ?
          (n < StorageConfiguration<Patch>::num_internal ? PSTR("INT") :
              PSTR("EXT")) :
      preset_status_ == 1 ? PSTR("EDITED *") :
      preset_status_ == 2 ? PSTR("RELOADED") : PSTR("LOADED");
  memcpy_P(row_1 + 5, status, strlen_P(status));
  uint8_t cursor = kLcdNoCursor;
  if (action_ == ACTION_SAVE) {
    if (empty_patch_) {
      row_1[10] = '*';  // Target slot is empty.
    }
    if (!number_hidden_ || cursor_ < kLcdWidth - 4) {
      row_1[13] = 'o';
      row_1[14] = 'k';
    }
    if (is_cursor_at_valid_position()) {
      // Name: on its letter, top row. The number blinks instead.
      cursor = cursor_ ? cursor_ + 1 : kLcdNoCursor;
    }
  }
  display.set_cursor_position(cursor);
  display.set_cursor_character(
      display_mode_ == DISPLAY_MODE_EDIT ? kLcdEditCursor : kLcdCursor);
  display.Print(0, line_buffer_);
  display.Print(1, row_1);
}

/* static */
bool Editor::display_idle() {
  return !glyph_dirty_ && display.synced();
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
  if (is_signed && static_cast<int8_t>(value) > 0) {
    *t++ = '+';
  }
  if (p.unit == UNIT_WAVEFORM) {
    memcpy_P(t, osc_names + 3 * value, 3);
  } else if (p.unit == UNIT_OPERATOR && value == OP_CC_MIX) {
    strcpy_P(t, PSTR("ccmix"));  // No resource string: resources.cc stays as is.
  } else if (p.unit == UNIT_LFO_RETRIGGER_MODE) {
    memcpy_P(t, lfo_mode_names + 3 * value, 3);
  } else if (p.unit == UNIT_FX_PROGRAM) {
    memcpy_P(t, fx_names + 3 * value, 3);
  } else if (p.unit == UNIT_LFO_WAVEFORM && value > LFO_WAVEFORM_WAVE_16) {
    memcpy_P(t, PSTR("smoexp") + 3 * (value - LFO_WAVEFORM_SMOOTH), 3);
  } else if (p.unit == UNIT_MODULATION_DESTINATION) {
    // Display order (modulation_destination_map); resources.cc stays as is.
    memcpy_P(t, PSTR("pw1pw2pt1pt2o12finmixnoisubcutresfenflfvcadrvfm pvktr1a1 d1 s1 "
                     "r1 tr2a2 d2 s2 r2 attlf1lf2fx ") + 3 * value, 3);
  } else if (p.unit == UNIT_MODULATION_SOURCE &&
      value >= MOD_SRC_CC_A && value <= MOD_SRC_CC_B) {
    memcpy_P(t, PSTR("c16c17") + 3 * (value - MOD_SRC_CC_A), 3);
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
  if (id == 2 || id == 6 || id == 44) {
    // Osc range, octave: plain number, keeps glyphs for bars.
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
void Editor::DisplaySysPage() {
  // List:  SETTINGS     1/4   inside:  KEYBOARD     1/2   last row inside:
  //        >keyboard                   >octave        +1  >back
  char* row_0 = line_buffer_;
  char* row_1 = line_buffer_ + kLcdWidth + 1;
  memset(line_buffer_, ' ', sizeof(line_buffer_));
  uint8_t count = sys_open_ ? SysItems() : 3;
  if (sys_open_) {
    memcpy_P(row_0, page_titles + kLcdWidth * current_page_, kLcdWidth);
  } else {
    memcpy_P(row_0, PSTR("SETTINGS"), 8);
  }
  uint8_t cursor = kLcdNoCursor;
  if (sys_open_ && cursor_ == count) {
    memcpy_P(row_1 + 1, PSTR("back"), 4);
  } else {
    row_0[kLcdWidth - 3] = '1' + cursor_;
    row_0[kLcdWidth - 2] = '/';
    row_0[kLcdWidth - 1] = '0' + count;
    if (sys_open_ && current_page_ == PAGE_SYS_PERF && !perf_setup_) {
      memcpy_P(row_1 + 1, cursor_ ? PSTR("setup") : PSTR("enter"), 5);
    } else if (sys_open_) {
      NameValue(KnobIndexToParameterId(cursor_), row_1 + 1, true);
    } else {
      memcpy_P(row_1 + 1, page_titles + kLcdWidth * current_page_, 15);
      for (uint8_t i = 1; i < kLcdWidth; ++i) {
        if (row_1[i] >= 'A' && row_1[i] <= 'Z') {
          row_1[i] += 32;
        }
      }
    }
  }
  row_0[kLcdWidth] = '\0';
  row_1[0] = 0x7e;  // HD44780 right arrow; blinks while editing.
  if (sys_open_ && display_mode_ == DISPLAY_MODE_EDIT) {
    row_1[0] = ' ';
    cursor = kLcdWidth;
    display.set_cursor_character(0x7e);
  }
  display.set_cursor_position(cursor);
  display.Print(0, row_0);
  display.Print(1, row_1);
}

/* static */
uint8_t Editor::SysItems() {
  if (current_page_ == PAGE_SYS_PERF && !perf_setup_) {
    return 2;  // enter, setup
  }
  uint8_t n = 0;
  while (n < 4 && pgm_read_byte(&page_parameters[current_page_][n]) != 0xff) {
    ++n;
  }
  return n;
}

/* static */
void Editor::OnSysInput(uint8_t knob_index, uint8_t value) {
  // Pots never change settings: a noisy pot would rewrite the MIDI channel.
}

/* static */
void Editor::OnSysIncrement(int8_t increment) {
  memset(snapped_, false, sizeof(snapped_));
  if (sys_open_ && display_mode_ == DISPLAY_MODE_EDIT) {
    OnEditIncrement(increment);  // Change the value.
    return;
  }
  display_mode_ = DISPLAY_MODE_OVERVIEW;
  // Browse: items, then "back" as the last row of a sub-menu.
  uint8_t count = sys_open_ ? SysItems() : 2;
  if (increment > 0 && cursor_ < count) {
    ++cursor_;
  } else if (increment < 0 && cursor_ > 0) {
    --cursor_;
  }
  if (!sys_open_) {
    current_page_ = PAGE_SYS_MIDI + cursor_;
    last_visited_page_[GROUP_SYS] = current_page_;
  }
}

/* static */
void Editor::OnSysClick() {
  if (sys_open_ && cursor_ == SysItems()) {
    if (perf_setup_) {
      perf_setup_ = false;  // back to enter/setup
      cursor_ = 1;
    } else {
      sys_open_ = false;  // back to the list
      cursor_ = current_page_ - PAGE_SYS_MIDI;
    }
    display_mode_ = DISPLAY_MODE_OVERVIEW;
  } else if (!sys_open_) {
    sys_open_ = true;
    perf_setup_ = false;
    cursor_ = 0;
    display_mode_ = DISPLAY_MODE_OVERVIEW;
  } else if (current_page_ == PAGE_SYS_PERF && !perf_setup_) {
    display_mode_ = DISPLAY_MODE_OVERVIEW;
    if (cursor_) {
      perf_setup_ = true;
      cursor_ = 0;
    } else {
      current_page_ = PAGE_PERFORM;
      memset(snapped_, false, sizeof(snapped_));
    }
  }
}

/* static */
void Editor::ExitPerform() {
  current_page_ = PAGE_SYS_PERF;  // sys_open_ is still set: enter/setup list.
  cursor_ = 0;
  display_mode_ = DISPLAY_MODE_OVERVIEW;
}

/* static */
// Name in cols 0-6, gap, value right-aligned in 8-15, all lowercase.
// wide (menu rows): 15 cols, the name may run up to the value.
void Editor::NameValue(uint8_t id, char* row, bool wide) {
  const Parameter& p = parameter_manager.parameter(id);
  char text[kLcdWidth + 1];
  memset(text, 0, sizeof(text));
  uint8_t value = GetParameterValue(p.offset);
  const prog_char* values = LongValues(p.unit);
  if (id >= 90 && id <= 93) {
    PerfLabel(value, text, true);
  } else if (p.unit == UNIT_LFO_WAVEFORM && value > LFO_WAVEFORM_WAVE_16) {
    strcpy_P(text, value == LFO_WAVEFORM_EXP ? PSTR("exp ramp") : PSTR("smooth"));
  } else if (p.unit == UNIT_LFO_WAVEFORM && value >= LFO_WAVEFORM_WAVE_1) {
    strcpy_P(text, PSTR("wave "));
    UnsafeItoa<int16_t>(value - LFO_WAVEFORM_WAVE_1 + 1, 3, text + 5);
  } else if (p.offset == PRM_SYS_MIDI_CHANNEL && value == 0) {
    strcpy_P(text, PSTR("omni"));
  } else if (values) {
    CopyNth(values, value, text);
  } else {
    char* t = text;
    if (p.unit == UNIT_INT8 && static_cast<int8_t>(p.min_value) < 0 &&
        static_cast<int8_t>(value) > 0) {
      *t++ = '+';
    }
    p.PrintValue(value, t, kValueWidth);
  }
  uint8_t len = 0;
  while (text[len]) {
    ++len;
  }
  uint8_t w = kLcdWidth - wide;
  uint8_t name_end = wide ? w - len - 1 : 7;
  memset(row, 0, w);  // row_1 + 1 has no room for a terminator.
  uint8_t n = 0;
  while (n < sizeof(long_name_ids) && pgm_read_byte(long_name_ids + n) != id) {
    ++n;
  }
  if (id >= 90 && id <= 93) {
    strcpy_P(row, PSTR("knob "));
    row[5] = '1' + id - 90;
  } else if (id == 96) {
    CopyNth(fx_param_names, part.patch().extra_data_[1], row);
  } else if (n < sizeof(long_name_ids)) {
    CopyNth(long_names, n, row);
  } else {
    ResourcesManager::LoadStringResource(p.long_name, row, kCaptionWidth);
  }
  // Fixed columns: name in 0-6, gap, value right-aligned in 8-15.
  for (uint8_t i = 0; i < w; ++i) {
    char c = i >= w - len ? text[i - (w - len)] :
        (i < name_end ? row[i] : ' ');
    if (c == '\0') {
      c = ' ';
    }
    if (c >= 'A' && c <= 'Z') {
      c += 32;
    }
    row[i] = c;
  }
}

/* static */
void Editor::DisplayEditOverviewPage() {
  // SHA PRM RNG SUB
  // SAW#  0# +0#SQ1#
  char* row_0 = line_buffer_;
  char* row_1 = line_buffer_ + kLcdWidth + 1;
  if (closeup_ && !show_numbers_ && HasCloseup(current_page_)) {
    DrawCloseup(current_page_, row_0);
    if (title_flash_) {  // "SYN  OSC 1   1/3" -> "SYN  OSC 1 1.1/3".
      memcpy_P(row_0, page_titles + kLcdWidth * current_page_, kLcdWidth);
      if (row_0[14] == '/') {
        row_0[11] = row_0[13];
        row_0[12] = '.';
        row_0[13] = '1';
      } else {  // Single page (amp env): "1.1".
        memcpy_P(row_0 + 13, PSTR("1.1"), 3);
      }
      if (current_page_ == PAGE_MOD_ENV_1) {
        memcpy_P(row_0 + 5, PSTR("ENV   "), 6);  // FLT ENV has no room.
      }
    }
    display.set_cursor_position(kLcdNoCursor);
    display.Print(0, row_0);
    display.Print(1, row_1);
    return;
  }
  memset(line_buffer_, ' ', sizeof(line_buffer_));
  row_0[kLcdWidth] = '\0';
  glyph_previous_ = glyph_used_;
  glyph_used_ = 0;
  if (display_mode_ == DISPLAY_MODE_OVERVIEW) {
    memset(touched_, false, sizeof(touched_));
  }
  // Encoder target: steady arrow when browsing, blinking when editing.
  uint8_t marker = current_page_ == PAGE_PERFORM ?
      kLcdNoCursor : cursor_ * kColumnWidth + 3;
  if (title_flash_) {
    marker = kLcdNoCursor;
    memcpy_P(row_0, page_titles + kLcdWidth * current_page_, kLcdWidth);
  } else {
    // Two passes: glyphs that stay on screen claim their slots before a new
    // glyph may take one, else it steals a slot a later cell still needs.
    for (uint8_t pass = 0; pass < 2; ++pass) {
    glyph_probe_ = pass == 0;
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
      if (current_page_ == PAGE_PERFORM) {
        PerfLabel(id, label, false);
      } else {
        memcpy_P(label, page_labels + 12 * current_page_ + 3 * i, 3);
      }
      char* value = row_1 + i * kColumnWidth;
      value[3] = PrintParameter(id, value, 3, label,
          touched_[i] || (i == cursor_ && display_mode_ == DISPLAY_MODE_EDIT));
      ::FillAudioBuffer();
    }
    }
    // A text value being turned takes the whole top row: long name + value.
    uint8_t id = KnobIndexToParameterId(cursor_);
    if (id != 0xff && display_mode_ != DISPLAY_MODE_OVERVIEW &&
        (touched_[cursor_] || display_mode_ == DISPLAY_MODE_EDIT)) {
      const Parameter& p = parameter_manager.parameter(id);
      if (p.unit > UNIT_INT8 && p.unit != UNIT_LFO_RATE) {
        NameValue(id, row_0, false);
        marker = kLcdNoCursor;
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
  if (current_page_ == PAGE_PERFORM) {
    return perf_[knob_index];
  }
  if (current_page_ > PAGE_SYS_PERF) {
    return 0xff;
  }
  uint8_t id = pgm_read_byte(&page_parameters[current_page_][knob_index]);
  if (id != 0xff && (id & 0x80)) {
    // Expansion board n has its 4 parameters at 58 + 4 * n.
    id = 58 + 4 * FILTER_BOARD_PVK + (id & 0x7f);
  }
  return id;
}

// FUNC + pot/encoder (Elektron style): snap to relevant values. Osc range
// jumps by octaves, others by 1/8 of their range; dir 0 = nearest (pots),
// +1/-1 = next/previous (encoder). Lists and small ranges just step by 1.
static uint8_t RelevantValue(uint8_t id, uint8_t value, int8_t dir) {
  const Parameter& p = parameter_manager.parameter(id);
  bool is_signed = p.unit == UNIT_INT8 && static_cast<int8_t>(p.min_value) < 0;
  int16_t lo = is_signed ? static_cast<int8_t>(p.min_value) : p.min_value;
  int16_t hi = is_signed ? static_cast<int8_t>(p.max_value) : p.max_value;
  if (p.unit > UNIT_INT8 || hi - lo < 16) {
    return dir ? p.Increment(value, dir) : value;
  }
  int16_t step = (id == 2 || id == 6) ? 12 : (hi - lo >= 100 ? 16 : 8);
  int16_t k = 16 * step;  // Offset so the divisions below never see negatives.
  int16_t s = (is_signed ? static_cast<int8_t>(value) : value) + k;
  if (dir > 0) {
    s = (s / step + 1) * step;
  } else if (dir < 0) {
    s = ((s + step - 1) / step - 1) * step;
  } else {
    s = (s + step / 2) / step * step;
  }
  s -= k;
  return s < lo ? lo : (s > hi ? hi : s);
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
  touched_[knob_index] = true;
  uint8_t v = parameter.Scale(value);
  SetParameterValue(index, parameter.offset,
      func_ ? RelevantValue(index, v, 0) : v);
}

/* static */
void Editor::OnEditIncrement(int8_t increment) {
  title_flash_ = false;
  if (display_mode_ == DISPLAY_MODE_EDIT) {
    uint8_t id = KnobIndexToParameterId(cursor_);
    if (id != 0xff && func_) {
      const Parameter& p = parameter_manager.parameter(id);
      SetParameterValue(id, p.offset, RelevantValue(
          id, GetParameterValue(p.offset), increment > 0 ? 1 : -1));
    } else if (id != 0xff) {
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
  MOD_DST_FILTER_ENV,
  MOD_DST_FILTER_LFO,
  MOD_DST_VCA,
  MOD_DST_CV_1,
  MOD_DST_CV_2,
  MOD_DST_PVK_MODE,
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
  MOD_DST_FX,
};

/* static */
void Editor::SetParameterValue(
    uint8_t index,
    uint8_t offset,
    uint8_t value) {
  if (offset >= PRM_SYS_OCTAVE) {
    // System settings (also octave on VOICE) and perf knob targets.
    sys_save_at_ = (milliseconds() + 1000) | 1;
  }
  // Dirty hack for the modulation page.
  if (offset >= 0xf0) {
    perf_[offset - 0xf0] = value;
  } else if (current_page_ == PAGE_MOD_MATRIX && offset == PRM_MOD_ROW) {
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
    if (page_definition_[current_page_].group != GROUP_SYS) {
      preset_status_ = 1;
    }
  }
}

/* static */
uint8_t Editor::GetParameterValue(uint8_t offset) {
  uint8_t value;
  if (offset >= 0xf0) {
    value = perf_[offset - 0xf0];
  } else if (current_page_ == PAGE_MOD_MATRIX && offset == PRM_MOD_ROW) {
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
void Editor::DisplaySplashScreen() {
  // SHRUTHI, ONE, POLI, VOKS in the tall font, each popping on for 550 ms
  // after 200 ms of blank (3 s), then the PRESET screen.
  boot_step_ = 0;
  boot_word_ = 0;
  popup_phase_ = 0xfe;
  BootStep();
  set_edited_item_number(part.system_settings().last_patch);
  deferred_load_ = current_patch_number_;
  ToggleLoadSaveAction();
}

/* static */
void Editor::BootStep() {
  if (boot_step_ & 1) {
    display.Print(0, line_buffer_);
    display.Print(1, line_buffer_ + kLcdWidth + 1);
    // Screen saver: every word 2 s. Boot: 550 ms, the last one 1 s.
    popup_until_ = milliseconds() + (saver_ ? 2000 : boot_step_ == 9 ? 1000 : 550);
    return;
  }
  memset(line_buffer_, ' ', sizeof(line_buffer_));
  display.Print(0, line_buffer_);
  display.Print(1, line_buffer_ + kLcdWidth + 1);
  // The next word's glyphs upload while the screen is blank.
  uint8_t chars[7];
  uint8_t n = 0;
  uint8_t c;
  while ((c = pgm_read_byte(boot_words + boot_word_++)) != 0xff) {
    chars[n++] = c;
  }
  PrintTall(chars, n, line_buffer_ + ((kLcdWidth - n) >> 1));
  popup_until_ = milliseconds() + 200;
}

/* static */
void Editor::OnConfirmClick() {
  bool yes = cursor_;
  page_tool_ = cursor_;
  current_page_ = confirm_return_page_;  // Back where you were.
  display_mode_ = DISPLAY_MODE_OVERVIEW;
  cursor_ = 0;
  if (yes) {
    (*confirm_page_settings_.callback)();
  }
  Refresh();
}

/* static */
void Editor::OnConfirmIncrement(int8_t increment) {
  if (confirm_page_settings_.callback == &PageTool) {
    cursor_ += increment > 0 ? (cursor_ < 3) : -(cursor_ > 1);
  } else {
    cursor_ = !cursor_;
  }
}

/* static */
void Editor::OnConfirmInput(uint8_t knob_index, uint8_t value) {
}

/* static */
void Editor::DisplayConfirmPage() {
  display.set_cursor_position(kLcdNoCursor);
  if (confirm_page_settings_.callback == &PageTool) {
    // SYN  OSC 1   1/3
    // >copy paste rand
    char* row_1 = line_buffer_ + kLcdWidth + 1;
    memcpy_P(line_buffer_, page_titles + 16 * confirm_return_page_, 16);
    line_buffer_[kLcdWidth] = '\0';
    memcpy_P(row_1, PSTR(" copy paste rand"), 16);  // Print stops at 16.
    row_1[5 * (cursor_ - 1) + (cursor_ == 3)] = 0x7e;
    display.Print(0, line_buffer_);
    display.Print(1, row_1);
    return;
  }
  // Every yes/no screen: question on top, " yes  no" below, arrow on the choice.
  memset(line_buffer_, ' ', sizeof(line_buffer_));
  char* row_1 = line_buffer_ + kLcdWidth + 1;
  if (confirm_page_settings_.callback == &QuickSave) {
    memcpy_P(line_buffer_, PSTR("SAVE"), 4);
    memcpy(line_buffer_ + 5, part.patch().name, kPatchNameSize);
    line_buffer_[5 + kPatchNameSize] = '?';
  } else {
    const prog_char* q = confirm_page_settings_.callback == &RandomizePage ?
        PSTR("RANDOMIZE PAGE?") : PSTR("MIDI BACKUP?");
    memcpy_P(line_buffer_, q, strlen_P(q));
  }
  memcpy_P(row_1, PSTR(" yes  no"), 8);
  row_1[cursor_ ? 0 : 5] = 0x7e;
  display.Print(0, line_buffer_);
  display.Print(1, row_1);
}

}  // namespace shruthi
