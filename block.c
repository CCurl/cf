// block.c - a simple block system

#include "cf.h"

#define VALID(blk)    btwi(blk,0,NUM_BLOCKS-1)

static byte blocks[BLOCK_SZ*NUM_BLOCKS];
static byte dirty[NUM_BLOCKS];

void blockDirty(int blk, byte val) {
	if (VALID(blk)) { dirty[blk]=val; }
}

cell blockData(int blk) {
	if (VALID(blk)) {
		return (cell)&blocks[blk*BLOCK_SZ];
	}
	return 0;
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

void flushBlock(int blk, int force) {
	char fn[32];
	byte *data = (byte*)blockData(blk);
	if (data == NULL) { return; }
	if ((force==0) && (dirty[blk]==0)) { return; }
	sprintf(fn, " block-%03d.cf", blk);
	cell fp = fileOpen(fn, " wb");
	data[BLOCK_SZ-1] = 0;
	if (fp) {
		printf("-writeBlock(%d)-",blk);
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
