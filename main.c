#include <assert.h>
#include <elf.h>
#include <stdio.h>

#define MASK_7 0x00FF000000000000
#define VALUE(x) ((x).arch == ELFCLASS32 ? (uint32_t)((x).data) : (x).data)

typedef struct {
  int arch;
  uint64_t data;
} Word;

unsigned char reverse(unsigned char b) {
  b = (b & 0xF0) >> 4 | (b & 0x0F);
  b = (b & 0xCC) >> 2 | (b & 0x33);
  b = (b & 0xAA) >> 1 | (b & 0x55);
  return b;
}

Word read_word(FILE *f, int arch, int endianess) {
  Word ret;
  char buf[8];
  fread(buf, arch == ELFCLASS32 ? 4 : 8, 1, f);
  ret.arch = arch;

  if (endianess == ELFDATA2MSB) {
    char temp[8];
    if (arch == ELFCLASS32) {
      temp[3] = reverse(buf[0]);
      temp[2] = reverse(buf[1]);
      temp[1] = reverse(buf[2]);
      temp[0] = reverse(buf[3]);
    } else {
      temp[7] = reverse(buf[0]);
      temp[6] = reverse(buf[1]);
      temp[5] = reverse(buf[2]);
      temp[4] = reverse(buf[3]);
      temp[3] = reverse(buf[4]);
      temp[2] = reverse(buf[5]);
      temp[1] = reverse(buf[6]);
      temp[0] = reverse(buf[7]);
    }
    ret.data = (uint64_t)temp;
  } else {
    ret.data = arch == ELFCLASS32 ? *(uint32_t *)&buf : *(uint64_t *)&buf;
  }
  return ret;
}

int main(int argc, char **argv) {
  const char *path = argv[1];

  FILE *f = fopen(path, "r");
  assert(f);
  char buf[EI_NIDENT];
  fread(buf, EI_NIDENT, 1, f);
  assert(ELFMAG0 == buf[0] && "incorrect magic");
  assert(ELFMAG1 == buf[1] && "incorrect magic");
  assert(ELFMAG2 == buf[2] && "incorrect magic");
  assert(ELFMAG3 == buf[3] && "incorrect magic");

  int arch = buf[EI_CLASS];
  assert(arch != ELFCLASSNONE);
  if (arch == ELFCLASS32)
    arch = 4;
  else
    arch = 8;

  int endianess = buf[EI_DATA];
  assert(endianess != ELFDATANONE);

  // skip to e_shoff
  fseek(f, 2 + 2 + 4 + arch + arch, SEEK_CUR);
  char shoff[arch];
  fread(shoff, arch, 1, f);
}
