#include "Common.h"
#include "Driver.h"
#include "Map2DEnvironment.h"
#include "CanonicalGrid.h"
#include "TemplateAStar.h"
#include "TextOverlay.h"
#include "MapOverlay.h"
#include <string>
#include "NBS.h"
#include "fMM.h"
#include "SVGUtil.h"

enum mode {
	kFindPath = 1,
	kDrawObstacles = 2
};

enum drawing {
	kDrawWall,
	kDrawGround,
	kDrawSwamp
};

int whichAlgorithm = 0;
int numAlgorithms = 3;

void BGMap2(Map *theMap);

MapEnvironment *me = 0;
TemplateAStar<xyLoc, tDirection, MapEnvironment> AStarForward, AStarBackward;
//fMM<xyLoc, tDirection, MapEnvironment> fmm;
NBS<xyLoc, tDirection, MapEnvironment> nbs;
std::vector<xyLoc> path;

xyLoc start, goal;
bool mapChange = true;

mode m = kFindPath;
drawing d = kDrawGround;

bool recording = false;
bool running = false;
int stepsPerFrame = 1;

int main(int argc, char* argv[])
{
	InstallHandlers();
	RunHOGGUI(argc, argv, 1200, 1200);
	return 0;
}

/**
 * Allows you to install any keyboard handlers needed for program interaction.
 */
void InstallHandlers()
{
	InstallKeyboardHandler(MyDisplayHandler, "Draw", "Draw Obstacles", kAnyModifier, 'd');
	InstallKeyboardHandler(MyDisplayHandler, "Path", "Find Path", kAnyModifier, 'p');
	InstallKeyboardHandler(MyDisplayHandler, "Draw Walls", "Draw Wall Obstacles", kAnyModifier, '1');
	InstallKeyboardHandler(MyDisplayHandler, "Draw Ground", "Draw Open Ground", kAnyModifier, '2');
	InstallKeyboardHandler(MyDisplayHandler, "Draw Swamp", "Draw Slow Ground", kAnyModifier, '3');

	InstallKeyboardHandler(MyDisplayHandler, "Faster", "Double steps per frame", kAnyModifier, ']');
	InstallKeyboardHandler(MyDisplayHandler, "Slower", "Half steps per frame", kAnyModifier, '[');
	InstallKeyboardHandler(MyDisplayHandler, "Rotate", "Show different algorithm", kAnyModifier, '=');

	InstallKeyboardHandler(MyDisplayHandler, "Record", "Toggle SVG recording", kAnyModifier, 'r');

	InstallWindowHandler(MyWindowHandler);
	InstallMouseClickHandler(MyClickHandler);
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
		InstallFrameHandler(MyFrameHandler, windowID, 0);
		SetNumPorts(windowID, 1);
		
		Map *map = new Map(1,1);
		BGMap2(map);
		
		submitTextToBuffer("Drag to choose a start/goal");
		map->SetTileSet(kWinter);
		me = new MapEnvironment(map);
		me->SetDiagonalCost(1.5);
		start.x = start.y = 0xFFFF;
		printf("Done creating window\n");
	}
}

int frameCnt = 0;

void MyFrameHandler(unsigned long windowID, unsigned int viewport, void *)
{
	Graphics::Display &display = getCurrentContext()->display;

	if (mapChange)
	{
		display.StartBackground();
		me->Draw(display);
		display.EndBackground();
		mapChange = false;
	}

	if (!(start.x == 0xFFFF || running))
	{
		//me->Draw(display, start);
		me->SetColor(1, 0, 0);
		me->Draw(display, start);
		me->SetColor(0, 0, 1);
		me->Draw(display, goal);
		me->SetColor(0.5, 0.5, 0.5);
		me->DrawArrow(display, start, goal, 3);
	}
	
	if (running)
	{
		if (path.size() == 0 || whichAlgorithm == 0)
			nbs.Draw(display);
		else if (whichAlgorithm == 1)
			AStarForward.Draw(display);
		else if (whichAlgorithm == 2)
			AStarBackward.Draw(display);
		for (int x = 0; x < stepsPerFrame; x++)
			if (path.size() == 0)
			{
				nbs.DoSingleSearchStep(path);
				if (path.size() != 0)
				{
					char tmp[255];
					sprintf(tmp, "A*f %" PRId64 ", A*b %" PRId64 ", A*avg %" PRId64 ", NBS %" PRId64 "\n", AStarForward.GetNodesExpanded(), AStarBackward.GetNodesExpanded(),
							(AStarForward.GetNodesExpanded()+AStarBackward.GetNodesExpanded())/2, nbs.GetNodesExpanded());
					submitTextToBuffer(tmp);
				}
			}

		if (path.size() != 0) {
			me->SetColor(0, 0, 1);
			for (int x = 1; x < path.size(); x++)
			{
				me->DrawLine(display, path[x-1], path[x], 3);
			}
		}
	}

	if (recording && viewport == GetNumPorts(windowID)-1)
	{
		char fname[255];
		sprintf(fname, "/Users/nathanst/Movies/tmp/NBS-%d%d%d%d.svg",
				(frameCnt/1000)%10, (frameCnt/100)%10, (frameCnt/10)%10, frameCnt%10);
		
		MakeSVG(display, fname, 800, 800);
		
		printf("Saved %s\n", fname);
		frameCnt++;
//		if (path.size() == 0)
//		{
//			MyDisplayHandler(windowID, kNoModifier, 'o');
//		}
//		else {
//			recording = false;
//		}
	}
}

int MyCLHandler(char *argument[], int maxNumArgs)
{
	if (maxNumArgs <= 1)
		return 0;
	strncpy(gDefaultMap, argument[1], 1024);
	return 2;
}

void MyDisplayHandler(unsigned long windowID, tKeyboardModifier mod, char key)
{
	switch (key)
	{
		case 'o':
			nbs.DoSingleSearchStep(path);
			break;
		case 'r':
			recording = !recording;
			break;
		case 'd':
			start = goal = xyLoc();
			m = kDrawObstacles;
			running = false;
			submitTextToBuffer("Mode: Draw Obstacles");
			break;
		case 'p':
			start = goal = xyLoc();
			m = kFindPath;
			running = false;
			submitTextToBuffer("Click and drag to set path");
			break;
		case '1': if (m != kDrawObstacles) break; d = kDrawWall; submitTextToBuffer("Drawing walls"); break;
		case '2': if (m != kDrawObstacles) break;d = kDrawGround; submitTextToBuffer("Drawing ground"); break;
		case '3': if (m != kDrawObstacles) break;d = kDrawSwamp; submitTextToBuffer("Drawing swamp"); break;
		case '=': whichAlgorithm = (whichAlgorithm+1)%numAlgorithms; break;
		case '[':
			stepsPerFrame /= 2;
			break;
		case ']':
			stepsPerFrame *= 2;
			if (stepsPerFrame == 0)
				stepsPerFrame = 1;
			break;
			
	}
	
}

void SetStartGoalHandler(uint16_t x, uint16_t y, tMouseEventType mType)
{
	static bool currentlyDrawing = false;

	switch (mType)
	{
		case kMouseDown:
		{
			currentlyDrawing = true;
			running = false;
			if ((me->GetMap()->GetTerrainType(x, y)&kGround) == kGround)
			{
				start.x = x;
				start.y = y;
				goal = start;
				printf("Hit (%d, %d)\n", x, y);
			}
		}
		case kMouseDrag:
		{
			if ((me->GetMap()->GetTerrainType(x, y)&kGround) == kGround && currentlyDrawing)
			{
				//printf("drag (%d, %d)\n", x, y);
				goal.x = x;
				goal.y = y;
			}
			break;
		}
		case kMouseUp:
		{
			currentlyDrawing = false;
			if ((me->GetMap()->GetTerrainType(x, y)&kGround) == kGround)
			{
				goal.x = x;
				goal.y = y;
				AStarForward.GetPath(me, start, goal, path);
				AStarBackward.GetPath(me, goal, start, path);
				nbs.InitializeSearch(me, start, goal, me, me, path);
				running = true;
			}
		}
	}
}


void DrawHandler(uint16_t x, uint16_t y, tMouseEventType mType)
{
	static bool currentlyDrawing = false;
	static int lastx = 0, lasty = 0;
	if (mType == kMouseUp)
	{
		currentlyDrawing = false;
		return;
	}
	if (me->GetMap()->GetTerrainType(x, y) == kOutOfBounds)
		return;
	if (mType == kMouseDown)
	{
		currentlyDrawing = true;
		switch (d)
		{
			case kDrawGround: me->GetMap()->SetTerrainType(x, y, kGround); break;
			case kDrawWall: me->GetMap()->SetTerrainType(x, y, kTrees); break;
			case kDrawSwamp: me->GetMap()->SetTerrainType(x, y, kSwamp); break;
		}
	}
	else if (mType == kMouseDrag && currentlyDrawing)
	{
		switch (d)
		{
			case kDrawGround: me->GetMap()->SetTerrainType(x, y, kGround); break;
			case kDrawWall: me->GetMap()->SetTerrainType(x, y, kTrees); break;
			case kDrawSwamp: me->GetMap()->SetTerrainType(x, y, kSwamp); break;
		}
	}
	lastx = x; lasty = y;
}


bool MyClickHandler(unsigned long , int windowX, int windowY, point3d loc, tButtonType button, tMouseEventType mType)
{
	if (button != kLeftButton)
		return false;
	int x, y;
	me->GetMap()->GetPointFromCoordinate(loc, x, y);
	switch (m)
	{
		case kFindPath: SetStartGoalHandler(x, y, mType); break;
		case kDrawObstacles: DrawHandler(x, y, mType); break;
	}
	return true;
}
	

void BGMap2(Map *theMap)
{
	theMap->Scale(148, 139);
	const char map[] = "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@..@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.....@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.......@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@..........@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@..............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@....@@..........@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...@@@@..........@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...@@@@............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...@@@@....@@@.......@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...@@@@@@....@@.........@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@....@@@@@@@@................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.....@@@@@@@@.@..............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.......@.@@@@..@@...............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@....@@@...@@@..@@@................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@....@@@....@..@@@.................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...............@@@....................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@..............@@@.....................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...........@@@..@.......................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...........@@@@...........................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...........@@@........@@.................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@....................@@@@...............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...................@@@@@.............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.................@@@@@@............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@..@@@@@@.@@@@@@@...............@@@@@@@@..........@@@@@..@@.@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@..@@@.......@@@@@.............@@@@@@@@..........@@@@@...@@.@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@..@@.........@@@@@...........@@@@@@@@........@@@@@@@........@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@..............@@@@..........@@@@@@@@.........@@@@@@@.........@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...............@@@..........@@@@@@@.........@@@@@@...........@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...............@@@@.........@@@@@@@.........@@@@@@.............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...............@@@@..........@@@@@@.........@@@@@@.....@@.........@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@..............@@@.............@..@.........@@@@@@.....@@@@.........@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@..............@@@..........................@@@@@@.....@@@@@..........@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...............@@@.........................@@@@@@......@@@@.............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@..............@@@..........................@@@@@@.....@@@@...............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...............@@@..........................@@@@@@.....@@@@.................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...............@@..........................@@@@@@.....@@@@...................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@........................@@.................@@@@@@.....@@@@...@@@...............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.............@@@.........@@@@...............@@@@@@@....@@@@...@@@@@......@........@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@............@@@@@.......@@@@@@.............@@@@@......@@@@...@@@@@@.@...@@@........@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@............@@@@@.......@@@@@@@............@@@@@......@@@@...@@@@@@@@@..@@@@.........@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@..........@@@@@.......@@@@@@@@............@@@@......@@@@....@@@@@@@@..@@@@...........@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.........@@@@@.......@@@@@@@@@.............@@@.......@@.....@@@@@@@..@@@@..............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.......@@@@@.......@@@@@@@@................@@................@@.@..@@@@...............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.....@@@@@.......@@@@@@@@..........@.............................@@@@..................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...@@@@@........@@@@@@@..........@@@...........................@@@@..@@...............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...@@@@@@@..@@@@.........@@@@@@..........@@@@@.........................@@@@..@@@@......@........@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@....@@@@@@@@@@@..........@@@@@..........@@@@@@........................@@@@..@@@@@@....@@@........@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@......@@@@@@@@............@@@..........@@@@@@@.......................@@@@..@@@@@@@...@@@@.........@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@......@@@@@@.........................@@@@@@........................@@@@..@@@@@@@...@@@@...........@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.......@@@@@@@......................@@@@@@....................@@...@@@....@@@@@...@@@@.............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.........@@@@@@.....................@@@@@@....................@@@@...@......@@@...@@@@................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@..@@@@.....@.....@@@@@@...................@@@@@@....................@@@@@@..............@@@@..................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@....@@.....@@@.....@@@@@..................@@@@@@@........@.@@.......@@@..@@@............@@@@..@@@................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.........@@@@@.....@@@@.................@@@@@..........@@@@@@.....@@@....@@@..........@@@@@@@@@@@................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@........@@@@@@.....@..................@@@@@...........@@@@@@....@@@......@@@........@@@@@@@@@@@@.......@.........@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@........@@@@@@........@@@...........@@@@@...........@@@@@@....@@@........@@@......@@@@.@@@@@@@@@.....@@@.........@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@........@@@@@........@@@@.........@@@@@@............@@@@....@@@..........@@@....@@@@..@.@@@@@@@....@@@@..........@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@........@@@..@......@@@@@.......@@@@@@@.............@@....@@@............@@@...@@@.....@@@@@@....@@@@............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@........@..@@@.......@@@@.....@@@@@@@@@.................@@@..............@@....@.......@@@@....@@@@.............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.........@@@@@.......@@@@...@@@@@@@@...................@@...............@@@............@@....@@@@.............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@........@@@@@@.......@@@@.@@@@@@@@...................@@.........@@......@@@................@@@@.............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.........@@@@.......@@@@@@@@@@@@...................@@@.........@@@.....@@@...............@@@@.............@@@@@@@.@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.........@@.......@@@@@@@@@@@@......@@...........@@@@..........@@......@...............@@@@.............@@@@@@....@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@................@@@@@@@@@@@@......@@@@.........@@@@@@.........@@.....................@@@@.............@@@@@@@...@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@..............@@@@@@@@@@@@......@@@@@@........@@@@@@@........@@@.....@@............@@@@.............@@@@@@@@...@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@............@@@@@@@@@@@@@.....@@@@@@@@@......@@@@@@@@@@....@@@@.....@@...........@@@@.............@@@@@@@@@....@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@..........@@@@@@@@@@@@@@....@@@@@@@@@@@......@@@@@@@@@@@@@@..@@.................@@@.............@@@@@@@@.......@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@........@@@@@@@@@@@@....@@@@@@@@@@@@@@@S.....@@@@@@@@@@.....@@..................@.............@@@@@@@@.........@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@......@@@@@@@@@@@@.....@@@@@@@@@@@@@@@SS.....@@@@@@@@.......@...............................@@@@@@@@@..........@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@....@@@@@@@@@@@@......@@@@@@@@@@@@@@SSS.......@@@@@.......................................@@@@@@@@.............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@..@@@@@@@@@.@@........@@@@@@@@@@@@SSSS........@@@.......................................@@@@@@@@...............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@............@@@@@@@@@@SSSSS@@.......@.......................................@@@@@@@@.................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@............@@@@@@@@@SSSSS@@@@@............................................@@@@@@@@...................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...............@@@@@@SSSSSS@@@@@@@..........................................@@@@@@@@.....................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...............@@@@@@@SSSSS@@@@@@@@@........................................@@@@@@@@..........@............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...............@@@@@@@@SSSS@@@@@@@@@@@......................................@@@@@@@@..........@@@............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...............@@@@@@@@@SSSS@@@@@@@@@@@@S...................................@@@@@@@@..........@@@@@............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.............@@@@@@@..SSS@@@@@@@@@@@@@@@SS................................@@@@@@@@............@@@@...........@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...........@@@@@@@...SS@@@@@@@@@@@@@@@SSS...............................@@@@@@@@..............@@...........@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.........@@@@@@@@.....S@@@@@@@@@@@@SSSS................................@@@@@@@...........................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@......@@@@@@...........@@@@@@@@@@SSSSS@@...............................@@.@@@.....@....................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@....@@@@@@@...........@@@@@@@@@SSSSS@@@@...............................@........@@@..................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@..@@@@@@@.............@@@@@@@SSSSS@@@@@@......................................@@@@@................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@..............@@@@@@SSSSS@@@@@@@@......................................@@@@@..............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...............@@@@@@@SSSS@@@@@@@@@@......................................@@@@@............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...............@@@@@@@@SSS@@@@@@@@@@@@......................................@@@@@..........@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...............@@@@@@@SSSS@@@@@@@@@@@@@@......................................@@@@@........@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@............@@@@@@@......@@@@@@@@@@@@@@................................@......@@@@@......@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@..........@@@@@@@@.....@@@@@@@@@@@@@@.............@...................@@......@@@@@....@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.......@@@@@@.@@@......@@@@@@@@@@@@.............@@@...................@@......@@@@@..@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.....@@@@@@@...........@@@@@@@@@@.............@@@@@...................@@......@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...@@@@@@@@...........@@@@@@@@@.............@@@@@@...........@@.......@.......@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.@@@@@@@.............@@@@@@@@.............@@@@@@@..........@@@@..............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.............@@@@@@@@.............@@@@@@............@@@@.............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.............@@@@@@@@@............@@@@@@..............@@.............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@..............@@@@@@@@@@...........@@@@@@@............................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.............@@@@@@@@.....@.......@@@@@@@@...........................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...........@@@@@@@@......@@.....@@@@@@@@...........@@..............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.........@@@@@@@@.......@@@...@@@@@@@@.............@@..............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.......@@@@@@...........@@@@@@@@@@@@...............@@..............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@....@@@@@@@@...........@@@@@@@@@@@.......@@........@@.............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@..@@@@@@@@............@@@@@@@@@@.......@@@@........@............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...........@@@@@@@@@@@........@@@@...................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...........@@@@@@@@@............@@@@.................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...........@@@@@@@@@..............@@@@...............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...........@@@@@@@@@................@@@@.............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@............@@@@@@@@...................@@@@......@@@@.@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@..........@@@@@@@@.....................@@@@.....@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@........@@@@@@@@.......................@@@@...@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@......@@@@@@@@.........................@@@@.@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@....@@@@@@@@@..........................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...@@@@@.@@...........@@@..............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.@@@@@..............@@@@@.............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@...............@@@@............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.................@@.............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@................................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@..............................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@............................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@..........................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@........................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@......................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@....................@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";
	int which = 0;
	for (int y = 0; y < theMap->GetMapHeight(); y++)
	{
		for (int x = 0; x < theMap->GetMapWidth(); x++)
		{
			if (map[which] == '.')
				theMap->SetTerrainType(x, y, kGround);
			else if (map[which] == 'S')
				theMap->SetTerrainType(x, y, kSwamp);
			else
				theMap->SetTerrainType(x, y, kOutOfBounds);
			which++;
		}
	}
	mapChange = true;
}
	
	
