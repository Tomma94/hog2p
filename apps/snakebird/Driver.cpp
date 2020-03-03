/*
 * $Id: sample.cpp,v 1.23 2006/11/01 23:33:56 nathanst Exp $
 *
 *  sample.cpp
 *  hog
 *
 *  Created by Nathan Sturtevant on 5/31/05.
 *  Copyright 2005 Nathan Sturtevant, University of Alberta. All rights reserved.
 *
 * This file is part of HOG.
 *
 */

#include <cstring>
#include "Common.h"
#include "Driver.h"
#include "Timer.h"
#include <deque>
#include "Combinations.h"
#include "SVGUtil.h"

#include <sys/stat.h>
bool fileExists(const char *name)
{
	struct stat buffer;
	return (stat(name, &buffer) == 0);
}
#include "SnakeBird.h"

std::string mapName;

bool recording = false;
bool parallel = false;

SnakeBird::SnakeBird sb(20, 20);
SnakeBird::SnakeBirdState snake;
int snakeControl = 0;
std::vector<SnakeBird::SnakeBirdState> history;

int main(int argc, char* argv[])
{
	InstallHandlers();
	RunHOGGUI(argc, argv, 640, 640);
	return 0;
}

/**
 * Allows you to install any keyboard handlers needed for program interaction.
 */
void InstallHandlers()
{
//	InstallKeyboardHandler(MyDisplayHandler, "Solve", "Solve current board", kAnyModifier, 'v');
//	InstallKeyboardHandler(MyDisplayHandler, "Test", "Test constraints", kAnyModifier, 't');
//	InstallKeyboardHandler(MyDisplayHandler, "Record", "Record a movie", kAnyModifier, 'r');
//	InstallKeyboardHandler(MyDisplayHandler, "Save", "Save current puzzle as svg", kAnyModifier, 's');
//	InstallKeyboardHandler(MyDisplayHandler, "Cycle Abs. Display", "Cycle which group abstraction is drawn", kAnyModifier, '\t');
//	InstallKeyboardHandler(MyDisplayHandler, "Prev Board", "Jump to next found board.", kAnyModifier, '[');
//	InstallKeyboardHandler(MyDisplayHandler, "Next Board", "Jump to prev found board", kAnyModifier, ']');
//	InstallKeyboardHandler(MyDisplayHandler, "Prev 100 Board", "Jump to next 100 found board.", kAnyModifier, '{');
//	InstallKeyboardHandler(MyDisplayHandler, "Next 100 Board", "Jump to prev 100 found board", kAnyModifier, '}');
	InstallKeyboardHandler(MyDisplayHandler, "Up", "Move up", kAnyModifier, 'w');
	InstallKeyboardHandler(MyDisplayHandler, "Down", "Move down", kAnyModifier, 's');
	InstallKeyboardHandler(MyDisplayHandler, "Left", "Move left", kAnyModifier, 'a');
	InstallKeyboardHandler(MyDisplayHandler, "Right", "Move right", kAnyModifier, 'd');
	InstallKeyboardHandler(MyDisplayHandler, "Undo", "Undo last move", kAnyModifier, 'q');
	InstallKeyboardHandler(MyDisplayHandler, "Next", "Select Next Snake", kAnyModifier, 'e');
	InstallKeyboardHandler(MyDisplayHandler, "Reset", "Reset Level", kAnyModifier, 'r');
	InstallKeyboardHandler(MyDisplayHandler, "Print", "Print screen to file", kAnyModifier, 'p');

	InstallCommandLineHandler(MyCLHandler, "-load", "-load <file>", "Run snake bird with the given file");
//	InstallCommandLineHandler(MyCLHandler, "-test", "-test", "Basic test with MD heuristic");
	
	InstallWindowHandler(MyWindowHandler);
	InstallMouseClickHandler(MyClickHandler, static_cast<tMouseEventType>(kMouseMove|kMouseUp|kMouseDrag));
}

void LoadLevel19()
{
	sb.SetGroundType(14, 5, SnakeBird::kFruit);

	sb.SetGroundType(1, 4, SnakeBird::kSpikes);
	sb.SetGroundType(1, 5, SnakeBird::kGround);
	sb.SetGroundType(4, 3, SnakeBird::kSpikes);
	sb.SetGroundType(4, 4, SnakeBird::kGround);
	sb.SetGroundType(3, 5, SnakeBird::kExit);

	sb.SetGroundType(4, 9, SnakeBird::kGround);
	sb.SetGroundType(4, 10, SnakeBird::kGround);
	sb.SetGroundType(4, 11, SnakeBird::kGround);
	sb.SetGroundType(4, 12, SnakeBird::kGround);

	sb.AddSnake(4, 6, {SnakeBird::kRight});
	sb.AddSnake(5, 7, {SnakeBird::kLeft, SnakeBird::kLeft, SnakeBird::kDown, SnakeBird::kDown, SnakeBird::kDown});
	sb.AddSnake(4, 8, {SnakeBird::kRight, SnakeBird::kDown, SnakeBird::kDown, SnakeBird::kDown});


	snake = sb.GetStart();
	history.clear();
	history.push_back(snake);
}

void LoadLevel22()
{
	sb.SetGroundType(3, 7, SnakeBird::kGround);
	sb.SetGroundType(3, 8, SnakeBird::kGround);
	sb.SetGroundType(3, 9, SnakeBird::kGround);
	sb.SetGroundType(3, 10, SnakeBird::kGround);
	sb.SetGroundType(4, 7, SnakeBird::kGround);
	sb.SetGroundType(4, 8, SnakeBird::kGround);
	sb.SetGroundType(4, 9, SnakeBird::kGround);
	sb.SetGroundType(4, 10, SnakeBird::kGround);
	sb.SetGroundType(5, 10, SnakeBird::kGround);
	sb.SetGroundType(6, 8, SnakeBird::kGround);
	sb.SetGroundType(6, 10, SnakeBird::kGround);
	sb.SetGroundType(7, 10, SnakeBird::kGround);
	sb.SetGroundType(8, 9, SnakeBird::kGround);
	sb.SetGroundType(8, 10, SnakeBird::kGround);
	sb.SetGroundType(9, 9, SnakeBird::kGround);
	sb.SetGroundType(9, 10, SnakeBird::kGround);
	
	sb.SetGroundType(5, 1, SnakeBird::kExit);
	
	sb.AddSnake(4, 6, {SnakeBird::kLeft, SnakeBird::kLeft});
	//		sb.AddSnake(10, 10, {SnakeBird::kRight});
	//		sb.AddSnake(15, 10, {SnakeBird::kRight});
	//		sb.AddSnake(20, 10, {SnakeBird::kRight});
	
	sb.SetGroundType(8, 7, SnakeBird::kBlock1);
	sb.SetGroundType(8, 8, SnakeBird::kBlock1);
	sb.SetGroundType(9, 7, SnakeBird::kBlock1);
	sb.SetGroundType(9, 8, SnakeBird::kBlock1);
		
	snake = sb.GetStart();
	history.clear();
	history.push_back(snake);
}

void LoadLevel39()
{
	for (int x = 3; x <= 14; x++)
		sb.SetGroundType(x, 12, SnakeBird::kGround);
	for (int x = 3; x <= 6; x++)
		sb.SetGroundType(x, 11, SnakeBird::kGround);
	for (int x = 9; x <= 14; x++)
		sb.SetGroundType(x, 11, SnakeBird::kGround);
	for (int x = 11; x <= 14; x++)
	{
		sb.SetGroundType(x, 9, SnakeBird::kGround);
		sb.SetGroundType(x, 6, SnakeBird::kGround);
	}
	for (int x = 10; x <= 14; x++)
	{
		sb.SetGroundType(x, 10, SnakeBird::kGround);
		sb.SetGroundType(x, 8, SnakeBird::kGround);
		sb.SetGroundType(x, 7, SnakeBird::kGround);
		sb.SetGroundType(x, 5, SnakeBird::kGround);
		sb.SetGroundType(x, 4, SnakeBird::kGround);
	}

	
	sb.SetGroundType(12, 1, SnakeBird::kExit);
	
	sb.AddSnake(6, 8, {SnakeBird::kLeft, SnakeBird::kLeft, SnakeBird::kDown, SnakeBird::kDown});
	//		sb.AddSnake(10, 10, {SnakeBird::kRight});
	//		sb.AddSnake(15, 10, {SnakeBird::kRight});
	//		sb.AddSnake(20, 10, {SnakeBird::kRight});
	
	sb.SetGroundType(5, 10, SnakeBird::kBlock1);
	sb.SetGroundType(6, 10, SnakeBird::kBlock1);
	sb.SetGroundType(5, 9, SnakeBird::kBlock2);
	sb.SetGroundType(6, 9, SnakeBird::kBlock2);
		
	snake = sb.GetStart();
	history.clear();
	history.push_back(snake);
}

void LoadLevel32()
{
	for (int x = 5; x <= 10; x++)
		sb.SetGroundType(x, 9, SnakeBird::kGround);
	for (int x = 11; x <= 12; x++)
		sb.SetGroundType(x, 9, SnakeBird::kSpikes);
	for (int x = 12; x <= 14; x++)
		sb.SetGroundType(x, 7, SnakeBird::kGround);
	for (int y = 5; y <= 7; y++)
		sb.SetGroundType(8, y, SnakeBird::kGround);
	for (int y = 4; y <= 5; y++)
		sb.SetGroundType(13, y, SnakeBird::kGround);

	sb.SetGroundType(6, 6, SnakeBird::kExit);
	
	sb.AddSnake(9, 7, {SnakeBird::kRight});
	sb.AddSnake(10, 8, {SnakeBird::kLeft});

	sb.SetGroundType(12, 6, SnakeBird::kBlock1);

	sb.SetGroundType(8, 8, SnakeBird::kPortal1);
	sb.SetGroundType(13, 6, SnakeBird::kPortal2);

	snake = sb.GetStart();
	history.clear();
	history.push_back(snake);
}

void LoadLevel63()
{
	for (int x = 5; x <= 11; x++)
		sb.SetGroundType(x, 6, SnakeBird::kGround);
	for (int x = 13; x <= 13; x++)
		sb.SetGroundType(x, 6, SnakeBird::kGround);
	for (int x = 14; x <= 15; x++)
		sb.SetGroundType(x, 5, SnakeBird::kGround);
	for (int x = 13; x <= 15; x++)
		sb.SetGroundType(x, 4, SnakeBird::kGround);

	sb.SetGroundType(14, 2, SnakeBird::kExit);
	
	sb.AddSnake(10, 5, {SnakeBird::kRight});

	sb.SetGroundType(10, 4, SnakeBird::kBlock1);
	sb.SetGroundType(9, 3, SnakeBird::kBlock1);
	sb.SetGroundType(10, 3, SnakeBird::kBlock1);
	sb.SetGroundType(11, 3, SnakeBird::kBlock1);

	sb.SetGroundType(6, 3, SnakeBird::kPortal1);
	sb.SetGroundType(13, 5, SnakeBird::kPortal2);

	snake = sb.GetStart();
	history.clear();
	history.push_back(snake);
}

void MyWindowHandler(unsigned long windowID, tWindowEventType eType)
{
	if (eType == kWindowDestroyed)
	{
		printf("Window %ld destroyed\n", windowID);
		RemoveFrameHandler(MyFrameHandler, windowID, 0);
	}
	else if (eType == kWindowCreated)
	{
		printf("Window %ld created\n", windowID);
		InstallFrameHandler(MyFrameHandler, windowID, 0);
		SetNumPorts(windowID, 1);
		if (mapName.size() != 0)
		{
			sb.Load(mapName.c_str());
			snake = sb.GetStart();
		}
		else {
			//LoadLevel19();
			//LoadLevel22();
			//LoadLevel39();
			//LoadLevel63();
			LoadLevel32();
		}
		
	}
}


void MyFrameHandler(unsigned long windowID, unsigned int viewport, void *)
{
	Graphics::Display &d = GetContext(windowID)->display;
	sb.Draw(d);
	sb.Draw(d, snake, snakeControl);
//	iws.IncrementTime();
//	w.Draw(d);
//	w.Draw(d, iws);
}

int MyCLHandler(char *argument[], int maxNumArgs)
{
	if (strcmp(argument[0], "-load") == 0)
	{
		if (maxNumArgs > 1)
		{
			mapName = argument[1];
			return 2;
		}
		printf("Failed -load <file>: missing file name");
		return 1;
		// "/home/aaron/Desktop/SnakeBird_lvls/Snakebird_lvl_20"
	}
	return 0;
}

void MyDisplayHandler(unsigned long windowID, tKeyboardModifier mod, char key)
{
//	SnakeBird::SnakeBirdAction a;
	std::vector<SnakeBird::SnakeBirdAction> acts;
//	a.bird = snakeControl;
	switch (key)
	{
		case 'w':
			sb.GetActions(snake, acts);
			for (auto &a : acts)
			{
				if (a.bird == snakeControl && a.direction == SnakeBird::kUp)
				{
					sb.ApplyAction(snake, a);
					history.push_back(snake);
				}
			}
			// If snake went into goal, switch active snake
			for (int x = 0; x < snake.GetNumSnakes(); x++)
			{
				if (snake.IsInPlay(snakeControl))
					break;
				snakeControl = (snakeControl+1)%snake.GetNumSnakes();
			}

//			a.direction = SnakeBird::kUp;
//			if (sb.Legal(snake, a))
//			{
//				sb.ApplyAction(snake, a);
//				history.push_back(snake);
//			}
			break;
		case 's':
			sb.GetActions(snake, acts);
			for (auto &a : acts)
			{
				if (a.bird == snakeControl && a.direction == SnakeBird::kDown)
				{
					sb.ApplyAction(snake, a);
					history.push_back(snake);
				}
			}
			// If snake went into goal, switch active snake
			for (int x = 0; x < snake.GetNumSnakes(); x++)
			{
				if (snake.IsInPlay(snakeControl))
					break;
				snakeControl = (snakeControl+1)%snake.GetNumSnakes();
			}
//			a.direction = SnakeBird::kDown;
//			if (sb.Legal(snake, a))
//			{
//				sb.ApplyAction(snake, a);
//				history.push_back(snake);
//			}
			break;
		case 'a':
			sb.GetActions(snake, acts);
			for (auto &a : acts)
			{
				if (a.bird == snakeControl && a.direction == SnakeBird::kLeft)
				{
					sb.ApplyAction(snake, a);
					history.push_back(snake);
				}
			}
			// If snake went into goal, switch active snake
			for (int x = 0; x < snake.GetNumSnakes(); x++)
			{
				if (snake.IsInPlay(snakeControl))
					break;
				snakeControl = (snakeControl+1)%snake.GetNumSnakes();
			}

//			a.direction = SnakeBird::kLeft;
//			if (sb.Legal(snake, a))
//			{
//				sb.ApplyAction(snake, a);
//				history.push_back(snake);
//			}
			break;
		case 'd':
			sb.GetActions(snake, acts);
			for (auto &a : acts)
			{
				if (a.bird == snakeControl && a.direction == SnakeBird::kRight)
				{
					sb.ApplyAction(snake, a);
					history.push_back(snake);
				}
			}
			// If snake went into goal, switch active snake
			for (int x = 0; x < snake.GetNumSnakes(); x++)
			{
				if (snake.IsInPlay(snakeControl))
					break;
				snakeControl = (snakeControl+1)%snake.GetNumSnakes();
			}

//			a.direction = SnakeBird::kRight;
//			if (sb.Legal(snake, a))
//			{
//				sb.ApplyAction(snake, a);
//				history.push_back(snake);
//			}
			break;
		case 'q':
			if (history.size() > 1)
				history.pop_back();
			snake = history.back();
			break;
		case 'e':
			for (int x = 0; x < snake.GetNumSnakes(); x++)
			{
				snakeControl = (snakeControl+1)%snake.GetNumSnakes();
				if (snake.IsInPlay(snakeControl))
					break;
			}
			break;
		case 'p':
		{
			Graphics::Display d;
			sb.Draw(d);
			sb.Draw(d, snake, snakeControl);
			std::string fname = "/Users/nathanst/Desktop/SVG/sb_";
			int count = 0;
			while (fileExists((fname+std::to_string(count)+".svg").c_str()))
			{
				count++;
			}
			printf("Save to '%s'\n", (fname+std::to_string(count)+".svg").c_str());
			MakeSVG(d, (fname+std::to_string(count)+".svg").c_str(), 400, 400, 0);
		}
		case 't':
			break;
		case 'v':
		{
		}
			break;
//		case 's':
		{
//			Graphics::Display d;
//			//d.FillRect({-1, -1, 1, 1}, Colors::darkgray);
//			w.Draw(d);
//			w.Draw(d, iws);
//			std::string fname = "/Users/nathanst/Desktop/SVG/witness_";
//			int count = 0;
//			while (fileExists((fname+std::to_string(count)+".svg").c_str()))
//			{
//				count++;
//			}
//			printf("Save to '%s'\n", (fname+std::to_string(count)+".svg").c_str());
//			MakeSVG(d, (fname+std::to_string(count)+".svg").c_str(), 400, 400, 0, w.SaveToHashString().c_str());
//
//			{
//				int wide, high;
//				w.GetDimensionsFromHashString(w.SaveToHashString(), wide, high);
//			}
		}
			break;
		case 'r':
			snake = history[0];
			history.resize(1);
			break;
		case '\t':
			if (mod != kShiftDown)
				SetActivePort(windowID, (GetActivePort(windowID)+1)%GetNumPorts(windowID));
			else
			{
				SetNumPorts(windowID, 1+(GetNumPorts(windowID)%MAXPORTS));
			}
			break;
		case '[':
			break;
		case ']':
			break;
		case '{':
			break;
		case '}':
			break;

		case 'o':
			break;
		default:
			break;
	}
}

bool MyClickHandler(unsigned long, int, int, point3d p, tButtonType , tMouseEventType e)
{
	if (e == kMouseDrag) // ignore movement with mouse button down
		return true;
	
	if (e == kMouseUp)
	{
	}
	if (e == kMouseMove)
	{
	}

	// Don't need any other mouse support
	return true;
}

