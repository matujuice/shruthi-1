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
#define STR_RES_SEQ 186  // seq
#define STR_RES__SQ1 187  // sq1
#define STR_RES__SQ2 188  // sq2
#define STR_RES_ARP 189  // arp
#define STR_RES_WHL 190  // whl
#define STR_RES_AFT 191  // aft
#define STR_RES_BND 192  // bnd
#define STR_RES_OFS 193  // ofs
#define STR_RES__CV1 194  // cv1
#define STR_RES__CV2 195  // cv2
#define STR_RES_CV3 196  // cv3
#define STR_RES_CV4 197  // cv4
#define STR_RES_CCA 198  // ccA
#define STR_RES_CCB 199  // ccB
#define STR_RES_BRE 200  // bre
#define STR_RES_PED 201  // ped
#define STR_RES__NOI 202  // noi
#define STR_RES_EN1 203  // en1
#define STR_RES_EN2 204  // en2
#define STR_RES_VEL 205  // vel
#define STR_RES_RND 206  // rnd
#define STR_RES_NOT 207  // not
#define STR_RES_GAT 208  // gat
#define STR_RES_AUD 209  // aud
#define STR_RES_OP1 210  // op1
#define STR_RES_OP2 211  // op2
#define STR_RES_E4 212  // =4
#define STR_RES_E8 213  // =8
#define STR_RES_E16 214  // =16
#define STR_RES_E32 215  // =32
#define STR_RES_LFO_1 216  // lfo 1
#define STR_RES_LFO_2 217  // lfo 2
#define STR_RES_STPSEQ 218  // stpseq
#define STR_RES_STPSQ1 219  // stpsq1
#define STR_RES_STPSQ2 220  // stpsq2
#define STR_RES__ARP 221  // arp
#define STR_RES_MWHEEL 222  // mwheel
#define STR_RES_AFTTCH 223  // afttch
#define STR_RES_BENDER 224  // bender
#define STR_RES_OFFSET 225  // offset
#define STR_RES__CV_1 226  // cv 1
#define STR_RES__CV_2 227  // cv 2
#define STR_RES_CV_3 228  // cv 3
#define STR_RES_CV_4 229  // cv 4
#define STR_RES_CC_A 230  // cc A
#define STR_RES_CC_B 231  // cc B
#define STR_RES_BREATH 232  // breath
#define STR_RES_PEDAL 233  // pedal
#define STR_RES__NOISE 234  // noise
#define STR_RES_ENV_1 235  // env 1
#define STR_RES_ENV_2 236  // env 2
#define STR_RES_VELO 237  // velo
#define STR_RES_RANDOM 238  // random
#define STR_RES_NOTE 239  // note
#define STR_RES_GATE 240  // gate
#define STR_RES_AUDIO 241  // audio
#define STR_RES_OP__1 242  // op. 1
#define STR_RES_OP__2 243  // op. 2
#define STR_RES_E_4 244  // = 4
#define STR_RES_E_8 245  // = 8
#define STR_RES_E_16 246  // = 16
#define STR_RES_E_32 247  // = 32
#define STR_RES_PATCH 248  // patch
#define STR_RES_START_FULL_MIDI 249  // start full midi
#define STR_RES_BACKUP_ 250  // backup?
#define STR_RES_BROWSE 251  // browse
#define STR_RES_SAVE 252  // save
#define STR_RES_COMPARE 253  // compare
#define STR_RES_ 254  // ----
#define STR_RES_V 255  // v
#define STR_RES_671____SN__4370 256  // -1    SN #4370
#define STR_RES_V1_03 257  // v1.03
#define STR_RES_SUM 258  // sum
#define STR_RES_SYNC 259  // sync
#define STR_RES_RING 260  // ring
#define STR_RES_XOR 261  // xor
#define STR_RES_FUZZ 262  // fuzz
#define STR_RES_GG4 263  // >>4
#define STR_RES_GG8 264  // >>8
#define STR_RES_FOLD 265  // fold
#define STR_RES_BITS 266  // bits
#define STR_RES_DUO 267  // duo
#define STR_RES_2STEPS 268  // 2steps
#define STR_RES_4STEPS 269  // 4steps
#define STR_RES_8STEPS 270  // 8steps
#define STR_RES_SEQMIX 271  // seqmix
#define STR_RES_FREE 272  // free
#define STR_RES_SLAVE 273  // slave
#define STR_RES_MASTER 274  // master
#define STR_RES_1SHOT 275  // 1shot
#define STR_RES_1_1 276  // 1/1
#define STR_RES__OFF 277  // off
#define STR_RES_THRU 278  // thru
#define STR_RES__SEQ 279  // seq
#define STR_RES_CTRL 280  // ctrl
#define STR_RES_FULL 281  // full
#define STR_RES_1TV 282  // 1~|
#define STR_RES_2T1 283  // 2~1
#define STR_RES_3T2 284  // 3~2
#define STR_RES_4T3 285  // 4~3
#define STR_RES_5T4 286  // 5~4
#define STR_RES_6T5 287  // 6~5
#define STR_RES_7T6 288  // 7~6
#define STR_RES_8T7 289  // 8~7
#define STR_RES_TRI 290  // tri
#define STR_RES_SQR 291  // sqr
#define STR_RES_S_H 292  // s&h
#define STR_RES_RAMP 293  // ramp
#define STR_RES_STEP 294  // step
#define STR_RES_3 295  // 
#define STR_RES_4 296  // 
#define STR_RES_34 297  // 
#define STR_RES__ 298  // ?
#define STR_RES_PLAYED 299  // played
#define STR_RES_OFF 300  // off
#define STR_RES_ON 301  // on
#define STR_RES_SQ1 302  // sq1
#define STR_RES__TR1 303  // tr1
#define STR_RES_PL1 304  // pl1
#define STR_RES_SQ2 305  // sq2
#define STR_RES__TR2 306  // tr2
#define STR_RES_PL2 307  // pl2
#define STR_RES_CLICK 308  // click
#define STR_RES_GLITCH 309  // glitch
#define STR_RES_BLOW 310  // blow
#define STR_RES_METAL 311  // metal
#define STR_RES_POP 312  // pop
#define STR_RES_NONE 313  // none
#define STR_RES_SAW 314  // saw
#define STR_RES_SQUARE 315  // square
#define STR_RES_TRIANG 316  // triang
#define STR_RES_ZSAW 317  // zsaw
#define STR_RES_ZRESO 318  // zreso
#define STR_RES_ZTRI 319  // ztri
#define STR_RES_ZPULSE 320  // zpulse
#define STR_RES_ZSYNC 321  // zsync
#define STR_RES_PAD 322  // pad
#define STR_RES_FM 323  // fm
#define STR_RES_WAVES 324  // waves
#define STR_RES_TAMPUR 325  // tampur
#define STR_RES_DIGITL 326  // digitl
#define STR_RES_METALL 327  // metall
#define STR_RES_BOWED 328  // bowed
#define STR_RES_SLAP 329  // slap
#define STR_RES_ORGAN 330  // organ
#define STR_RES_MALE 331  // male
#define STR_RES_USER 332  // user
#define STR_RES_8BITS 333  // 8bits
#define STR_RES_CRUSH 334  // crush
#define STR_RES_PWM 335  // pwm
#define STR_RES___NOISE 336  // noise
#define STR_RES_VOWEL 337  // vowel
#define STR_RES_BELISH 338  // belish
#define STR_RES_POLATD 339  // polatd
#define STR_RES_CELLO 340  // cello
#define STR_RES_CLPSWP 341  // clpswp
#define STR_RES_FEMALE 342  // female
#define STR_RES_FMTVOC 343  // fmtvoc
#define STR_RES_FRMNT2 344  // frmnt2
#define STR_RES_RES3HP 345  // res3hp
#define STR_RES_ELECTP 346  // electp
#define STR_RES_VIBES 347  // vibes
#define STR_RES__LPF 348  // lpf
#define STR_RES_SSM 349  // ssm
#define STR_RES_SVF 350  // svf
#define STR_RES_DSP 351  // dsp
#define STR_RES_PVK 352  // pvk
#define STR_RES_4PM 353  // 4pm
#define STR_RES_DLY 354  // dly
#define STR_RES_SP 355  // sp
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
