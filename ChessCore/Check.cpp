#include "stdafx.h"
#include "Check.h"
#include "Move.h"
#include "BoardPositionsCache.h"

using namespace Chess;

GameChecks::GameChecks(const GameState &state)
	:_state(state)
{
}

GameChecks::~GameChecks()
{
}

PositionPiece GetKingPosition(const BoardAptr &board, EPieceColors side)
{
	return MoveGeneration::GetPositionsOf(*board, KING, side)[0];
}

bool IsInCheck(BoardPositionsCacheAptr cache, BoardAptr board, EPieceColors side)
{
	auto imTheKing = GetKingPosition(board, side);

	auto attackMap = cache->GetAttackMap(board, OppositeSideOf(side));
	return MoveGeneration::IsUnderAttack(*attackMap, imTheKing.Position);
}

bool GameChecks::IsInCheck(EPieceColors side)
{
	return ::IsInCheck(_state.Cache, _state.Board, side);
}

bool GameChecks::IsCheckMate(EPieceColors side)
{
	auto result = false;
	if (IsInCheck(side))
	{
		result = true;
		auto imTheKing = GetKingPosition(_state.Board, side);
		auto attackMap = _state.Cache->GetAttackMap(_state.Board, OppositeSideOf(side));

		auto whoAttacksTheKing = attackMap->at(imTheKing.Position);
		auto thisSideKillMap = _state.Cache->GetViktimsMap(_state.Board, side);

		// Can capture the checking piece or move away
		auto kingAttackList = (*thisSideKillMap)[imTheKing.Position];
		auto newBoard = std::make_shared<Board>(*_state.Board);

		// Someone can protect King
		for (auto attacker : whoAttacksTheKing)
		{
			for (auto piecePtr = thisSideKillMap->begin(); piecePtr != thisSideKillMap->end(); ++piecePtr)
			{
				for (auto myPieceMovesTo : piecePtr->second)
				{
					if (attacker.Piece.Type == KNIGHT && myPieceMovesTo.Position != attacker.Position)
					{
						// my piece is unable to capture the Knight
						continue;
					}

					auto capturing = !newBoard->At(myPieceMovesTo.Position).IsEmpty();

					newBoard->DoMove({ (BoardPosition)piecePtr->first, myPieceMovesTo.Position, capturing });
					if (!::IsInCheck(_state.Cache, newBoard, side))
					{
						//Piece can capture the threatener or escape
						return false;
					}

					newBoard->UndoLastMove();
				}
			}
		}
	}

	return result;
}
