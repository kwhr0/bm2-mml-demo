#include <stdio.h>
#include <ctype.h>
#include <math.h>

#if POLY
#define CLK				(3000 * 1000)
#define CLK_PER_LOOP	180
#define DESC_N			4
#define ENV				(CLK / 300000)
#else
#define CLK				(750 * 1000)
#define CLK_PER_LOOP	8
#define DESC_N			1
#endif
#define DEFAULT_BPM		120
#define OCTAVE			12
#define TBASE			(4 * 60 * CLK / CLK_PER_LOOP)

typedef signed char s8;
typedef signed short s16;
typedef signed long s32;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

typedef struct Desc {
	u8 *mml;
	u8 len, base;
	u16 wait, reg, v;
} Desc;

static Desc desc[DESC_N];
static u32 tempo;
static u16 tonetable[128];

static void mmlInit() {
	u8 i;
#if POLY
	printf("\t.data\n\t.export\t_tonetable\n_tonetable:\n");
	for (i = 0; i < 128; i++) {
		tonetable[i] = 65536 * 440 * pow(2, (i - 69) / 12.) / (CLK / (2 * CLK_PER_LOOP));
		printf("\tdefw\t%d\n", tonetable[i]);
	}
#else
	for (i = 0; i < 128; i++) tonetable[i] = CLK / CLK_PER_LOOP / (440 * pow(2, (i - 69) / 12.));
#endif
	printf("\t.export\t_data\n_data:\n");
	u16 reg = 0x8000;
	Desc *p;
	for (p = desc; p < desc + DESC_N; p++, reg += 0x1000) {
		p->mml = 0;
		p->base = 5 * OCTAVE;
		p->len = 4;
		p->wait = 0;
		p->reg = reg;
		p->v = 0x700;
	}
	tempo = TBASE / DEFAULT_BPM;
}

static void mmlExit() {
	printf("\tdefw\t0\n\tdefw\t0\n");
}

static u8 *decimal(u8 *p, u8 *r) {
	if (isdigit(*p)) {
		*r = 0;
		do {
			*r *= 10;
			*r += *p++ - '0';
		} while (isdigit(*p));
	}
	return p;
}

static u32 waitsub(Desc *d) {
	u32 t, t0;
	u8 l = d->len;
	d->mml = decimal(d->mml + 1, &l);
	if (!l) {
		d->mml--;
		return 0;
	}
	t = tempo / l;
	t0 = t;
	while (*d->mml++ == '.') t += t0 >>= 1;
	d->mml -= 2;
	return t ? t : 1;
}

static void mmlUpdate(Desc *d) {
	static const u8 cnv[] = { 9, 11, 0, 2, 4, 5, 7 };
	s8 oneshot = 0;
	while (d->mml && !d->wait) {
		u8 index, i;
		switch (*d->mml) {
			default:
				if (*d->mml < 'a') break;
					if (*d->mml > 'g') break;
					index = d->base + cnv[*d->mml++ - 'a'] + oneshot;
					oneshot = 0;
					if (*d->mml == '+' || *d->mml == '#') index++;
					else if (*d->mml == '-') index--;
					else d->mml--;
					if (index < 128) {
#if POLY
						printf("\tdefw\t$%04x\n", index << 1 | d->reg | d->v);
						d->wait = waitsub(d);
#else
						u16 t = tonetable[index], w = waitsub(d) / t;
						printf("\tdefw\t%d\n\tdefw\t%d\n", t, w ? w : 1);
#endif
					}
				break;
			case 'm':
				d->mml = decimal(d->mml + 1, &i) - 1;
#if POLY
				i *= ENV;
				printf("\tdefw\t$%04x\n", d->reg | 0x4000 | i);
#endif
				break;
			case 'r':
#if POLY
				printf("\tdefw\t$%04x\n", d->reg);
				d->wait = waitsub(d);
#else
				printf("\tdefw\t0\n\tdefw\t%lu\n", waitsub(d) >> 7);
#endif
				break;
			case 'v':
				d->mml = decimal(d->mml + 1, &i) - 1;
				if (i > 7) i = 7;
				d->v = i << 8;
				break;
			case 'o':
				if (isdigit(*++d->mml)) d->base = OCTAVE * (*d->mml - '0') + OCTAVE;
				break;
			case '>':
				if (d->base < 9 * OCTAVE) d->base += OCTAVE;
				break;
			case '<':
				if (d->base >= OCTAVE) d->base -= OCTAVE;
				break;
			case '~':
				oneshot = OCTAVE;
				break;
			case '_':
				oneshot = -OCTAVE;
				break;
			case 'l':
				d->mml = decimal(d->mml + 1, &d->len) - 1;
				break;
			case 't':
				i = 1;
				d->mml = decimal(d->mml + 1, &i) - 1;
				tempo = TBASE / i;
				break;
			case 0:
				d->mml = 0;
				continue;
		}
		d->mml++;
	}
}

#if POLY
static void mmlLoop() {
	Desc *p;
	do {
		for (p = desc; p < desc + DESC_N; p++) mmlUpdate(p);
		u16 min = 0xffff;
		for (p = desc; p < desc + DESC_N; p++) 
			if (p->wait && min > p->wait) min = p->wait;
		if (min == 0xffff) break;
		if (min > 0x7fff) min = 0x7fff;
		printf("\tdefw\t%d\n", min);
		for (p = desc; p < desc + DESC_N; p++)
			if (p->wait && !(p->wait -= min) && !*p->mml)
				printf("\tdefw\t$%04x\n", p->reg & 0xf000);
	} while (1);
}
#endif

static void play(const char *mml) {
	desc[0].mml = (u8 *)mml;
#if POLY
	mmlLoop();
#else
	mmlUpdate(desc);
#endif
}

#if POLY

static void play3(const char *mml1, const char *mml2, const char *mml3) {
	desc[0].mml = (u8 *)mml1;
	desc[1].mml = (u8 *)mml2;
	desc[2].mml = (u8 *)mml3;
	mmlLoop();
}

void playnoise(const char *mml_noise) {
	desc[3].mml = (u8 *)mml_noise;
	mmlLoop();
}

#else

#define play3(a, b, c)
#define playnoise(a)

#endif

int main() {
	u8 i;
	mmlInit();
	play3("t120l128m0o1gg-fee-dd-c>gg-fee-dd-c>gg-fee-dd-c>gg-fee-dd-cr2", "m0l128o1a-gg-fee-dd->a-gg-fee-dd->a-gg-fee-dd->a-gg-fee-dd-", "");
	play3("t120m0o5l16f4rcfab+arfgrggrddrgfref4rcfab+arfg-rg-g-rf4.", "m0o4l16a4rfa>cfcr<ab-rb-b-rb-b-rb-b-rb-a4rfa>cfcr<ab-rb-b-ra4.", "m2o3l16ffffr8ffffr8ffffr8ffffr8ffffr8ffffr8ffffr16f");
	play("t160l16o5cb+b>cec<bb+cb+a+>cec<a+b+cb+a>cec<ab+cb+a->cec<a-b+");
	for (i = 0; i < 3; i++) play("t255m0l64o3g+>>>>f+<<<<a>>>>c+<<<<a+>>>g+<<<b>>>d+r16");
	for (i = 0; i < 2; i++) play3("t150m0o7l32cc+dd+", "m0o7l32f+eg+f+", "m0o7l32agba");
	for (i = 0; i < 3; i++) play3("t120m3o6c16", "m3o6c+16", "");
	for (i = 0; i < 3; i++) play("t255l64o5cc+dd+ee-dd-c<bb-aa-gg-fee-dd-c<bb-aa-gg-fee-dd-c<bb-aa-gg-fee-dd-c");
	play("t120l128o5cc+dd+eff+gcc+dd+eff+gcc+dd+eff+gg+aa+");
	play("t255o7l128cc+d+f+a+f+d+d64.c+64.c64.l16<c<c<c<c<cb+cb+cb+cb+cb+cb+cb+c");
	for (i = 0; i < 3; i++) play("t120l32o2cc+cdcd+cdcd+cdcc+");
#if POLY
	play("t255l64o5cc+dd+ee-dd-c<bb-aa-gg-fee-dd-c<bb-aa-gg-fee-dd-c<bb-aa-gg-fee-dd-c");
#endif
	playnoise("t60m9o5c8");
	play3("t120m0l16o5ec<grg>cer","m0l16o5gec<g>ceg","m0l16o5b+gecegb+");
	play3("t100m3l16o5l32ccl16ccccccdefffffagfgggggb+b-aaaaaab-b+", "m3l16ro3ccrrccrrffrrffrrccrrccrrffrrff", "m5l16ro3b-b-rrb-b-rraarraarrb-b-rrb-b-rraarraa");
	play3("t80m0l32o3fa>>ee4.dc+16.ed4.<<fa>>e-e-4.de-16.fg16.fe-16.dc16.d", "m9l8o5ffffffffffffffff", "m4l8o5aaaaaaaaaaaaaaaa");

	mmlExit();
	return 0;
}
