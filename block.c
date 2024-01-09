// block.c - a simple block system

#include "cf.h"

#define CACHE_SZ      16

#define BLOCK_FREE  0
#define BLOCK_READ  1
#define BLOCK_DIRTY 2  

#define BLKNUM(bn)         (bn)
#define BLK(bn)            (&cache[bn%CACHE_SZ])
#define BLKADDR(bn)        &blocks[BLKNUM(bn)][0]
#define SAMEBLK(blk, bn)   (blk->num==bn)
#define ISDIRTY(blk)       (blk->stat==BLOCK_DIRTY)

typedef struct { int num; int stat; byte data[BLOCK_SZ]; } cache_t;

static cache_t cache[CACHE_SZ];
static int init = 0;

void blockClear(cell_t blockNum, int force) {
	if (blockNum < 0) { return; }
	cache_t *blk=BLK(blockNum);
	if (SAMEBLK(blk,blockNum) || force) {
		blk->num = -1;
		blk->stat = BLOCK_FREE;
		fill(blk->data, 0, BLOCK_SZ);
	}
}

void blockIsDirty(cell_t blockNum) {
	if (blockNum < 0) { return; }
	cache_t *blk=BLK(blockNum);
	if (SAMEBLK(blk, blockNum)) { blk->stat = BLOCK_DIRTY; }
}

static void blockInit(int force) {
	if (!init || force) {
		for (int i=0; i<CACHE_SZ; i++) { blockClear(i, 1); }
		init = 1;
	}
}

static void readWriteBlock(cache_t *blk, int isWrite, int isText) {
	char fn[32];
	sprintf(fn, "block-%03d.cf", blk->num);
	FILE* fp = fopen(fn, isWrite ? "wb" : "rb");
	if (fp) {
		if (isWrite) {
			if (isText) { fputs(blk->data, fp); }
			else { fwrite(blk->data, 1, BLOCK_SZ, fp); }
		}
		else { fread(blk->data, 1, BLOCK_SZ, fp); }
		fclose(fp);
	}
}

void blockFlush(cell_t blockNum, int force, int isText) {
	blockInit(0);
	if (blockNum < 0) { return; }
	cache_t *blk = BLK(blockNum);
	if ((!SAMEBLK(blk,blockNum)) && (!force)) { return; }
	if ((!ISDIRTY(blk)) && (!force)) { return; }
	readWriteBlock(blk, 1, isText);
	blk->stat = BLOCK_READ;
}

byte *blockRead(cell_t blockNum) {
	blockInit(0);
	if (blockNum < 0) { return NULL; }
	cache_t *blk = BLK(blockNum);
	if (!SAMEBLK(blk,blockNum)) {
		blockFlush(blk->num, 0, 0);
		blk->stat = BLOCK_FREE;
	}
	if (blk->stat == BLOCK_FREE) {
		blockClear(blockNum, 1);
		blk->num = (int)blockNum;
		readWriteBlock(blk, 0, 0);
		blk->stat = BLOCK_READ;
	}
	return blk->data;
}

void blockFlushAll() {
	blockInit(0);
	for (int i = 0; i < CACHE_SZ; i++) { blockFlush(i, 0, 0); }
}
