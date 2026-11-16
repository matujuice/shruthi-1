#!/usr/bin/env python3
"""Converts Shruthi-1 patches in a .syx file to the elektron-ui format (version ').

Usage: patch_translator.py in.syx out.syx [--bpm 120]

Handles single patch dumps (command 0x01) and full backups (bulk dump,
commands 0x40-0x43: internal and external patch memory). Other messages are
copied unchanged.

- All older versions: SEQ, SEQ 1-2, ARP STEP and CV 1-4 modulation sources
  are gone (they were always 0 on this unit): those slots get amount 0; the
  other sources and the operator inputs are renumbered; the "seq mix" option
  becomes "sum".
- '!' and '%' (original firmware): the LFOs become tempo-locked SPD/MUL.
  Step-synced rates keep their length (one cycle = n sixteenths); free rates
  are matched at --bpm. Fade-in is dropped. STEP SEQ waveform -> triangle.
"""
import argparse
import sys

HEADER = bytes([0xf0, 0x00, 0x21, 0x02, 0x00, 0x02])
PATCH_SIZE = 92
VERSION = ord("'")
OLD_VERSIONS = (ord('!'), ord('%'), ord('&'))
INTERNAL_EEPROM = 2048
BANK_SIZE = 8192

# lut_res_lfo_increments of the original firmware (phase increment per
# control block at 39215.7 / 40 Hz).
OLD_LFO_INCREMENTS = [
    4, 4, 4, 4, 5, 5, 5, 6, 6, 7, 7, 7, 8, 8, 9, 9,
    10, 11, 11, 12, 13, 14, 14, 15, 16, 17, 18, 20, 21, 22, 23, 25,
    26, 28, 30, 31, 33, 35, 37, 40, 42, 45, 47, 50, 53, 57, 60, 64,
    67, 71, 76, 80, 85, 90, 96, 101, 108, 114, 121, 128, 136, 144, 153, 162,
    172, 182, 193, 204, 217, 230, 243, 258, 273, 290, 307, 325, 345, 366, 388, 411,
    435, 461, 489, 518, 549, 582, 617, 654, 693, 735, 779, 825, 875, 927, 982, 1041,
    1104, 1170, 1240, 1314, 1392, 1476, 1564, 1657, 1757, 1862, 1973, 2091, 2216, 2349, 2489, 2638,
    2796, 2963, 3141, 3329, 3528, 3739, 3962, 4199, 4451, 4717, 4999, 5298, 5615, 5951, 6307, 6684,
]
CONTROL_RATE = 20000000 / 510 / 40


def remap_source(s):
  """Old source number -> new one, None if removed (same as Patch::Update)."""
  if s >= 14:
    return s - 8
  if s >= 10:
    return None
  if s >= 6:
    return s - 4
  return None if s >= 2 else s


def speed_multiplier(t):
  """SPD x 2^MUL closest to t (cycles per beat x 512), SPD 1..63, MUL 0..11."""
  for mul in range(12):
    if t / (1 << mul) <= 63 or mul == 11:
      return max(1, min(63, round(t / (1 << mul)))), mul


def convert_lfo(waveform, rate, mode, bpm):
  if waveform == 4:
    waveform = 0  # Step sequencer -> triangle.
  elif waveform > 4:
    waveform -= 1
  if rate < 16:
    t = 2048 / (rate + 1)  # One cycle = rate + 1 sixteenths.
  else:
    hz = OLD_LFO_INCREMENTS[min(rate, 127)] * CONTROL_RATE / 65536
    t = hz * 60 / bpm * 512
  speed, mul = speed_multiplier(t)
  # free, slave (reset on note), master, one shot -> free, trigger, free, one shot.
  mode = {0: 0, 1: 1, 2: 0, 3: 3}.get(mode, 0)
  return [waveform, speed, mul, mode]


def convert_patch(p, bpm):
  """Converts 92 bytes in place. Returns False if not a patch to convert."""
  if p[91] not in OLD_VERSIONS or any(b >= 128 for b in p[8:24]):
    return False
  for i in range(12):
    s = remap_source(p[32 + 3 * i])
    if s is None:
      s = 0
      p[32 + 3 * i + 2] = 0
    p[32 + 3 * i] = s
  # Operator inputs: 6 bits each, op_data[0..3] at 87..90.
  for j in (87, 89):
    a = p[j] & 0x3f
    b = (p[j + 1] >> 3) | (0x20 if p[j] & 0x40 else 0)
    a, b = (remap_source(x) for x in (a, b))
    a = 0 if a is None else a
    b = 0 if b is None else b
    p[j] = (p[j] & 0x80) | a | (0x40 if b & 0x20 else 0)
    p[j + 1] = ((b & 0x1f) << 3) | (p[j + 1] & 7)
  if p[3] == 13:
    p[3] = 0  # Seq mix -> sum.
  if p[91] in (ord('!'), ord('%')):
    for i in range(2):
      w, r, _, m = p[24 + 4 * i:28 + 4 * i]
      p[24 + 4 * i:28 + 4 * i] = convert_lfo(w, r, m, bpm)
  p[91] = VERSION
  return True


def nibbles_to_bytes(n):
  return bytearray((n[i] << 4) | n[i + 1] for i in range(0, len(n), 2))


def message(command, argument, data):
  out = bytearray(HEADER) + bytes([command, argument])
  for b in data:
    out += bytes([b >> 4, b & 0xf])
  checksum = sum(data) & 0xff
  return out + bytes([checksum >> 4, checksum & 0xf, 0xf7])


def patch_addresses(size):
  for i in range(16):
    yield 16 + i * PATCH_SIZE
  bank = INTERNAL_EEPROM
  while bank + 64 * PATCH_SIZE <= size:
    for i in range(64):
      yield bank + i * PATCH_SIZE
    bank += BANK_SIZE


def translate(syx, bpm):
  """Returns (converted .syx bytes, number of patches converted)."""
  messages, start = [], None
  for i, b in enumerate(syx):
    if b == 0xf0:
      start = i
    elif b == 0xf7 and start is not None:
      messages.append(bytes(syx[start:i + 1]))
      start = None
  out, converted = [], 0  # Bulk blocks are None until rewritten.
  bulk, memory = [], bytearray(4 * 16384)
  for m in messages:
    if not m.startswith(HEADER) or len(m) < 11:
      out.append(m)
      continue
    command, argument = m[6], m[7]
    data = nibbles_to_bytes(m[8:-3])
    if (sum(data) & 0xff) != nibbles_to_bytes(m[-3:-1])[0]:
      sys.exit('checksum error in message %02x %02x' % (command, argument))
    if command == 0x01 and len(data) == PATCH_SIZE:
      converted += convert_patch(data, bpm)
      out.append(message(command, argument, data))
    elif 0x40 <= command <= 0x43:
      address = (command - 0x40) * 16384 + argument * 128
      memory[address:address + len(data)] = data
      bulk.append((command, argument, address, len(data)))
      out.append(None)
    else:
      out.append(m)
  if bulk:
    size = max(a + n for _, _, a, n in bulk)
    for a in patch_addresses(size):
      if a + PATCH_SIZE > size:
        break
      p = memory[a:a + PATCH_SIZE]
      if convert_patch(p, bpm):
        memory[a:a + PATCH_SIZE] = p
        converted += 1
    blocks = iter(message(c, g, memory[a:a + n]) for c, g, a, n in bulk)
    out = [next(blocks) if x is None else x for x in out]
  return b''.join(out), converted


def self_test():
  p = bytearray(PATCH_SIZE)
  p[24:32] = [4, 3, 0, 1, 6, 16, 9, 2]  # Step seq 4 steps; wave 1, slowest.
  p[32:35] = [2, 0, 40]  # SEQ -> removed.
  p[35:38] = [6, 0, 40]  # Wheel -> 2.
  p[38:41] = [27, 0, 40]  # Op 2 -> 19.
  p[87], p[88] = 10 | 0x80, (14 << 3) | 3  # Inputs CV 1, CC A.
  p[3] = 13
  p[91] = ord('!')
  assert convert_patch(p, 120)
  assert p[32:41] == bytearray([0, 0, 0, 2, 0, 40, 19, 0, 40]), p[32:41]
  assert p[87] == 0x80 and p[88] == (6 << 3) | 3, (p[87], p[88])
  assert p[24:28] == bytearray([0, 32, 4, 1]), p[24:28]  # 512 = 1 cycle/beat.
  assert p[28] == 5 and p[31] == 0 and p[3] == 0 and p[91] == VERSION
  assert not convert_patch(p, 120)  # Already converted.
  syx = message(0x01, 0, bytearray(p[:91]) + b'!')
  out, n = translate(syx + message(0x40, 0, bytes(128)), 120)
  assert n == 1 and out[-1] == 0xf7 and len(out) == len(syx) + 128 * 2 + 11


if __name__ == '__main__':
  self_test()
  parser = argparse.ArgumentParser(description=__doc__.split('\n')[0])
  parser.add_argument('input')
  parser.add_argument('output')
  parser.add_argument('--bpm', type=float, default=120,
                      help='tempo used for the free LFO rates of old patches')
  args = parser.parse_args()
  data, n = translate(open(args.input, 'rb').read(), args.bpm)
  open(args.output, 'wb').write(data)
  print('%d patch(es) converted -> %s' % (n, args.output))
