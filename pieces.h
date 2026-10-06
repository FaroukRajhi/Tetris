// All the tetris pieces with their rotations and displacements

#ifndef _PIECES_
#define _PIECES_

class Pieces
{
    public:
      int GetBlockType    (int pPiece, int pRotation, int pX, int pY);
      int GetXinitialPosition (int pPiece, int pRotation);
      int GetYinitialPosition (int pPiece, int pRotation);
};

#endif