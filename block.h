#ifndef __BLOCK_H__

#define BLOCK_SZ    2000

extern void blockIsText(cell_t blockNum);
extern void blockIsBinary(cell_t blockNum);
extern void blockIsDirty(cell_t blockNum);
extern void blockClear(cell_t blockNum, int force);
extern void blockFlush(cell_t blockNum, int force);
extern void blockFlushAll();
extern char *blockRead(cell_t blockNum);

#endif // __BLOCK_H__
