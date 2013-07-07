/*
 *  Fling.cpp
 *  hog2
 *
 *  Created by Nathan Sturtevant on 3/5/10.
 *  Copyright 2010 NS Software. All rights reserved.
 *
 */

#include "Fling.h"
#include <stdint.h>
#include <string.h>

void FlingBoard::AddFling(unsigned int x, unsigned int y)
{
	assert(x < width);
	assert(y < height);
	if (board[y*width+x] == false)
	{
		board[y*width+x] = true;
		locs.push_back(y*width+x);
		
		std::sort(locs.begin(), locs.end(), std::greater<int>());
	}
}

void FlingBoard::AddFling(unsigned int offset)
{
	assert(offset < board.size());
	assert(board[offset] == false);
	board[offset] = true;
	locs.push_back(offset);
	std::sort(locs.begin(), locs.end(), std::greater<int>());
}

void FlingBoard::RemoveFling(unsigned int x, unsigned int y)
{
	assert(x < width);
	assert(y < height);
	RemoveFling(y*width+x);
}

void FlingBoard::RemoveFling(unsigned int offset)
{
	board[offset] = false;
	for (unsigned int x = 0; x < width*height; x++)
	{
		if (locs[x] == offset)
		{
			locs[x] = locs.back();
			locs.pop_back();
			std::sort(locs.begin(), locs.end(), std::greater<int>());
			return;
		}
	}
}

bool FlingBoard::HasPiece(int x, int y) const
{
	return (board[y*width+x]);
}

bool FlingBoard::CanMove(int which, int x, int y) const
{
	int xx = locs[which]%width;
	int yy = locs[which]/width;
	xx+=x; yy += y;
	
	bool first = true;
	while ((xx >= 0) && (xx < width) && (yy >= 0) && (yy < height))
	{
		if (board[yy*width+xx])
			return !first;
		first = false;
		xx+=x; yy += y;
	}
	return false;

}

void FlingBoard::Move(int which, int x, int y)
{
	int xx = locs[which]%width;
	int yy = locs[which]/width;

	int lastx = xx;
	int lasty = yy;
	xx+=x; yy += y;
	while ((xx >= 0) && (xx < width) && (yy >= 0) && (yy < height))
	{
		board[lasty*width+lastx] = board[yy*width+xx];

		lastx = xx;
		lasty = yy;
		xx+=x; yy += y;
	}
	board[lasty*width+lastx] = false;
	locs.resize(0);
	for (int t = board.size()-1; t >= 0; t--)
		if (board[t])
			locs.push_back(t);
}

Fling::Fling()
{
	initBinomial();
	//		initBinomialSums();
}

void Fling::GetSuccessors(const FlingBoard &nodeID, std::vector<FlingBoard> &neighbors) const
{
	neighbors.resize(0);
	for (unsigned int x = 0; x < nodeID.locs.size(); x++)
	{
		if (nodeID.CanMove(x, 1, 0))
		{ FlingBoard b(nodeID); b.Move(x, 1, 0); neighbors.push_back(b); }
		if (nodeID.CanMove(x, -1, 0))
		{ FlingBoard b(nodeID); b.Move(x, -1, 0); neighbors.push_back(b); }
		if (nodeID.CanMove(x, 0, 1))
		{ FlingBoard b(nodeID); b.Move(x, 0, 1); neighbors.push_back(b); }
		if (nodeID.CanMove(x, 0, -1))
		{ FlingBoard b(nodeID); b.Move(x, 0, -1); neighbors.push_back(b); }
	}
}

void Fling::GetActions(const FlingBoard &nodeID, std::vector<FlingMove> &actions) const
{
	assert(false); 
}

void Fling::ApplyAction(FlingBoard &s, FlingMove a) const
{
}

void Fling::UndoAction(FlingBoard &s, FlingMove a) const
{
}

uint64_t Fling::GetStateHash(const FlingBoard &node) const
{
	uint64_t hash = 0;
	for (unsigned int x = 0; x < node.locs.size(); x++)
	{
		hash |= (1ull<<node.locs[x]);
//		std::cout << "Storing piece at " << node.locs[x] << std::endl;
//		printf("0x%llX (out:%d)\n", hash, x);
	}
//	printf("0x%llX (out)\n", hash);
	return hash;
//	return 0;
}

void Fling::GetStateFromHash(uint64_t parent, FlingBoard &s) const
{
//	printf("0x%llX (in)\n", parent);
	s.Reset();
	for (int x = 0; x < s.width*s.height; x++)
	{
		if (1 == ((parent>>x)&0x1))
		{
//			std::cout << "Setting piece at " << x << std::endl;
			s.AddFling(x);
		}
	}
}

uint64_t Fling::GetActionHash(FlingMove act) const
{
	return 0;
}

void Fling::OpenGLDraw(const FlingBoard&b) const
{
	double radius = 1.0/(1+max(b.width, b.height));
	double diameter = radius*2;
	double xLoc = -1+radius;
	double yLoc = -1+radius;

	glColor3f(0.0, 0.0, 0.5); //
	glBegin(GL_QUADS);
	glVertex3f(-1+diameter, -1+diameter, 0);
	glVertex3f(-1+diameter, -1+(diameter*b.height)+diameter, 0);
	glVertex3f(-1+diameter*(b.width)+diameter, -1+diameter*(b.height)+diameter, 0);
	glVertex3f(-1+diameter*(b.width)+diameter, -1+diameter, 0);
	glEnd();
	
	glLineWidth(2.0);
	glColor3f(1.0, 1.0, 1.0); // white
	glBegin(GL_LINES);
	for (double x = 0; x <= b.width; x++)
	{
		glVertex3f(-1+(x+1)*diameter, -1+diameter, 0);
		glVertex3f(-1+(x+1)*diameter, -1+diameter*b.height+diameter, -0.01);
		xLoc += diameter;
	}
	for (double y = 0; y <= b.height; y++)
	{
		yLoc += diameter;
		glVertex3f(-1+diameter, -1+(y+1)*diameter, 0);
		glVertex3f(-1+diameter*b.width+diameter, -1+(y+1)*diameter, -0.01);
	}
	glEnd();
	glLineWidth(1.0);
	
	xLoc = -1+radius;
	for (double x = 0; x < b.width; x++)
	{
		xLoc += diameter;
		yLoc = -1+radius;
		for (double y = 0; y < b.height; y++)
		{
			yLoc += diameter;
			recColor r = getColor(x+y*b.width, 0, b.width*b.height, 4);
			glColor3f(r.r, r.g, r.b);
			if (b.HasPiece(x, y))
				DrawSphere(xLoc, yLoc, 0, radius);
		}
	}
}

bool Fling::GetXYFromPoint(const FlingBoard &b, point3d loc, int &x, int &y) const
{
	double radius = 1.0/(1+max(b.width, b.height));
	double diameter = radius*2;

	loc.x = loc.x-diameter+1;
	loc.y = loc.y-diameter+1;
	loc.x /= diameter;
	loc.y /= diameter;
	x = loc.x;
	y = loc.y;
	
	if (x >= 0 && x < b.width && y >= 0 && y < b.height)
		return true;
	return false;
}





int64_t Fling::getMaxSinglePlayerRank(int spots, int numPieces)
{
	return binomial(spots, numPieces);
}

int64_t Fling::getMaxSinglePlayerRank2(int spots, int numPieces)
{
	return binomial(spots-(numPieces-2), 2);
}

int64_t Fling::getMaxSinglePlayerRank2(int spots, int numPieces, int64_t firstIndex)
{
	int NUM_SPOTS = spots;
	int NUM_PIECES = numPieces;
	unsigned int ls = 2;
	int i = 0;
	for (; ls > 0; ++i)
	{
		int64_t value;
		if (ls > 0)
		{
			value = binomial(NUM_SPOTS-(NUM_PIECES-2) - i - 1, ls - 1);
		}
		else {
			value = 0;
		}
		if (firstIndex < value)
		{
			ls--;
		}
		else {
			firstIndex -= value;
		}
	}
	return binomial(NUM_SPOTS-i,NUM_PIECES-2);
}

int64_t Fling::rankPlayer(FlingBoard &s)
{
	int NUM_SPOTS = s.width*s.height;
	int NUM_PIECES = s.locs.size();

	int64_t r2 = 0;
	int last = NUM_SPOTS-1;
	for (int x = 0; x < NUM_PIECES; x++)
	{
		int64_t tmp = binomialSum(last, NUM_SPOTS-s.locs[NUM_PIECES-1-x]-1, NUM_PIECES-1-x);
		r2 += tmp;
		last = NUM_SPOTS-s.locs[NUM_PIECES-1-x]-1-1;
	}
	return r2;
}

void Fling::rankPlayer(FlingBoard &s, int64_t &index1, int64_t &index2)
{
	int NUM_SPOTS = s.width*s.height;
	int NUM_PIECES = s.locs.size();
	index1 = 0;
	int tot = NUM_SPOTS-1-(NUM_PIECES-2);
	int last = tot;
	for (int x = 0; x < 2; x++)
	{
		int64_t tmp = binomialSum(last, tot-s.locs[NUM_PIECES-1-x], (2)-1-x);
		index1 += tmp;
		last = tot-s.locs[NUM_PIECES-1-x]-1;
	}
	
	index2 = 0;
	last = NUM_SPOTS-s.locs[NUM_PIECES-1-1]-1-1;
	for (int x = 2; x < NUM_PIECES; x++)
	{
		int64_t tmp = binomialSum(last, NUM_SPOTS-s.locs[NUM_PIECES-1-x]-1, NUM_PIECES-1-x);
		index2 += tmp;
		last = NUM_SPOTS-s.locs[NUM_PIECES-1-x]-1-1;
	}
}

void Fling::rankPlayerFirstTwo(FlingBoard &s, int64_t &index1)
{
	int NUM_SPOTS = s.width*s.height;
	int NUM_PIECES = s.locs.size();

	index1 = 0;
	int tot = NUM_SPOTS-1-(NUM_PIECES-2);
	int last = tot;
	for (int x = 0; x < 2; x++)
	{
		int64_t tmp = binomialSum(last, tot-s.locs[NUM_PIECES-1-x], (2)-1-x);
		index1 += tmp;
		last = tot-s.locs[NUM_PIECES-1-x]-1;
	}
}

void Fling::rankPlayerRemaining(FlingBoard &s, int64_t &index2)
{
	int NUM_SPOTS = s.width*s.height;
	int NUM_PIECES = s.locs.size();

	int last;
	index2 = 0;
	last = NUM_SPOTS-s.locs[NUM_PIECES-1-1]-1-1;
	for (int x = 2; x < NUM_PIECES; x++)
	{
		int64_t tmp = binomialSum(last, NUM_SPOTS-s.locs[NUM_PIECES-1-x]-1, NUM_PIECES-1-x);
		index2 += tmp;
		last = NUM_SPOTS-s.locs[NUM_PIECES-1-x]-1-1;
	}
}


// returns true if it is a valid unranking given existing pieces
bool Fling::unrankPlayer(int64_t theRank, int pieces, FlingBoard &s)
{
	int NUM_SPOTS = s.width*s.height;
	int NUM_PIECES = pieces;

//	int tag = who + 1;
	unsigned int ls = NUM_PIECES;
//	memset(s.board, 0, NUM_SPOTS*sizeof(int));
	s.board.resize(0);
	s.board.resize(s.width*s.height);
	s.locs.resize(0);
	s.locs.resize(pieces);
	for (int i=0; ls > 0; ++i)
	{
		int64_t value;
		if (ls > 0)
		{
			value = binomial(NUM_SPOTS - i - 1, ls - 1);
		}
		else {
			value = 0;
		}
		if (theRank < value)
		{
			s.board[i] = true;
			s.locs[ls-1] = i;
			ls--;
		}
		else {
			s.board[i] = 0;
			theRank -= value;
		}
	}
	for (int x = 1; x < NUM_PIECES; x++)
		assert(s.locs[x-1] > s.locs[x]);
	return true;
}

//
//
//void Fling::initBinomialSums()
//{
//	if (theSums.size() == 0)
//	{
//		theSums.resize((NUM_PIECES+1)*(NUM_SPOTS+1));
//		//		sums.resize(NUM_PIECES+1);
//		for (int x = 0; x <= NUM_PIECES; x++)
//		{
//			//			sums[x].resize(NUM_SPOTS+1);
//			int64_t result = 0;
//			for (int y = 0; y <= NUM_SPOTS; y++)
//			{
//				result+=binomial(y, x);
//				//				sums[x][y] = result;
//				theSums[x*(NUM_SPOTS+1)+y] = result;
//			}
//		}
//	}
//}
//
int64_t Fling::binomialSum(unsigned int n1, unsigned int n2, unsigned int k)
{
	//	static std::vector<std::vector<int64_t> > sums;
	//assert(theSums[k*(NUM_SPOTS+1)+n1]-theSums[k*(NUM_SPOTS+1)+n2] == sums[k][n1]-sums[k][n2]);
	int64_t result = 0;
	for (int x = n1; x > n2; x--)
		result += binomial(x, k);
	return result;
	//return theSums[k*(NUM_SPOTS+1)+n1]-theSums[k*(NUM_SPOTS+1)+n2];
	//return sums[k][n1]-sums[k][n2];
}

const int maxPieces = 14;

void Fling::initBinomial()
{
	if (binomials.size() == 0)
	{
		for (int x = 0; x <= 56; x++)
		{
			for (int y = 0; y <= maxPieces; y++)
			{
				binomials.push_back(bi(x, y));
			}
		}
	}
}

int64_t Fling::binomial(unsigned int n, unsigned int k)
{
	//assert(bi(n, k) == binomials[n*(1+NUM_PLAYERS*NUM_PIECES)+k]);
	return binomials[n*(1+maxPieces)+k];
}

int64_t Fling::bi(unsigned int n, unsigned int k)
{
	int64_t num = 1;
	const unsigned int bound = (n - k);
	while(n > bound)
	{
		num *= n--;
	}
	
	int64_t den = 1;
	while(k > 1)
	{
		den *= k--;
	}
	return num / den;
}

