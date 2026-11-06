/*
===========================================================================

Return to Castle Wolfenstein single player GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein single player GPL Source Code (RTCW SP Source Code).  

RTCW SP Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RTCW SP Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RTCW SP Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the RTCW SP Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the RTCW SP Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/


/* This is based on the Adaptive Huffman algorithm described in Sayood's Data
 * Compression book.  The ranks are not actually stored, but implicitly defined
 * by the location of a node within a doubly-linked list */

#include "precompiled.h"
#include "Huffman.h"

static int bloc = 0;

#define NYT ID_HUFFMAN_NYT
#define INTERNAL_NODE ID_HUFFMAN_INTERNAL_NODE

static void Huff_putBit( int bit, byte *fout, int *offset ) {
	bloc = *offset;
	if ( ( bloc & 7 ) == 0 ) {
		fout[( bloc >> 3 )] = 0;
	}
	fout[( bloc >> 3 )] |= bit << ( bloc & 7 );
	bloc++;
	*offset = bloc;
}

static int Huff_getBit( byte *fin, int *offset ) {
	int t;
	bloc = *offset;
	t = ( fin[( bloc >> 3 )] >> ( bloc & 7 ) ) & 0x1;
	bloc++;
	*offset = bloc;
	return t;
}

/* Add a bit to the output file (buffered) */
static void add_bit( char bit, byte *fout ) {
	if ( ( bloc & 7 ) == 0 ) {
		fout[( bloc >> 3 )] = 0;
	}
	fout[( bloc >> 3 )] |= bit << ( bloc & 7 );
	bloc++;
}

/* Receive one bit from the input file (buffered) */
static int get_bit( byte *fin ) {
	int t;
	t = ( fin[( bloc >> 3 )] >> ( bloc & 7 ) ) & 0x1;
	bloc++;
	return t;
}

static idHuffmanNode **get_ppnode( idHuffmanTree* huff ) {
	idHuffmanNode **tppnode;
	if ( !huff->freelist ) {
		return &( huff->nodePtrs[huff->blocPtrs++] );
	} else {
		tppnode = huff->freelist;
		huff->freelist = (idHuffmanNode **)*tppnode;
		return tppnode;
	}
}

static void free_ppnode( idHuffmanTree* huff, idHuffmanNode **ppnode ) {
	*ppnode = (idHuffmanNode *)huff->freelist;
	huff->freelist = ppnode;
}

/* Swap the location of these two nodes in the tree */
static void swap( idHuffmanTree* huff, idHuffmanNode *node1, idHuffmanNode *node2 ) {
	idHuffmanNode *par1, *par2;

	par1 = node1->parent;
	par2 = node2->parent;

	if ( par1 ) {
		if ( par1->left == node1 ) {
			par1->left = node2;
		} else {
			par1->right = node2;
		}
	} else {
		huff->tree = node2;
	}

	if ( par2 ) {
		if ( par2->left == node2 ) {
			par2->left = node1;
		} else {
			par2->right = node1;
		}
	} else {
		huff->tree = node1;
	}

	node1->parent = par2;
	node2->parent = par1;
}

/* Swap these two nodes in the linked list (update ranks) */
static void swaplist( idHuffmanNode *node1, idHuffmanNode *node2 ) {
	idHuffmanNode *par1;

	par1 = node1->next;
	node1->next = node2->next;
	node2->next = par1;

	par1 = node1->prev;
	node1->prev = node2->prev;
	node2->prev = par1;

	if ( node1->next == node1 ) {
		node1->next = node2;
	}
	if ( node2->next == node2 ) {
		node2->next = node1;
	}
	if ( node1->next ) {
		node1->next->prev = node1;
	}
	if ( node2->next ) {
		node2->next->prev = node2;
	}
	if ( node1->prev ) {
		node1->prev->next = node1;
	}
	if ( node2->prev ) {
		node2->prev->next = node2;
	}
}

/* Do the increments */
static void increment( idHuffmanTree* huff, idHuffmanNode *node ) {
	idHuffmanNode *lnode;

	if ( !node ) {
		return;
	}

	if ( node->next != NULL && node->next->weight == node->weight ) {
		lnode = *node->head;
		if ( lnode != node->parent ) {
			swap( huff, lnode, node );
		}
		swaplist( lnode, node );
	}
	if ( node->prev && node->prev->weight == node->weight ) {
		*node->head = node->prev;
	} else {
		*node->head = NULL;
		free_ppnode( huff, node->head );
	}
	node->weight++;
	if ( node->next && node->next->weight == node->weight ) {
		node->head = node->next->head;
	} else {
		node->head = get_ppnode( huff );
		*node->head = node;
	}
	if ( node->parent ) {
		increment( huff, node->parent );
		if ( node->prev == node->parent ) {
			swaplist( node, node->parent );
			if ( *node->head == node ) {
				*node->head = node->parent;
			}
		}
	}
}

static void Huff_addRef( idHuffmanTree* huff, byte ch ) {
	idHuffmanNode *tnode, *tnode2;
	if ( huff->loc[ch] == NULL ) { /* if this is the first transmission of this node */
		tnode = &( huff->nodeList[huff->blocNode++] );
		tnode2 = &( huff->nodeList[huff->blocNode++] );

		tnode2->symbol = INTERNAL_NODE;
		tnode2->weight = 1;
		tnode2->next = huff->lhead->next;
		if ( huff->lhead->next ) {
			huff->lhead->next->prev = tnode2;
			if ( huff->lhead->next->weight == 1 ) {
				tnode2->head = huff->lhead->next->head;
			} else {
				tnode2->head = get_ppnode( huff );
				*tnode2->head = tnode2;
			}
		} else {
			tnode2->head = get_ppnode( huff );
			*tnode2->head = tnode2;
		}
		huff->lhead->next = tnode2;
		tnode2->prev = huff->lhead;

		tnode->symbol = ch;
		tnode->weight = 1;
		tnode->next = huff->lhead->next;
		if ( huff->lhead->next ) {
			huff->lhead->next->prev = tnode;
			if ( huff->lhead->next->weight == 1 ) {
				tnode->head = huff->lhead->next->head;
			} else {
				/* this should never happen */
				tnode->head = get_ppnode( huff );
				*tnode->head = tnode2;
			}
		} else {
			/* this should never happen */
			tnode->head = get_ppnode( huff );
			*tnode->head = tnode;
		}
		huff->lhead->next = tnode;
		tnode->prev = huff->lhead;
		tnode->left = tnode->right = NULL;

		if ( huff->lhead->parent ) {
			if ( huff->lhead->parent->left == huff->lhead ) { /* lhead is guaranteed to by the NYT */
				huff->lhead->parent->left = tnode2;
			} else {
				huff->lhead->parent->right = tnode2;
			}
		} else {
			huff->tree = tnode2;
		}

		tnode2->right = tnode;
		tnode2->left = huff->lhead;

		tnode2->parent = huff->lhead->parent;
		huff->lhead->parent = tnode->parent = tnode2;

		huff->loc[ch] = tnode;

		increment( huff, tnode2->parent );
	} else {
		increment( huff, huff->loc[ch] );
	}
}

/* Get a symbol */
static int Huff_Receive( idHuffmanNode *node, int *ch, byte *fin ) {
	while ( node && node->symbol == INTERNAL_NODE ) {
		if ( get_bit( fin ) ) {
			node = node->right;
		} else {
			node = node->left;
		}
	}
	if ( !node ) {
		idLib::FatalError( "Illegal tree!" );
	}
	return ( *ch = node->symbol );
}

/* Get a symbol */
static void Huff_offsetReceive( idHuffmanNode *node, int *ch, byte *fin, int *offset ) {
	bloc = *offset;
	while ( node && node->symbol == INTERNAL_NODE ) {
		if ( get_bit( fin ) ) {
			node = node->right;
		} else {
			node = node->left;
		}
	}
	if ( !node ) {
		idLib::FatalError( "Illegal tree!" );
	}
	*ch = node->symbol;
	*offset = bloc;
}

/* Send the prefix code for this node */
static void send( idHuffmanNode *node, idHuffmanNode *child, byte *fout ) {
	if ( node->parent ) {
		send( node->parent, node, fout );
	}
	if ( child ) {
		if ( node->right == child ) {
			add_bit( 1, fout );
		} else {
			add_bit( 0, fout );
		}
	}
}

/* Send a symbol */
static void Huff_transmit( idHuffmanTree *huff, int ch, byte *fout ) {
	int i;
	if ( huff->loc[ch] == NULL ) {
		/* idHuffmanNode hasn't been transmitted, send a NYT, then the symbol */
		Huff_transmit( huff, NYT, fout );
		for ( i = 7; i >= 0; i-- ) {
			add_bit( (char)( ( ch >> i ) & 0x1 ), fout );
		}
	} else {
		send( huff->loc[ch], NULL, fout );
	}
}

static void Huff_offsetTransmit( idHuffmanTree *huff, int ch, byte *fout, int *offset ) {
	bloc = *offset;
	send( huff->loc[ch], NULL, fout );
	*offset = bloc;
}

static void Huff_Init( idHuffmanTables *huff ) {

	memset( &huff->compressor, 0, sizeof( idHuffmanTree ) );
	memset( &huff->decompressor, 0, sizeof( idHuffmanTree ) );

	// Initialize the tree & list with the NYT node
	huff->decompressor.tree = huff->decompressor.lhead = huff->decompressor.ltail = huff->decompressor.loc[NYT] = &( huff->decompressor.nodeList[huff->decompressor.blocNode++] );
	huff->decompressor.tree->symbol = NYT;
	huff->decompressor.tree->weight = 0;
	huff->decompressor.lhead->next = huff->decompressor.lhead->prev = NULL;
	huff->decompressor.tree->parent = huff->decompressor.tree->left = huff->decompressor.tree->right = NULL;

	// Add the NYT (not yet transmitted) node into the tree/list */
	huff->compressor.tree = huff->compressor.lhead = huff->compressor.loc[NYT] =  &( huff->compressor.nodeList[huff->compressor.blocNode++] );
	huff->compressor.tree->symbol = NYT;
	huff->compressor.tree->weight = 0;
	huff->compressor.lhead->next = huff->compressor.lhead->prev = NULL;
	huff->compressor.tree->parent = huff->compressor.tree->left = huff->compressor.tree->right = NULL;
	huff->compressor.loc[NYT] = huff->compressor.tree;
}

void idHuffman::Init() {
	Huff_Init( &tables );
}

void idHuffman::AddRef( byte ch ) {
	Huff_addRef( &tables.compressor, ch );
}

void idHuffman::DecodeAddRef( byte ch ) {
	Huff_addRef( &tables.decompressor, ch );
}

void idHuffman::OffsetTransmit( int ch, byte *output, int *offset ) {
	Huff_offsetTransmit( &tables.compressor, ch, output, offset );
}

void idHuffman::OffsetReceive( int *ch, byte *input, int *offset ) {
	Huff_offsetReceive( tables.decompressor.tree, ch, input, offset );
}

void idHuffman::PutBit( int bit, byte *output, int *offset ) {
	Huff_putBit( bit, output, offset );
}

int idHuffman::GetBit( byte *input, int *offset ) {
	return Huff_getBit( input, offset );
}
