#!/usr/bin/env python3
"""Converts Shruthi-1 patches in a .syx file to the elektron-ui format.

Usage: patch_translator.py in.syx out.syx [--bpm 160]

Handles single patch dumps (command 0x01) and full backups (bulk dump,
commands 0x40-0x43: internal and external patch memory). Other messages are
copied unchanged. Single dumps stay 92 bytes (version ', the firmware does the
rest on load); old full backups are repacked to the 66-byte EEPROM layout
(version +): 30 internal slots, 124 per bank, patches kept in the same order.

- All older versions: SEQ, SEQ 1-2, ARP STEP and CV 1-4 modulation sources
  are gone (they were always 0 on this unit): those slots get amount 0; the
  other sources and the operator inputs are renumbered; the "seq mix" option
  stays and becomes "cc mix" (sources switched by CC 3).
- Before '+': FX drive/fold/crush are gone (-> off); ring and delay renumbered.
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
VERSIONS = OLD_VERSIONS + tuple(b"'()*+")
ERASED = bytes([0xff])
INTERNAL_EEPROM = 2048
BANK_SIZE = 8192
PACKED_SIZE = 66
NUM_INTERNAL = (INTERNAL_EEPROM - 4 - 16) // PACKED_SIZE  # Perf knobs at 2044.
NUM_EXTERNAL = (BANK_SIZE - 2) // PACKED_SIZE  # Bank marker in the last 2 bytes.
# Bits per patch byte, LSB first, 0x80 = signed. Same as kPatchBits in storage.cc.
PATCH_BITS = ([6, 7, 0x86, 4, 6, 7, 0x86, 7, 6, 6, 6, 4, 7, 6, 6, 6] + [7] * 8 +
              [5, 0x87, 4, 3] * 2 + [5, 5, 0x87] * 12 + [8] * 8 +
              [8, 2, 7, 7, 7, 0, 0, 0, 1, 1, 5, 8, 8, 8, 8, 0])

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
  if p[89] == ord('n') and p[90] == ord('g'):
    p[84:91] = bytes(7)  # Pre-1.0 patch: these bytes held text, not settings.
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
  if p[91] in (ord('!'), ord('%')):
    for i in range(2):
      w, r, _, m = p[24 + 4 * i:28 + 4 * i]
      p[24 + 4 * i:28 + 4 * i] = convert_lfo(w, r, m, bpm)
  p[91] = VERSION
  return True


def to_plus(p):
  """Version ' to + in place, as Patch::Update does, plus the storage limits."""
  v = p[91]
  if v == ord("'"):
    p[77:80] = [0, 0, 0]  # FX off.
  if v in (ord("'"), ord('(')):
    p[80] = 127  # Full volume.
  if v != ord('+'):
    p[77] = {4: 1, 5: 2}.get(p[77], 0)
  p[81:84] = [0, 0, 0]
  p[84], p[85] = p[84] != 0, p[85] != 0  # PVK overdrive, FM: on/off.
  p[86] = (p[86] >= 16) << 4 | (p[86] & 15)  # PVK: not LP plays as BP.
  p[91] = ord('+')


def pack(p):
  out, bit = bytearray(PACKED_SIZE), 0
  for v, w in zip(p, PATCH_BITS):
    n = w & 15
    if n == 1:
      v = int(v != 0)
    elif w & 0x80:
      assert -(1 << (n - 1)) <= (v ^ 0x80) - 0x80 < 1 << (n - 1), (bit, v)
    else:
      assert n == 0 or v < 1 << n, (bit, v)
    for j in range(n):
      out[(bit + j) >> 3] |= ((v >> j) & 1) << ((bit + j) & 7)
    bit += n
  return out


def unpack(b):
  p, bit = bytearray(PATCH_SIZE), 0
  for i, w in enumerate(PATCH_BITS):
    n = w & 15
    v = sum(((b[(bit + j) >> 3] >> ((bit + j) & 7)) & 1) << j for j in range(n))
    if w & 0x80 and v >> (n - 1):
      v |= (0xff << n) & 0xff
    p[i] = v
    bit += n
  p[91] = ord('+') if p[0] < 36 else 0  # WAVEFORM_LAST; erased = 63.
  return p


def nibbles_to_bytes(n):
  return bytearray((n[i] << 4) | n[i + 1] for i in range(0, len(n), 2))


def message(command, argument, data):
  out = bytearray(HEADER) + bytes([command, argument])
  for b in data:
    out += bytes([b >> 4, b & 0xf])
  checksum = sum(data) & 0xff
  return out + bytes([checksum >> 4, checksum & 0xf, 0xf7])


def patch_addresses(size):
  """Slots of the old 92-byte layout."""
  for i in range(16):
    yield 16 + i * PATCH_SIZE
  bank = INTERNAL_EEPROM
  while bank + 64 * PATCH_SIZE <= size:
    for i in range(64):
      yield bank + i * PATCH_SIZE
    bank += BANK_SIZE


def packed_addresses(size):
  for i in range(NUM_INTERNAL):
    yield 16 + i * PACKED_SIZE
  for bank in range(INTERNAL_EEPROM, size - BANK_SIZE + 1, BANK_SIZE):
    for i in range(NUM_EXTERNAL):
      yield bank + i * PACKED_SIZE


def repack(memory, size, bpm):
  """Old 92-byte layout -> packed, same slot order. Returns patches written."""
  old = [memory[a:a + PATCH_SIZE] for a in patch_addresses(size)]
  if not all(p[91] in VERSIONS or p == ERASED * PATCH_SIZE for p in old):
    return 0  # Already packed (or not a patch memory image).
  memory[16:INTERNAL_EEPROM - 4] = ERASED * (INTERNAL_EEPROM - 20)
  for bank in range(INTERNAL_EEPROM, size - BANK_SIZE + 1, BANK_SIZE):
    memory[bank:bank + BANK_SIZE - 2] = ERASED * (BANK_SIZE - 2)
  n = 0
  for a, p in zip(packed_addresses(size), old):
    if p[91] in VERSIONS:
      convert_patch(p, bpm)
      to_plus(p)
      memory[a:a + PACKED_SIZE] = pack(p)
      n += 1
  return n


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
    converted += repack(memory, max(a + n for _, _, a, n in bulk), bpm)
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
  assert p[28] == 5 and p[31] == 0 and p[3] == 13 and p[91] == VERSION
  assert not convert_patch(p, 120)  # Already converted.
  syx = message(0x01, 0, bytearray(p[:91]) + b'!')
  out, n = translate(syx + message(0x40, 0, bytes(128)), 120)
  assert n == 1 and out[-1] == 0xf7 and len(out) == len(syx) + 128 * 2 + 11
  q = bytearray(p)
  q[2], q[34], q[77], q[84], q[86], q[91] = 0xfb, 0xc1, 5, 50, 0x21, ord('*')
  to_plus(q)
  assert q[77] == 2 and q[84] == 1 and q[86] == 0x11 and q[91] == ord('+')
  assert unpack(pack(q)) == q and unpack(ERASED * PACKED_SIZE)[91] == 0
  assert NUM_INTERNAL == 30 and NUM_EXTERNAL == 124
  assert sum(w & 15 for w in PATCH_BITS) == 8 * PACKED_SIZE
  memory = bytearray(ERASED * (INTERNAL_EEPROM + BANK_SIZE))
  memory[INTERNAL_EEPROM:INTERNAL_EEPROM + PATCH_SIZE] = q  # Old slot 17.
  assert repack(memory, len(memory), 120) == 1
  assert unpack(memory[16 + 16 * PACKED_SIZE:][:PACKED_SIZE]) == q
  assert repack(memory, len(memory), 120) == 0  # Already packed.


if __name__ == '__main__':
  self_test()
  parser = argparse.ArgumentParser(description=__doc__.split('\n')[0])
  parser.add_argument('input')
  parser.add_argument('output')
  parser.add_argument('--bpm', type=float, default=160,
                      help='tempo used for the free LFO rates of old patches')
  args = parser.parse_args()
  data, n = translate(open(args.input, 'rb').read(), args.bpm)
  open(args.output, 'wb').write(data)
  print('%d patch(es) converted -> %s' % (n, args.output))
