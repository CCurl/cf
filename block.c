// block.c - a simple block system

#include "cf.h"

#define VALID(blk)    btwi(blk,0,NUM_BLOCKS-1)

static byte blocks[BLOCK_SZ*NUM_BLOCKS];
static byte dirty[NUM_BLOCKS];

cell blockData(int blk) {
	return VALID(blk) ? (cell)&blocks[blk*BLOCK_SZ] : 0;
}

void blockDirty(int blk, int val) {
	if (VALID(blk)) { dirty[blk]=val; }
}

int blockDirtyQ(int blk) {
	return VALID(blk) ? dirty[blk] : 0;
}

static void readBlock(int blk) {
	char fn[32];
	byte *data = (byte*)blockData(blk);
	if (data == NULL) { return; }
	fill(data, BLOCK_SZ, 0);
	sprintf(fn, " block-%03d.cf", blk);
	cell fp = fileOpen(fn, " rb");
	if (fp) {
		fileRead(data, BLOCK_SZ, fp);
		fileClose(fp);
		// printf("-readblock:%d-",blk);
	}
	blockDirty(blk, 0);
	data[BLOCK_SZ-1] = 0;
}

static void flushBlock(int blk, int force) {
	char fn[32];
	byte *data = (byte*)blockData(blk);
	if (data == NULL) { return; }
	if ((force==0) && (dirty[blk]==0)) { return; }
	sprintf(fn, " block-%03d.cf", blk);
	cell fp = fileOpen(fn, " wb");
	data[BLOCK_SZ-1] = 0;
	if (fp) {
		printf("-flushBlock(%d)-",blk);
		fileWrite(data, BLOCK_SZ, fp);
		fileClose(fp);
	}
	blockDirty(blk, 0);
}

void flushBlocks() {
	for (int i = 0; i < NUM_BLOCKS; i++) { flushBlock(i, 0); }
}

void initBlocks() {
	for (int i = 0; i < NUM_BLOCKS; i++) { readBlock(i); }
}
