#ifndef __BLOCK_H__

#define BLOCK_SZ    2000

extern void blockIsDirty(cell_t blockNum);
extern void blockClear(cell_t blockNum);
extern void blockFlush(cell_t blockNum);
extern void blockFlushAll();
extern byte *blockRead(cell_t blockNum);

#endif __BLOCK_H__