#ifndef __IDLIB_HUFFMAN_H__
#define __IDLIB_HUFFMAN_H__

#include "sys/sys_types.h"

constexpr int ID_HUFFMAN_MAX_SYMBOL = 256;
constexpr int ID_HUFFMAN_NYT = ID_HUFFMAN_MAX_SYMBOL;
constexpr int ID_HUFFMAN_INTERNAL_NODE = ID_HUFFMAN_MAX_SYMBOL + 1;

struct idHuffmanNode {
	idHuffmanNode *left;
	idHuffmanNode *right;
	idHuffmanNode *parent;
	idHuffmanNode *next;
	idHuffmanNode *prev;
	idHuffmanNode **head;
	int weight;
	int symbol;
};

struct idHuffmanTree {
	int blocNode;
	int blocPtrs;
	idHuffmanNode *tree;
	idHuffmanNode *lhead;
	idHuffmanNode *ltail;
	idHuffmanNode *loc[ID_HUFFMAN_MAX_SYMBOL + 1];
	idHuffmanNode **freelist;
	idHuffmanNode nodeList[768];
	idHuffmanNode *nodePtrs[768];
};

struct idHuffmanTables {
	idHuffmanTree compressor;
	idHuffmanTree decompressor;
};

class idHuffman {
public:
	void Init();
	void AddRef(byte ch);
	void DecodeAddRef(byte ch);
	void OffsetTransmit(int ch, byte *output, int *offset);
	void OffsetReceive(int *ch, byte *input, int *offset);
	static void PutBit(int bit, byte *output, int *offset);
	static int GetBit(byte *input, int *offset);

	idHuffmanTables tables;
};

#endif
