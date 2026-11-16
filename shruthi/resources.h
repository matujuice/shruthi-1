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
// Resources definitions.
//
// Automatically generated with:
// make resources


#ifndef SHRUTHI_RESOURCES_H_
#define SHRUTHI_RESOURCES_H_


#include "avrlib/base.h"

#include <avr/pgmspace.h>


#include "avrlib/resources_manager.h"

namespace shruthi {

typedef uint16_t ResourceId;

extern const prog_char* const string_table[];

extern const prog_uint16_t* const lookup_table_table[];


extern const prog_uint8_t* const waveform_table[];

extern const prog_uint8_t* const character_table[];

extern const prog_uint16_t lut_res_lfo_increments[] PROGMEM;
extern const prog_uint16_t lut_res_env_portamento_increments[] PROGMEM;
extern const prog_uint16_t lut_res_oscillator_increments[] PROGMEM;
extern const prog_uint16_t lut_res_fm_frequency_ratios[] PROGMEM;
extern const prog_uint8_t wav_res_formant_sine[] PROGMEM;
extern const prog_uint8_t wav_res_formant_square[] PROGMEM;
extern const prog_uint8_t wav_res_sine[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_square_0[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_square_1[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_square_2[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_square_3[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_square_4[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_square_5[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_saw_0[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_saw_1[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_saw_2[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_saw_3[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_saw_4[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_saw_5[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_triangle_0[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_triangle_3[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_triangle_4[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_triangle_5[] PROGMEM;
extern const prog_uint8_t wav_res_vowel_data[] PROGMEM;
extern const prog_uint8_t wav_res_distortion[] PROGMEM;
extern const prog_uint8_t wav_res_env_expo[] PROGMEM;
extern const prog_uint8_t wav_res_waves[] PROGMEM;
extern const prog_uint8_t wav_res_wavetables[] PROGMEM;
extern const prog_uint8_t wav_res_version[] PROGMEM;
extern const prog_uint8_t chr_res_special_characters[] PROGMEM;
#define STR_RES_SHAPE 0  // shape
#define STR_RES_PARAMETER 1  // parameter
#define STR_RES_RESONANCE 2  // resonance
#define STR_RES_PRM 3  // prm
#define STR_RES_RANGE 4  // range
#define STR_RES_RNG 5  // rng
#define STR_RES_OP_ 6  // op.
#define STR_RES_OPERATOR 7  // operator
#define STR_RES_DETUNE 8  // detune
#define STR_RES_OSC_MIX 9  // osc mix
#define STR_RES_SUB_OSC_ 10  // sub osc.
#define STR_RES_ENV1TVCF 11  // env1~vcf
#define STR_RES_LFO2TVCF 12  // lfo2~vcf
#define STR_RES_ATTACK 13  // attack
#define STR_RES_ATK 14  // atk
#define STR_RES_DECAY 15  // decay
#define STR_RES_SUSTAIN 16  // sustain
#define STR_RES_RELEASE 17  // release
#define STR_RES_RATE 18  // rate
#define STR_RES_MODULATION 19  // modulation
#define STR_RES_MOD_ 20  // mod.
#define STR_RES_SOURCE 21  // source
#define STR_RES_SRC 22  // src
#define STR_RES_DEST_ 23  // dest.
#define STR_RES_DST 24  // dst
#define STR_RES_AMOUNT 25  // amount
#define STR_RES_AMT 26  // amt
#define STR_RES_MODE 27  // mode
#define STR_RES_OVERDRIVE 28  // overdrive
#define STR_RES__FM 29  // fm
#define STR_RES_FLAVOUR 30  // flavour
#define STR_RES_TIME 31  // time
#define STR_RES_LEVEL 32  // level
#define STR_RES_FDB 33  // fdb
#define STR_RES_FEEDBACK 34  // feedback
#define STR_RES_EQ_ 35  // eq.
#define STR_RES_FDBCK_EQ_ 36  // fdbck eq.
#define STR_RES_START 37  // start
#define STR_RES_LENGTH 38  // length
#define STR_RES_OCTAVE 39  // octave
#define STR_RES_PORTA 40  // porta
#define STR_RES_PRT 41  // prt
#define STR_RES_LEGATO 42  // legato
#define STR_RES_TUNING 43  // tuning
#define STR_RES_MIDI_CHAN 44  // midi chan
#define STR_RES_CHN 45  // chn
#define STR_RES_MIDI_OUT 46  // midi out
#define STR_RES_PAUSE 47  // pause
#define STR_RES_SNAP 48  // snap
#define STR_RES_FILTER 49  // filter
#define STR_RES_MODE_1 50  // mode 1
#define STR_RES_MODE_2 51  // mode 2
#define STR_RES_MD1 52  // md1
#define STR_RES_MD2 53  // md2
#define STR_RES_PERFORMANCE 54  // performance
#define STR_RES_PARAM__1 55  // param. 1
#define STR_RES_PARAM__2 56  // param. 2
#define STR_RES_PR1 57  // pr1
#define STR_RES_PR2 58  // pr2
#define STR_RES_FX_ 59  // fx.
#define STR_RES_FX_PROG_ 60  // fx prog.
#define STR_RES_LPF 61  // lpf
#define STR_RES_BPF 62  // bpf
#define STR_RES_HPF 63  // hpf
#define STR_RES_LPL 64  // lp<
#define STR_RES_BPL 65  // bp<
#define STR_RES_HPL 66  // hp<
#define STR_RES_SLP 67  // +lp
#define STR_RES_SBP 68  // +bp
#define STR_RES_SHP 69  // +hp
#define STR_RES_GLP 70  // >lp
#define STR_RES_GBP 71  // >bp
#define STR_RES_GHP 72  // >hp
#define STR_RES_LP4 73  // lp4
#define STR_RES_LP3 74  // lp3
#define STR_RES_LP2 75  // lp2
#define STR_RES_LP1 76  // lp1
#define STR_RES_HP1 77  // hp1
#define STR_RES_HP2 78  // hp2
#define STR_RES_HP3 79  // hp3
#define STR_RES_BP2 80  // bp2
#define STR_RES_BP4 81  // bp4
#define STR_RES_NOTCH 82  // notch
#define STR_RES_PHASER 83  // phaser
#define STR_RES_H2LP 84  // h2lp
#define STR_RES_H3LP 85  // h3lp
#define STR_RES_NSLP 86  // n+lp
#define STR_RES_PSLP 87  // p+lp
#define STR_RES_LP 88  // lp
#define STR_RES_BP 89  // bp
#define STR_RES_HP 90  // hp
#define STR_RES_AP 91  // ap
#define STR_RES_LIQUID 92  // liquid
#define STR_RES_MS 93  // ms
#define STR_RES_WOBBLY 94  // wobbly
#define STR_RES_WHACKY 95  // whacky
#define STR_RES_PW1 96  // pw1
#define STR_RES_PW2 97  // pw2
#define STR_RES_51 98  // 1
#define STR_RES_52 99  // 2
#define STR_RES_5 100  // 
#define STR_RES_5F 101  // f
#define STR_RES_MIX 102  // mix
#define STR_RES_NOI 103  // noi
#define STR_RES_SUB 104  // sub
#define STR_RES_CUT 105  // cut
#define STR_RES_RES 106  // res
#define STR_RES_VCA 107  // vca
#define STR_RES_CV1 108  // cv1
#define STR_RES_CV2 109  // cv2
#define STR_RES_TR1 110  // tr1
#define STR_RES_A1 111  // a1
#define STR_RES_D1 112  // d1
#define STR_RES_S1 113  // s1
#define STR_RES_R1 114  // r1
#define STR_RES_TR2 115  // tr2
#define STR_RES_A2 116  // a2
#define STR_RES_D2 117  // d2
#define STR_RES_S2 118  // s2
#define STR_RES_R2 119  // r2
#define STR_RES__ATK 120  // atk
#define STR_RES__LF1 121  // lf1
#define STR_RES__LF2 122  // lf2
#define STR_RES_IN1 123  // in1
#define STR_RES_IN2 124  // in2
#define STR_RES_OUT 125  // out
#define STR_RES___OFF 126  // off
#define STR_RES_ADD 127  // add
#define STR_RES_PRD 128  // prd
#define STR_RES_MAX 129  // max
#define STR_RES_MIN 130  // min
#define STR_RES__XOR 131  // xor
#define STR_RES_GE 132  // >=
#define STR_RES_LE 133  // <=
#define STR_RES_QTZ 134  // qtz
#define STR_RES_LAG 135  // lag
#define STR_RES_PWM1 136  // pwm1
#define STR_RES_PWM2 137  // pwm2
#define STR_RES_OSC1 138  // osc1
#define STR_RES_OSC2 139  // osc2
#define STR_RES_OSC1S2 140  // osc1+2
#define STR_RES_FINE 141  // fine
#define STR_RES__MIX 142  // mix
#define STR_RES_NOISE 143  // noise
#define STR_RES_SUBOSC 144  // subosc
#define STR_RES_CUTOFF 145  // cutoff
#define STR_RES_RESO 146  // reso
#define STR_RES__VCA 147  // vca
#define STR_RES_CV_1 148  // cv 1
#define STR_RES_CV_2 149  // cv 2
#define STR_RES_TRG_E1 150  // trg e1
#define STR_RES_ATK__1 151  // atk. 1
#define STR_RES_DEC__1 152  // dec. 1
#define STR_RES_SUS__1 153  // sus. 1
#define STR_RES_REL__1 154  // rel. 1
#define STR_RES_TRG_E2 155  // trg e2
#define STR_RES_ATK__2 156  // atk. 2
#define STR_RES_DEC__2 157  // dec. 2
#define STR_RES_SUS__2 158  // sus. 2
#define STR_RES_REL__2 159  // rel. 2
#define STR_RES_ATK1S2 160  // atk1+2
#define STR_RES__LFO_1 161  // lfo 1
#define STR_RES__LFO_2 162  // lfo 2
#define STR_RES_DISTRT 163  // distrt
#define STR_RES_CRUSHR 164  // crushr
#define STR_RES_SCMBFL 165  // +cmbfl
#define STR_RES_CMBFL 166  // -cmbfl
#define STR_RES_RINGMD 167  // ringmd
#define STR_RES_DELAY 168  // delay
#define STR_RES_FB_DLY 169  // fb dly
#define STR_RES_DUBDLY 170  // dubdly
#define STR_RES__FBDLY 171  // !fbdly
#define STR_RES__DBDLY 172  // !dbdly
#define STR_RES__16_DL 173  // /16 dl
#define STR_RES__12_DL 174  // /12 dl
#define STR_RES__8_DLY 175  // /8 dly
#define STR_RES_3_16DL 176  // 3/16dl
#define STR_RES_LOOPER 177  // looper
#define STR_RES_PITCH 178  // pitch
#define STR_RES_LGF 179  // l>f
#define STR_RES_HGF 180  // h>f
#define STR_RES_FGL 181  // f>l
#define STR_RES_FGH 182  // f>h
#define STR_RES_GFX 183  // >fx
#define STR_RES_LF1 184  // lf1
#define STR_RES_LF2 185  // lf2
#define STR_RES_WHL 186  // whl
#define STR_RES_AFT 187  // aft
#define STR_RES_BND 188  // bnd
#define STR_RES_OFS 189  // ofs
#define STR_RES_CCA 190  // ccA
#define STR_RES_CCB 191  // ccB
#define STR_RES_BRE 192  // bre
#define STR_RES_PED 193  // ped
#define STR_RES__NOI 194  // noi
#define STR_RES_EN1 195  // en1
#define STR_RES_EN2 196  // en2
#define STR_RES_VEL 197  // vel
#define STR_RES_RND 198  // rnd
#define STR_RES_NOT 199  // not
#define STR_RES_GAT 200  // gat
#define STR_RES_AUD 201  // aud
#define STR_RES_OP1 202  // op1
#define STR_RES_OP2 203  // op2
#define STR_RES_E4 204  // =4
#define STR_RES_E8 205  // =8
#define STR_RES_E16 206  // =16
#define STR_RES_E32 207  // =32
#define STR_RES_LFO_1 208  // lfo 1
#define STR_RES_LFO_2 209  // lfo 2
#define STR_RES_MWHEEL 210  // mwheel
#define STR_RES_AFTTCH 211  // afttch
#define STR_RES_BENDER 212  // bender
#define STR_RES_OFFSET 213  // offset
#define STR_RES_CC_A 214  // cc A
#define STR_RES_CC_B 215  // cc B
#define STR_RES_BREATH 216  // breath
#define STR_RES_PEDAL 217  // pedal
#define STR_RES__NOISE 218  // noise
#define STR_RES_ENV_1 219  // env 1
#define STR_RES_ENV_2 220  // env 2
#define STR_RES_VELO 221  // velo
#define STR_RES_RANDOM 222  // random
#define STR_RES_NOTE 223  // note
#define STR_RES_GATE 224  // gate
#define STR_RES_AUDIO 225  // audio
#define STR_RES_OP__1 226  // op. 1
#define STR_RES_OP__2 227  // op. 2
#define STR_RES_E_4 228  // = 4
#define STR_RES_E_8 229  // = 8
#define STR_RES_E_16 230  // = 16
#define STR_RES_E_32 231  // = 32
#define STR_RES_PATCH 232  // patch
#define STR_RES_START_FULL_MIDI 233  // start full midi
#define STR_RES_BACKUP_ 234  // backup?
#define STR_RES_BROWSE 235  // browse
#define STR_RES_SAVE 236  // save
#define STR_RES_COMPARE 237  // compare
#define STR_RES_ 238  // ----
#define STR_RES_V 239  // v
#define STR_RES_671____SN__4370 240  // -1    SN #4370
#define STR_RES_V1_03 241  // v1.03
#define STR_RES_SUM 242  // sum
#define STR_RES_SYNC 243  // sync
#define STR_RES_RING 244  // ring
#define STR_RES_XOR 245  // xor
#define STR_RES_FUZZ 246  // fuzz
#define STR_RES_GG4 247  // >>4
#define STR_RES_GG8 248  // >>8
#define STR_RES_FOLD 249  // fold
#define STR_RES_BITS 250  // bits
#define STR_RES_DUO 251  // duo
#define STR_RES_2STEPS 252  // 2steps
#define STR_RES_4STEPS 253  // 4steps
#define STR_RES_8STEPS 254  // 8steps
#define STR_RES_FREE 255  // free
#define STR_RES_SLAVE 256  // slave
#define STR_RES_MASTER 257  // master
#define STR_RES_1SHOT 258  // 1shot
#define STR_RES_1_1 259  // 1/1
#define STR_RES__OFF 260  // off
#define STR_RES_THRU 261  // thru
#define STR_RES_SEQ 262  // seq
#define STR_RES_CTRL 263  // ctrl
#define STR_RES_FULL 264  // full
#define STR_RES_1TV 265  // 1~|
#define STR_RES_2T1 266  // 2~1
#define STR_RES_3T2 267  // 3~2
#define STR_RES_4T3 268  // 4~3
#define STR_RES_5T4 269  // 5~4
#define STR_RES_6T5 270  // 6~5
#define STR_RES_7T6 271  // 7~6
#define STR_RES_8T7 272  // 8~7
#define STR_RES_TRI 273  // tri
#define STR_RES_SQR 274  // sqr
#define STR_RES_S_H 275  // s&h
#define STR_RES_RAMP 276  // ramp
#define STR_RES_STEP 277  // step
#define STR_RES_3 278  // 
#define STR_RES_4 279  // 
#define STR_RES_34 280  // 
#define STR_RES__ 281  // ?
#define STR_RES_PLAYED 282  // played
#define STR_RES_OFF 283  // off
#define STR_RES_ON 284  // on
#define STR_RES_SQ1 285  // sq1
#define STR_RES__TR1 286  // tr1
#define STR_RES_PL1 287  // pl1
#define STR_RES_SQ2 288  // sq2
#define STR_RES__TR2 289  // tr2
#define STR_RES_PL2 290  // pl2
#define STR_RES_CLICK 291  // click
#define STR_RES_GLITCH 292  // glitch
#define STR_RES_BLOW 293  // blow
#define STR_RES_METAL 294  // metal
#define STR_RES_POP 295  // pop
#define STR_RES_NONE 296  // none
#define STR_RES_SAW 297  // saw
#define STR_RES_SQUARE 298  // square
#define STR_RES_TRIANG 299  // triang
#define STR_RES_ZSAW 300  // zsaw
#define STR_RES_ZRESO 301  // zreso
#define STR_RES_ZTRI 302  // ztri
#define STR_RES_ZPULSE 303  // zpulse
#define STR_RES_ZSYNC 304  // zsync
#define STR_RES_PAD 305  // pad
#define STR_RES_FM 306  // fm
#define STR_RES_WAVES 307  // waves
#define STR_RES_TAMPUR 308  // tampur
#define STR_RES_DIGITL 309  // digitl
#define STR_RES_METALL 310  // metall
#define STR_RES_BOWED 311  // bowed
#define STR_RES_SLAP 312  // slap
#define STR_RES_ORGAN 313  // organ
#define STR_RES_MALE 314  // male
#define STR_RES_USER 315  // user
#define STR_RES_8BITS 316  // 8bits
#define STR_RES_CRUSH 317  // crush
#define STR_RES_PWM 318  // pwm
#define STR_RES___NOISE 319  // noise
#define STR_RES_VOWEL 320  // vowel
#define STR_RES_BELISH 321  // belish
#define STR_RES_POLATD 322  // polatd
#define STR_RES_CELLO 323  // cello
#define STR_RES_CLPSWP 324  // clpswp
#define STR_RES_FEMALE 325  // female
#define STR_RES_FMTVOC 326  // fmtvoc
#define STR_RES_FRMNT2 327  // frmnt2
#define STR_RES_RES3HP 328  // res3hp
#define STR_RES_ELECTP 329  // electp
#define STR_RES_VIBES 330  // vibes
#define STR_RES__LPF 331  // lpf
#define STR_RES_SSM 332  // ssm
#define STR_RES_SVF 333  // svf
#define STR_RES_DSP 334  // dsp
#define STR_RES_PVK 335  // pvk
#define STR_RES_4PM 336  // 4pm
#define STR_RES_DLY 337  // dly
#define STR_RES_SP 338  // sp
#define LUT_RES_LFO_INCREMENTS 0
#define LUT_RES_LFO_INCREMENTS_SIZE 128
#define LUT_RES_ENV_PORTAMENTO_INCREMENTS 1
#define LUT_RES_ENV_PORTAMENTO_INCREMENTS_SIZE 128
#define LUT_RES_OSCILLATOR_INCREMENTS 2
#define LUT_RES_OSCILLATOR_INCREMENTS_SIZE 97
#define LUT_RES_FM_FREQUENCY_RATIOS 3
#define LUT_RES_FM_FREQUENCY_RATIOS_SIZE 25
#define WAV_RES_FORMANT_SINE 0
#define WAV_RES_FORMANT_SINE_SIZE 256
#define WAV_RES_FORMANT_SQUARE 1
#define WAV_RES_FORMANT_SQUARE_SIZE 256
#define WAV_RES_SINE 2
#define WAV_RES_SINE_SIZE 257
#define WAV_RES_BANDLIMITED_SQUARE_0 3
#define WAV_RES_BANDLIMITED_SQUARE_0_SIZE 257
#define WAV_RES_BANDLIMITED_SQUARE_1 4
#define WAV_RES_BANDLIMITED_SQUARE_1_SIZE 257
#define WAV_RES_BANDLIMITED_SQUARE_2 5
#define WAV_RES_BANDLIMITED_SQUARE_2_SIZE 257
#define WAV_RES_BANDLIMITED_SQUARE_3 6
#define WAV_RES_BANDLIMITED_SQUARE_3_SIZE 257
#define WAV_RES_BANDLIMITED_SQUARE_4 7
#define WAV_RES_BANDLIMITED_SQUARE_4_SIZE 257
#define WAV_RES_BANDLIMITED_SQUARE_5 8
#define WAV_RES_BANDLIMITED_SQUARE_5_SIZE 257
#define WAV_RES_BANDLIMITED_SQUARE_6 9
#define WAV_RES_BANDLIMITED_SQUARE_6_SIZE 257
#define WAV_RES_BANDLIMITED_SAW_0 10
#define WAV_RES_BANDLIMITED_SAW_0_SIZE 257
#define WAV_RES_BANDLIMITED_SAW_1 11
#define WAV_RES_BANDLIMITED_SAW_1_SIZE 257
#define WAV_RES_BANDLIMITED_SAW_2 12
#define WAV_RES_BANDLIMITED_SAW_2_SIZE 257
#define WAV_RES_BANDLIMITED_SAW_3 13
#define WAV_RES_BANDLIMITED_SAW_3_SIZE 257
#define WAV_RES_BANDLIMITED_SAW_4 14
#define WAV_RES_BANDLIMITED_SAW_4_SIZE 257
#define WAV_RES_BANDLIMITED_SAW_5 15
#define WAV_RES_BANDLIMITED_SAW_5_SIZE 257
#define WAV_RES_BANDLIMITED_SAW_6 16
#define WAV_RES_BANDLIMITED_SAW_6_SIZE 257
#define WAV_RES_BANDLIMITED_TRIANGLE_0 17
#define WAV_RES_BANDLIMITED_TRIANGLE_0_SIZE 257
#define WAV_RES__BANDLIMITED_TRIANGLE_0 18
#define WAV_RES__BANDLIMITED_TRIANGLE_0_SIZE 257
#define WAV_RES___BANDLIMITED_TRIANGLE_0 19
#define WAV_RES___BANDLIMITED_TRIANGLE_0_SIZE 257
#define WAV_RES_BANDLIMITED_TRIANGLE_3 20
#define WAV_RES_BANDLIMITED_TRIANGLE_3_SIZE 257
#define WAV_RES_BANDLIMITED_TRIANGLE_4 21
#define WAV_RES_BANDLIMITED_TRIANGLE_4_SIZE 257
#define WAV_RES_BANDLIMITED_TRIANGLE_5 22
#define WAV_RES_BANDLIMITED_TRIANGLE_5_SIZE 257
#define WAV_RES_BANDLIMITED_TRIANGLE_6 23
#define WAV_RES_BANDLIMITED_TRIANGLE_6_SIZE 257
#define WAV_RES_VOWEL_DATA 24
#define WAV_RES_VOWEL_DATA_SIZE 63
#define WAV_RES_DISTORTION 25
#define WAV_RES_DISTORTION_SIZE 256
#define WAV_RES_ENV_EXPO 26
#define WAV_RES_ENV_EXPO_SIZE 257
#define WAV_RES_SSM2164_LINEARIZATION 27
#define WAV_RES_SSM2164_LINEARIZATION_SIZE 64
#define WAV_RES_WAVES 28
#define WAV_RES_WAVES_SIZE 16383
#define WAV_RES_WAVETABLES 29
#define WAV_RES_WAVETABLES_SIZE 324
#define WAV_RES_VERSION 30
#define WAV_RES_VERSION_SIZE 2
#define CHR_RES_SPECIAL_CHARACTERS 0
#define CHR_RES_SPECIAL_CHARACTERS_SIZE 56
typedef avrlib::ResourcesManager<
    ResourceId,
    avrlib::ResourcesTables<
        string_table,
        lookup_table_table> > ResourcesManager; 

}  // namespace shruthi

#endif  // SHRUTHI_RESOURCES_H_
