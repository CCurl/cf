// block.c - a simple block system

#include "cf.h"

#define CACHE_SZ      16

enum { BLOCK_FREE=0, BLOCK_READ, BLOCK_DIRTY };
enum { BLOCK_TEXT=0, BLOCK_BINARY };

typedef struct {
	int num; int stat; int type; char data[BLOCK_SZ];
} cache_t;

static cache_t cache[CACHE_SZ];
static int init = 0;

static cache_t *findFree() {
	for (int i=0; i < CACHE_SZ; i++) {
		cache_t *blk = &cache[i];
		if (blk->num < 0) { return blk; }
		if (blk->stat == BLOCK_FREE) { return blk; }
	}
	return NULL;
}

static int findBlock(cell_t blkNum) {
	for (int i=0; i < CACHE_SZ; i++) {
		if (cache[i].num == blkNum) { return i; }
	}
	return -1;
}

static cache_t *inCache(cell_t blkNum) {
	int i = findBlock(blkNum);
	return (0 <= i) ? &cache[i] : NULL;
}

void blockClear(cell_t blkNum, int discard) {
	cache_t *blk = inCache(blkNum);
	if (!blk) { return; }
	if (!discard) { blockWrite(blkNum, 0); }
	blk->num = -1;
	blk->stat = BLOCK_FREE;
	blk->type = BLOCK_BINARY;
	fill(blk->data, 0, BLOCK_SZ);
}

void blockIsDirty(cell_t blockNum) {
	cache_t *blk = inCache(blockNum);
	if (blk) { blk->stat = BLOCK_DIRTY; }
}

void blockIsText(cell_t blockNum) {
	cache_t *blk = inCache(blockNum);
	if (blk) { blk->type = BLOCK_TEXT; }
}

void blockIsBinary(cell_t blockNum) {
	cache_t *blk = inCache(blockNum);
	if (blk) { blk->type = BLOCK_BINARY; }
}

static void blockInit(int force) {
	if (!init || force) {
		for (int i=0; i<CACHE_SZ; i++) { cache[i].num=12345; }
		for (int i=0; i<CACHE_SZ; i++) { blockClear(12345, 1); }
		init = 1;
	}
}

static void readWriteBlock(cache_t *blk, int isWrite) {
	char fn[32], *md = NULL;
	sprintf(fn, "block-%03d.cf", blk->num);
	if (isWrite) { md = (blk->type == BLOCK_BINARY) ? "wb" : "wt"; }
	else { md = (blk->type == BLOCK_BINARY) ? "rb" : "rt"; }
	FILE* fp = fopen(fn, md);
	if (fp) {
		if (isWrite) {
			if (blk->type==BLOCK_TEXT) { fputs(blk->data, fp); }
			else { fwrite(blk->data, 1, BLOCK_SZ, fp); }
		}
		else { fread(blk->data, 1, BLOCK_SZ, fp); }
		fclose(fp);
		blk->stat = BLOCK_READ;
	}
}

char *blockRead(cell_t blockNum) {
	blockInit(0);
	if (blockNum < 0) { return NULL; }
	cache_t *blk = inCache(blockNum);
	if (!blk) { blk = findFree(); }
	if (!blk) { return NULL; }
	if (blk->stat == BLOCK_FREE) {
		blk->num = (int)blockNum;
		readWriteBlock(blk, 0);
	}
	return blk->data;
}

void blockWrite(cell_t blockNum, int force) {
	blockInit(0);
	if (blockNum < 0) { return; }
	cache_t *blk = inCache(blockNum);
	if (!blk) { return; }
	if ((blk->stat == BLOCK_DIRTY) || (force)) { return; }
	readWriteBlock(blk, 1);
}

void blockFlushAll() {
	if (init) {
		for (int i = 0; i < CACHE_SZ; i++) {
			blockWrite(cache[i].num, 0);
		}
	}
	blockInit(1);
}
