// block.c - a simple block system

#include "cf.h"

#define CACHE_SZ      16

#define BLKNUM(bn)         (bn)
#define BLK(bn)            (&cache[bn%CACHE_SZ])
#define BLKADDR(bn)        &blocks[BLKNUM(bn)][0]
#define SAMEBLK(blk, bn)   (blk->num==bn)
#define ISDIRTY(blk)       (blk->stat==BLOCK_DIRTY)

enum { BLOCK_FREE=0, BLOCK_READ, BLOCK_DIRTY };
enum { BLOCK_TEXT=0, BLOCK_BINARY };

typedef struct {
	int num; int stat; int type; char data[BLOCK_SZ];
} cache_t;

static cache_t cache[CACHE_SZ];
static int init = 0;

void blockClear(cell_t blockNum, int force) {
	if (blockNum < 0) { return; }
	cache_t *blk=BLK(blockNum);
	if (SAMEBLK(blk,blockNum) || force) {
		blk->num = -1;
		blk->stat = BLOCK_FREE;
		blk->type = BLOCK_BINARY;
		fill(blk->data, 0, BLOCK_SZ);
	}
}

void blockIsDirty(cell_t blockNum) {
	if (blockNum < 0) { return; }
	cache_t *blk=BLK(blockNum);
	if (SAMEBLK(blk, blockNum)) { blk->stat = BLOCK_DIRTY; }
}

void blockIsText(cell_t blockNum) {
	if (blockNum < 0) { return; }
	cache_t *blk=BLK(blockNum);
	if (SAMEBLK(blk, blockNum)) { blk->type = BLOCK_TEXT; }
}

void blockIsBinary(cell_t blockNum) {
	if (blockNum < 0) { return; }
	cache_t *blk=BLK(blockNum);
	if (SAMEBLK(blk, blockNum)) { blk->stat = BLOCK_BINARY; }
}

static void blockInit(int force) {
	if (!init || force) {
		for (int i=0; i<CACHE_SZ; i++) { blockClear(i, 1); }
		init = 1;
	}
}

static void readWriteBlock(cache_t *blk, int isWrite) {
	char fn[32];
	sprintf(fn, "block-%03d.cf", blk->num);
	FILE* fp = fopen(fn, isWrite ? "wb" : "rb");
	if (fp) {
		if (isWrite) {
			if (blk->type==BLOCK_TEXT) { fputs(blk->data, fp); }
			else { fwrite(blk->data, 1, BLOCK_SZ, fp); }
		}
		else { fread(blk->data, 1, BLOCK_SZ, fp); }
		fclose(fp);
	}
}

void blockFlush(cell_t blockNum, int force) {
	blockInit(0);
	if (blockNum < 0) { return; }
	cache_t *blk = BLK(blockNum);
	if ((!SAMEBLK(blk,blockNum)) && (!force)) { return; }
	if ((!ISDIRTY(blk)) && (!force)) { return; }
	readWriteBlock(blk, 1);
	blk->stat = BLOCK_READ;
}

char *blockRead(cell_t blockNum) {
	blockInit(0);
	if (blockNum < 0) { return NULL; }
	cache_t *blk = BLK(blockNum);
	if (!SAMEBLK(blk,blockNum)) {
		blockFlush(blk->num, 0);
		blk->stat = BLOCK_FREE;
	}
	if (blk->stat == BLOCK_FREE) {
		blockClear(blockNum, 1);
		blk->num = (int)blockNum;
		readWriteBlock(blk, 0);
		blk->stat = BLOCK_READ;
	}
	return blk->data;
}

void blockFlushAll() {
	blockInit(0);
	for (int i = 0; i < CACHE_SZ; i++) { blockFlush(i, 0); }
}
