/*
 * scenarioLoader.h
 * hog
 *
 * Created by Renee Jansen on 5/2/2006
 *
 */ 

#ifndef SCENARIOLOADER_H
#define SCENARIOLOADER_H

#include <vector>
#include <string>
using std::string;

static const int kNoScaling = -1;

/** 
 * Experiments stored by the ScenarioLoader class. 
 */

class Experiment{
public:
  Experiment(int sx,int sy,int gx,int gy,int b, double d, string m)
    :startx(sx),starty(sy),goalx(gx),goaly(gy),scaleX(kNoScaling),scaleY(kNoScaling),bucket(b),distance(d),map(m){}
  Experiment(int sx,int sy,int gx,int gy,int sizeX, int sizeY,int b, double d, string m)
    :startx(sx),starty(sy),goalx(gx),goaly(gy),scaleX(sizeX),scaleY(sizeY),bucket(b),distance(d),map(m){}
  int GetStartX(){return startx;}
  int GetStartY(){return starty;}
  int GetGoalX(){return goalx;}
  int GetGoalY(){return goaly;}
  int GetBucket(){return bucket;}
  double GetDistance(){return distance;}
  void GetMapName(char* mymap){strcpy(mymap,map.c_str());}
	const char *GetMapName() { return map.c_str(); }
  int GetXScale(){return scaleX;}
  int GetYScale(){return scaleY;}

private:
  int startx, starty, goalx, goaly;
  int scaleX;
  int scaleY;
  int bucket;
  double distance;
  string map;
};

/** A class which loads and stores scenarios from files.  
 * Versions currently handled: 0.0 and 1.0 (includes scale). 
 */

class ScenarioLoader{
public:
  ScenarioLoader(const char *);
  int GetNumExperiments(){return experiments.size();}
	const char *GetScenarioName() { return scenName; }
  Experiment GetNthExperiment(int which){return experiments[which];}

private:
		char scenName[1024];
  std::vector<Experiment> experiments;
};

#endif