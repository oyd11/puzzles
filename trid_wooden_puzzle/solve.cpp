/**
 * The Tridenskaja-puzzle, 32-bit hack-solution:
 *
 * Storing 5x5 board (25 bits) in a 32bit register
 *
 * Note: A 64 bit solution could/should be faster to run (code?)
 * as we could fit the 5x5 board into a 8x8 board, thus do nicer shifts
 *
 * OyD11 ^ NHF , 2007
 */

#include <stdio.h>

#include "pieces.h"

//#define VERBOSE

typedef unsigned long Piece;

//Piece board=0; // game-state
Piece board=0x1000; // game-state
// keep middle empty

char takenH[4]={0};
char takenV[4]={0};

// @Module: Stack
// Stack:: (max number of pieces)
#define STACKSZ 64
Piece pieceStack[STACKSZ];
int stackPointer=0;
bool stackEmpty(void){
	return 0==stackPointer; }
void pushPiece(Piece piece) {
	pieceStack[stackPointer++] = piece; }
Piece popPiece(){
	return pieceStack[--stackPointer]; }
// @End (Stack) ------

// @Module: I/O
// Textfile piece loading/printing:
Piece strToInt64(char *str) {
	char c; Piece bitMask=1,ret=0;
	while(c=*str++) {
		if (' '!=c) ret |= bitMask;
		bitMask <<=1;
	}
	return ret;
}

void printPiece(Piece piece) {
	char str[5*6+1];int pos=0,i;
	for (i=0;i<5;++i) {
		str[pos++]=1&piece ? 'o':' ';
		str[pos++]=2&piece ? 'o':' ';
		str[pos++]=4&piece ? 2!=i?'o':'x' :' ';
		str[pos++]=8&piece ? 'o':' ';
		str[pos++]=0x10&piece ? 'o':' ';
		str[pos++]='\n';
		piece>>=5;
	}
	str[pos]=0;
	printf("%s",str);
}
// @End (I/O)

// @Module: Piece handing
#define pieceFits(P) (!(board&P))
#define addPiece(P) board |= P
#define rmPiece(P) board &= ~P

// this is the ugly part, which could (should?) be nicer on 64 bits(?)
Piece flip(Piece piece) {
	Piece ret=0;
// TODO: more elegant / efficient code for flipping a bitrow
	//Piece row0=piece&(1|2|4|8|0x10);
	ret |= piece&1 ? 0x200:0;
	ret |= piece&2 ? 0x100:0;
	ret |= piece&4 ? 0x80:0;
	ret |= piece&8 ? 0x40:0;
	ret |= piece&0x10 ? 0x20:0;

//	Piece row1=piece&(0x20|0x40|0x80|0x100|0x200);
//	return (row0<<5)|(row1>>5);
	ret |= piece&0x20 ? 0x10:0;
	ret |= piece&0x40 ? 8:0;
	ret |= piece&0x80 ? 4:0;
	ret |= piece&0x100 ? 2:0;
	ret |= piece&0x200 ? 1:0;
	return ret;
}

Piece rot(Piece piece) {
// quite(!) ugly:
	//Piece row0=piece&(1|2|4|8|0x10);
	Piece ret=piece&1; // col0
	ret |= piece&2 ? 0x20 : 0;
	ret |= piece&4 ? 0x400 : 0;
	ret |= piece&8 ? 0x8000 : 0;
	ret |= piece&0x10 ? 0x100000 : 0;
//	Piece row1=piece&(0x20|0x40|0x80|0x100|0x200);
//	col1:
	ret |= piece&0x20 ? 2 : 0;
	ret |= piece&0x40 ? 0x40 : 0;
	ret |= piece&0x80 ? 0x800 : 0;
	ret |= piece&0x100 ? 0x10000 : 0;
	ret |= piece&0x200 ? 0x200000 : 0;

	return ret;
}

Piece flipRot(Piece piece) {
// quite(!) ugly:
	//Piece row0=piece&(1|2|4|8|0x10);
	Piece ret=piece&1? 0x200000:0; // col0
	ret |= piece&2 ? 0x10000 : 0;
	ret |= piece&4 ? 0x800 : 0;
	ret |= piece&8 ? 0x40 : 0;
	ret |= piece&0x10 ? 2: 0;
//	Piece row1=piece&(0x20|0x40|0x80|0x100|0x200);
//	col1:
	ret |= piece&0x20 ? 0x100000 : 0;
	ret |= piece&0x40 ? 0x8000 : 0;
	ret |= piece&0x80 ? 0x400 : 0;
	ret |= piece&0x100 ? 0x20: 0;
	ret |= piece&0x200 ? 1 : 0;

	return ret;
}
// @End (Piece handing)

// @Module: Game-tree:
// forward decl
bool tryNextPiece(int depth);

// returns working index
int tryDir(Piece piece,int shift,int depth) {
	int i;
#ifdef VERBOSE
	printf("Working on:\n---..\n");
#endif
	for (i=0;i<4;
		++i,piece <<=shift // try next row
		) {
#ifdef VERBOSE
		printPiece(piece);
		printf("--\n");
#endif
		if (5==shift) {
			if (takenH[i]) {
				continue; // place taken
			}
		} else {
			if (takenV[i]) {
				continue;
			}
		}
		if (pieceFits(piece)) {

			addPiece(piece);
			if (5==shift)
				takenH[i]=1;
			else takenV[i]=1;

#ifdef VERBOSE
			printf("Fits!\n");
			printf("board:\n");
			printPiece(board);
			printf("~~~~\n");
#endif
			if (tryNextPiece(depth+1))
				return i; // w/o removing

			rmPiece(piece);
			if (5==shift)
				takenH[i]=0;
			else takenV[i]=0;

			board &= ~piece; // rm piece
		}
	}
	return -1;
}

// DFS, walk the game-tree:
bool tryNextPiece(int depth) {
	char *word[]={"plain","flip","rot","rotFlip"};
	Piece piece;
	int i,dir=0;
#ifdef VERBOSE
	printf("Try depth(%d)\n",depth);
#endif
	if (stackEmpty()) {
		// we've won! recurse-out answer
		printf("Winning!\n");
		printf("BoardState:\n");
		printPiece(board);
		printf("--~~~--\nunwinding solution:\n");
		return true;
	}
	piece = popPiece();
	i=tryDir(piece,5,depth); // Horizontal bits
	if (i>=0) goto weWin;
	//flip piece
	dir++;
	i=tryDir(flip(piece),5,depth);
	if (i>=0) goto weWin;
	// rot:
	dir++;
	i=tryDir(rot(piece),1,depth);
	if (i>=0) goto weWin;
	// rotFlip:
	dir++;
	i=tryDir(flipRot(piece),1,depth);
	if (i>=0) goto weWin;

	// not sure about this returning pieces bussiness...
	pushPiece(piece); // but each piece gotta be somewhere
	// so it makes somekindof sense...

	return false;

weWin:
	printf("uw [%d]\n",depth);
	switch (dir){
		case 0:
			printPiece(piece<<(5*i));
			break;
		case 1:
			printPiece(flip(piece)<<(5*i));
			break;
		case 2:
			printPiece(rot(piece)<<i);
			break;
		case 3:
			printPiece(flipRot(piece)<<i);
			break;
	}
	printf("Direction: %s, offset: %d\n",word[dir],i);
	return true;
}
//@End  (Game-tree)


// @Module: Main
int main(void) {

// Load puzzle-pieces:
	int i;Piece piece;int nPieces=sizeof(piecesStr)/11;
	printf("%d pieces\n",nPieces);
	for (i=0;i<nPieces;++i) {
		piece=strToInt64(piecesStr[i]);
		pushPiece(piece);
#ifdef VERBOSE
		printPiece(piece);
		printf("---\n");
		printPiece(flip(piece));
		printf("---\n");
		printPiece(rot(piece));
		printf("---\n");
		printPiece(flipRot(piece));
		printf("---\n");
		printf("~-~\n");
#endif
	}

	printf("STarTing::\n---!!\n");
	if (!tryNextPiece(0)) {
		printf("No solution!\n");
		return -1;
	}
	printf("Solved!");
	return 0;
}
// @End (Main)

// EOF
