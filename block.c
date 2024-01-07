// block.c - a simple block system

#include "cf.h"

#define NUM_BLOCKS  1000

#define BLOCK_FREE  0
#define BLOCK_READ  1
#define BLOCK_DIRTY 2  

#define BLKNUM(bn)    (bn)
#define BLKADDR(bn)   &blocks[BLKNUM(bn)][0]

static byte blocks[NUM_BLOCKS][BLOCK_SZ];
static byte stat[NUM_BLOCKS];
static int init = 0;

void blockClear(cell_t blockNum) {
	if (stat[BLKNUM(blockNum)] != BLOCK_FREE) {
		fill(BLKADDR(blockNum), 0, BLOCK_SZ);
		stat[BLKNUM(blockNum)] = BLOCK_FREE;
	}
}

void blockIsDirty(cell_t blockNum) {
	stat[BLKNUM(blockNum)] = BLOCK_DIRTY;
}

static void blockInit() {
	if (!init) {
		for (int i=0; i<NUM_BLOCKS; i++) { blockClear(i); }
		init = 1;
	}
}

void blockFlush(cell_t blockNum) {
	if (stat[blockNum] != BLOCK_DIRTY) { return; }
	char fn[32];
	byte *a = BLKADDR(blockNum);
	blockInit();
	sprintf(fn, "block-%03ld.cf", (long)blockNum);
	FILE* fp = fopen(fn, "wb");
	if (fp) {
		fwrite(a, 1, BLOCK_SZ, fp);
		fclose(fp);
	}
	stat[BLKNUM(blockNum)] = BLOCK_READ;
}

byte *blockRead(cell_t blockNum) {
	char fn[32];
	byte *a = BLKADDR(blockNum);
	blockInit();
	if (stat[BLKNUM(blockNum)]==BLOCK_DIRTY) {
		blockFlush(blockNum);
	}
	blockClear(blockNum);
	sprintf(fn, "block-%03ld.cf", (long)blockNum);
	FILE* fp = fopen(fn, "rb");
	if (fp) {
		fread(a, 1, BLOCK_SZ, fp);
		fclose(fp);
	}
	stat[BLKNUM(blockNum)] = BLOCK_READ;
	return a;
}

void blockFlushAll() {
	blockInit();
	for (int i = 0; i < NUM_BLOCKS; i++) { blockFlush(i); }
}
