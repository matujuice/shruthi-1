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
// Parameter editor.

#ifndef SHRUTHI_EDITOR_H_
#define SHRUTHI_EDITOR_H_

#include "avrlib/base.h"
#include "shruthi/patch.h"
#include "shruthi/resources.h"
#include "shruthi/shruthi.h"

namespace avrlib {
  class Event;
}

namespace shruthi {

class ParameterDefinition;

// These 3 modes determine within which group of pages the switches navigate.
enum EditorMode {
  EDITOR_MODE_PATCH,
  EDITOR_MODE_SEQUENCE,
};

enum LoadSaveTarget {
  LOAD_SAVE_TARGET_PATCH = 1,
  LOAD_SAVE_TARGET_SEQUENCE = 2,
  LOAD_SAVE_TARGET_BOTH = 3
};

enum DisplayMode {
  // Show the parameter names.
  DISPLAY_MODE_OVERVIEW = 0,
  // Show a single parameter without pesky acronyms.
  DISPLAY_MODE_EDIT = 1,
  // Show a single parameter without pesky acronyms, and revert to overview
  // mode after a while
  DISPLAY_MODE_EDIT_TEMPORARY = 2,
};

enum Group {
  GROUP_SYN,
  GROUP_FLTR,
  GROUP_AMP,
  GROUP_MOD,
  // FUNC + S1..S4.
  GROUP_FX,
  GROUP_VCF,
  GROUP_VOICE,
  GROUP_MOD_MATRIX,

  GROUP_SYS,
  GROUP_LOAD_SAVE,
  GROUP_CONFIRM,
  GROUP_PERFORM,
  GROUP_LAST
};

enum Page {
  PAGE_OSC_OSC_1,
  PAGE_OSC_OSC_2,
  PAGE_OSC_OSC_MIX,
  PAGE_FILTER_FILTER,
  PAGE_MOD_ENV_1,
  PAGE_MOD_ENV_2,
  PAGE_MOD_LFO_1,
  PAGE_MOD_LFO_2,
  PAGE_FX,
  PAGE_FILTER_MULTIMODE,
  PAGE_FX_2,
  PAGE_VOICE,
  PAGE_MOD_MATRIX,
  PAGE_MOD_OPERATORS,
  PAGE_SYS_MIDI,
  PAGE_SYS_DISPLAY,
  PAGE_SYS_PERF,

  PAGE_LOAD_SAVE,
  PAGE_CONFIRM,
  PAGE_PERFORM
};

enum Action {
  ACTION_LOAD,
  ACTION_SAVE,
  ACTION_COMPARE,
  ACTION_MENU,  // PATCH list on the PRESET screen.
};

// We do not use enums here because they take 2 bytes, not 1.
typedef uint8_t ParameterGroup;
typedef uint8_t ParameterPage;

// Size (in char) of the display elements.
static const uint8_t kCaptionWidth = 10;
static const uint8_t kValueWidth = 6;
static const uint8_t kColumnWidth = 4;

enum PageUiType {
  PARAMETER_EDITOR,
  LOAD_SAVE,
  CONFIRM,
  SYS_MENU,
};

struct PageDefinition {
  ParameterPage next;
  ParameterGroup group;
  uint8_t ui_type;
  uint8_t leds_pattern;
};

// For each type of page (basic parameter editor, step sequencer, load/save
// dialog, 4 functions must be defined:
// - a function displaying the "overview" page.
// - a function displaying a specific parameter value ("details").
// - a function handling a change in one of the 4 editing pots.
// - a function handling presses on the increment/decrement buttons.
struct UiHandler {
  void (*overview_page)();
  void (*edit_page)();
  void (*input_handler)(uint8_t knob_index, uint8_t value);
  void (*increment_handler)(int8_t increment);
  void (*click_handler)();
};

struct ConfirmPageSettings {
  void (*callback)();
};

class Editor {
 public:
  Editor() { }
  static void Init();

  // Ons a press on a switch.
  static void OnSwitch(const avrlib::Event& event);

  // Ons the modification of one of the editing pots.
  static void OnInput(uint8_t knob_index, uint8_t value);

  // Ons a rotation of the encoder.
  static void OnIncrement(int8_t increment);

  // Ons a click on the encoder.
  static void OnClick();
  // Ons a long click on the encoder.
  
  // The editor can react to a note on. This is used for step-by-step
  // pattern programming. This function returns true when the note has been
  // recorded in the sequence. In this case, it is not processed!
  static uint8_t OnNoteOn(uint8_t note, uint16_t velocity);


  // Displays variants of the current page.
  static void Refresh();
  static void ScreenSaver();
  
  // Notify the editor that the user has not provided any input for 2s.
  static bool Relax();
  
  // Displays two lines of text read from a resource.
  static void DisplaySplashScreen();
  static void BootStep();

  static inline ParameterPage current_page() { return current_page_; }
  static inline uint8_t current_mode() { return editor_mode_; }
  static inline uint8_t leds_pattern() {
    return page_definition_[current_page_].leds_pattern;
  }
  static inline uint8_t cursor() { return cursor_; }
  static inline bool show_numbers() { return show_numbers_; }
  static inline void set_show_numbers(bool b) { show_numbers_ = b; }
  static inline void set_func(bool b) { func_ = b; }
  static inline uint8_t subpage() { return subpage_; }
  
  static void Confirm(ConfirmPageSettings confirm_page_settings);

  
  static void RandomizeParameter(uint8_t subpage, uint8_t parameter_index);
  static void RandomizePatch();

  static inline uint16_t current_patch_number() {
    return current_patch_number_;
  }
  
  static inline void set_current_patch_number(uint16_t patch_number) {
    current_patch_number_ = patch_number;
    loaded_patch_ = patch_number;  // MIDI program change.
    deferred_load_ = patch_number;  // Loaded with the LOADED popup.
  }
  
  static bool FinishLoadingPatch();

  // FUNC + encoder click: reload the stored version of the current patch.
  // Also shows PATCH RELOADED for 1 s.
  static void ReloadPatch();
  // True when the popup blinks or times out (redraw).
  static bool PopupTick();

  // Sends at most one changed custom glyph to the LCD CGRAM.
  static void UploadGlyphs();
  static bool display_idle();
  // Holding SYS in perform mode: back to SETTINGS > PERFORM MODE.
  static void ExitPerform();
  
 private:
  // A bunch of hacks for special values/pages.
  static void SetParameterValue(uint8_t id, uint8_t offset, uint8_t value);
  static void IncrementParameterValue(uint8_t id, int8_t increment);
  static uint8_t GetParameterValue(uint8_t offset);

  // Returns the parameter id of the parameter that should be edited when
  // touching knob #knob_index.
  static uint8_t KnobIndexToParameterId(uint8_t knob_index);
  static void DrawCloseup(uint8_t page, char* row_0);

  static void JumpToPageGroup(uint8_t id);
  static void PageChange();

  // Output and Input handling for all the different category of pages.
  static void DisplayEditOverviewPage();
  // Prints a parameter value in `width` chars, returns its meter/icon glyph.
  static char PrintParameter(
      uint8_t id, char* out, uint8_t width, char* label, bool touched);
  
  static void OnEditInput(uint8_t knob_index, uint8_t value);
  static void OnEditIncrement(int8_t increment);

  
  static void DisplayConfirmPage();
  static void OnConfirmInput(uint8_t knob_index, uint8_t value);
  static void OnConfirmIncrement(int8_t increment);
  static void OnConfirmClick();

  static void DisplayLoadSavePage();
  static void ToggleLoadSaveAction();
  static void OnPatchMenuClick();
  static uint8_t PatchMenu(uint8_t* items);
  static void QuickSave();
 public:
  static void QuickSaveConfirm();  // FUNC + encoder hold.
  static void PageToolsMenu();  // Encoder hold on a synth page.
  static void PageTool();
  static void RandomizePage();
  static uint8_t ToolKnob(uint8_t k);
 private:
  static void ShowPatchPopup(const prog_char* text);
  static void OnLoadSaveIncrement(int8_t increment);
  static void OnLoadSaveClick();

  static void DisplaySysPage();
  static void OnSysInput(uint8_t knob_index, uint8_t value);
  static void OnSysIncrement(int8_t increment);
  static void OnSysClick();
  static void NameValue(uint8_t id, char* row, bool wide);
  static uint8_t SysItems();  // Settings rows on the current SYS page.
  
  static void BackupEditBuffer();
  static void RestoreEditBuffer();
  static uint16_t edited_item_number();
  static void set_edited_item_number(int16_t value);
  static uint8_t is_cursor_at_valid_position();
  
  static void LoadPatch(uint8_t index);
  
 public:
  static void SaveSystemSettings();
 private:
  static void StartMidiBackup();
  static void PerfLabel(uint8_t id, char* out, bool long_name);
  static uint8_t perf_[4];  // Patch parameter ids of the preset-page knobs.
  
  
  static PageDefinition page_definition_[];
  static const UiHandler ui_handler_[];

  static ParameterPage current_page_;
  static ParameterPage last_visited_page_[GROUP_LAST];
  
  static uint8_t last_visited_group_[2];
  static uint8_t display_mode_;
  static uint8_t editor_mode_;
  static uint8_t cursor_;
  static uint8_t page_tool_;  // Menu choice: 1 rand, 2 copy, 3 paste.
  static uint8_t clip_page_;  // 0xff = nothing copied.
  static uint8_t clip_[4];
  static uint8_t last_knob_;
  static uint8_t last_visited_subpage_;

  static char line_buffer_[kLcdWidth * kLcdHeight + 1];

  static uint8_t subpage_;
  static uint8_t action_;
  static uint16_t current_patch_number_;  // Shown (browsed) on PRESET.
  static uint16_t loaded_patch_;
  static uint16_t current_sequence_number_;
  static ConfirmPageSettings confirm_page_settings_;
  static ParameterPage confirm_return_page_;

  static bool empty_patch_;

  
  static bool snapped_[4];
  static bool touched_[4];  // Knobs moved since the overview came back.
  static bool title_flash_;
  static bool show_numbers_;
  static bool sys_open_;  // false = SYS menu list, true = inside a sub-menu.
  static uint32_t sys_save_at_;  // 0: settings saved.
  static bool perf_setup_;  // PERFORM MODE > setup (knob targets) is open.
  static bool func_;  // FUNC held: pots and encoder jump to relevant values.
  
  static uint16_t deferred_load_;
  static uint32_t popup_until_;  // 0 = no popup.
  static uint8_t popup_phase_;  // 1 = to draw, 3 = shown, 0xfe+ = boot.

  DISALLOW_COPY_AND_ASSIGN(Editor);
};

extern Editor editor;

}  // namespace shruthi

#endif // SHRUTHI_EDITOR_H_
