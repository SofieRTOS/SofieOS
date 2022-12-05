#define BENCH_ECDH

#ifdef BENCH_NUMBER_POWER
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sofie.h>

extern int tick;

int real_main(void)
{
	sofie_set_protected_stack();
	int num = sofie_rand();
	int term = 100000000;
	int begin = tick;
	int i, res = 1;
	for (i = 0; i < term; i++) {
		res = res * num;
	}
	uprintf("%d, ", tick - begin);
	uprintf("END\r\n");
	for(;;);
	sofie_restore_protected_stack();
	return 0;
}

#endif

#ifdef BENCH_FIB_LOOP

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sofie.h>
extern int tick;

int real_main(void)
{
	sofie_set_protected_stack();
	int begin = tick;
	int a = 1, b = 1, c = 2;
	int i, n = 1000000;
	for (i = 0; i < n - 3; i++) {
		a = b;
		b = c;
		c = a + b;
	}
	int end = tick;
	uprintf("%d\r\n", end - begin);
	for(;;);
	sofie_restore_protected_stack();
	return 0;
}

#endif

#ifdef BENCH_FIB_RECURSIVE

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sofie.h>
extern int tick;
extern int call_cnt;

int fib(int n) {
	sofie_set_protected_stack();
	int ret = 1;
	if (n != 0 && n != 1) {
		ret = fib(n-1) + fib(n-2);
	}
	sofie_restore_protected_stack();
	return ret;
}

int real_main(void)
{
	sofie_set_protected_stack();
	int i, n = 10;
	int begin = tick;
	int a = 1, b = 1, c = 2;
	int f = fib(n);
	int end = tick;
	uprintf("%d, %d\r\n", end - begin, call_cnt);
	for(;;);
	sofie_restore_protected_stack();
	return 0;
}

#endif

#ifdef BENCH_ECDH
#include <assert.h>
#include <stdint.h>
#include <sofie.h>
#include "bignum.h"
#include "nist256p1.h"
#include "ecdsa-generic.h"

extern int tick;
extern int call_cnt;

int real_main() {
	sofie_set_protected_stack();
	int begin = tick;
	int duration;
  uint8_t pri_a[32], pub_a[64], pri_b[32], pub_b[64], shared1[64], shared2[64];
  curve_point pub;
  bignum256 s;

  ecdsa_generate_keypair(&nist256p1, pri_a, pub_a);
  ecdsa_generate_keypair(&nist256p1, pri_b, pub_b);
  // pri_a * pub_b
  ecdsa_read_pubkey(&nist256p1, pub_b, &pub);
  bn_read_be(pri_a, &s);
  point_multiply(&nist256p1, &s, &pub, &pub);
  bn_write_be(&pub.x, shared1);
  bn_write_be(&pub.y, shared1 + 32);

  // pri_b * pub_a
  ecdsa_read_pubkey(&nist256p1, pub_a, &pub);
  bn_read_be(pri_b, &s);
  point_multiply(&nist256p1, &s, &pub, &pub);
  bn_write_be(&pub.x, shared2);
  bn_write_be(&pub.y, shared2 + 32);

  // assert they are equal
  for (int i = 0; i != 64; ++i) {
	  if(shared1[i] != shared2[i]) {
	     printf("NOT EQUAL!\r\n");
	  }
  }

  duration = tick - begin;
  uprintf("time = %d, call = %d\r\n", duration, call_cnt);
  for(;;);
  sofie_restore_protected_stack();
  return 0;
}

#endif

#ifdef BENCH_QSORT

#include <sofie.h>
extern int tick;
extern int call_cnt;

void sort(short arr[], int l, int r) {
	sofie_set_protected_stack();
	int x = sofie_rand() % (r - l + 1) + l;
	int t = arr[x];
	int i = l, p = l;
	arr[x] = arr[r];
	arr[r] = t;
	for (i = l; i <= r - 1; i++) {
		if (arr[i] < arr[r]) {
			t = arr[i];
			arr[i] = arr[p];
			arr[p] = t;
			p++;
		}
	}

	t = arr[r];
	arr[r] = arr[p];
	arr[p] = t;

	if (l < p) sort(arr, l, p - 1);
	if (p < r) sort(arr, p + 1, r);
	sofie_restore_protected_stack();
}

short arr[1000];

int real_main() {
	sofie_set_protected_stack();
	int i, term, sum = 0;
	for (i = 0; i < 1000; i++) arr[i] = sofie_rand();
	int begin = tick;
	sort(arr, 0, 1000-1);
	int end = tick;

//	for (i = 0; i < 3000; i++) if (arr[i] != i) uprintf("NOT EQUAL!\r\n");
	uprintf("%d, %d\r\n", end - begin, call_cnt);
	for(;;);
	sofie_restore_protected_stack();
	return 0;
}
#endif

#ifdef BENCH_LFS
#include <stdio.h>

#include "lfs.h"

#define PAGE_SIZE 256
#define CACHE_SIZE 256
#define READ_SIZE 1
#define WRITE_SIZE 16
#define LOOKAHEAD_SIZE 32

static uint8_t fs[2048];
static uint8_t read_buffer[CACHE_SIZE];
static uint8_t prog_buffer[CACHE_SIZE];
static uint8_t lookahead_buffer[LOOKAHEAD_SIZE];

int block_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size) {
  sofie_set_protected_stack();
  memcpy(buffer, fs + block * PAGE_SIZE + off, size);
  sofie_restore_protected_stack();
  return 0;
}

int block_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) {
	sofie_set_protected_stack();
  memcpy(fs + block * PAGE_SIZE + off, buffer, size);
  sofie_restore_protected_stack();
  return 0;
}

int block_erase(const struct lfs_config *c, lfs_block_t block) {
  return 0;
}

int block_sync(const struct lfs_config *c) {
  return 0;
}

static const struct lfs_config config = {
    .read = block_read,
    .prog = block_prog,
    .erase = block_erase,
    .sync = block_sync,
    .read_size = READ_SIZE,
    .prog_size = WRITE_SIZE,
    .block_size = PAGE_SIZE,
    .block_count = 256,
    .block_cycles = 10000,
    .cache_size = CACHE_SIZE,
    .lookahead_size = LOOKAHEAD_SIZE,
    .read_buffer = read_buffer,
    .prog_buffer = prog_buffer,
    .lookahead_buffer = lookahead_buffer,
};

static lfs_t lfs;
extern int tick;
extern int call_cnt;

int real_main() {
  sofie_set_protected_stack();
  int start = tick;
  assert(lfs_format(&lfs, &config) == 0);
  assert(lfs_mount(&lfs, &config) == 0);

  lfs_file_t f;
  assert(lfs_file_open(&lfs, &f, "test", LFS_O_CREAT | LFS_O_WRONLY) == 0);
  assert(lfs_file_write(&lfs, &f, "test data", 10) == 10);
  assert(lfs_file_close(&lfs, &f) == 0);

  uint8_t buffer[100];
  assert(lfs_file_open(&lfs, &f, "test", LFS_O_RDONLY) == 0);
  assert(lfs_file_read(&lfs, &f, buffer, 10) == 10);
  int end = tick;
  uprintf("%s, duration: %d, call: %d\r\n", buffer, end - start, call_cnt);
  for(;;);
  sofie_restore_protected_stack();
  return 0;
}
#endif
