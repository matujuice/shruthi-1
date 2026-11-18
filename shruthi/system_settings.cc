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
// System settings.

#include "shruthi/system_settings.h"

#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <string.h>

namespace shruthi {

// Writes only bytes that differ: each EEPROM write blocks ~3.4 ms, and the
// 32-byte MIDI input buffer overflows after ~10 ms.
void EepromUpdate(uint8_t* address, const uint8_t* data, uint8_t size) {
  for (uint8_t i = 0; i < size; ++i) {
    if (eeprom_read_byte(address + i) != data[i]) {
      eeprom_write_byte(address + i, data[i]);
    }
  }
}

void SystemSettings::EepromSave() const {
  const uint8_t* data = (const uint8_t *)(this);
  uint8_t checksum = 0;
  for (int16_t i = 0; i < sizeof(SystemSettings); ++i) {
    checksum += data[i];
  }
  checksum = -checksum;
  EepromUpdate((uint8_t*)(0), data, sizeof(SystemSettings));
  EepromUpdate((uint8_t*)(sizeof(SystemSettings)), &checksum, 1);
}

uint8_t SystemSettings::EepromLoad() {
  uint8_t buffer[sizeof(SystemSettings) + 1];
  uint8_t* source = (uint8_t*)(0);
  uint8_t checksum = 0;
  eeprom_read_block(buffer, source, sizeof(SystemSettings) + 1);
  for (int16_t i = 0; i < sizeof(SystemSettings) + 1; ++i) {
    checksum += buffer[i];
  }
  if (checksum == 0) {
    uint8_t* data = (uint8_t *)(this);
    memcpy(data, buffer, sizeof(SystemSettings));
  }
  return checksum == 0;
}

}  // shruthi
