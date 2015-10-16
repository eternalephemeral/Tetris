/* pieces.h
Patrick Wang
301160793
CMPT 361 Asgn 1

Simple class containing data for tetris pieces, rotations

*/

#ifndef _PIECES_
#define _PIECES_


class Pieces
{
public:
	int getBlockType (int pPiece, int pRotation, int pX, int pY);
};

#endif
