//
// Created by User on 2/14/2023.
//


#include <iostream>
#include "PancakeExperiment.h"
#include "PancakePuzzle.h"
#include "TemplateAStar.h"

const int N = 12;

void TestPancake(int gap, float weight, int instance) {
    printf("GAP,Instance,Stack,Weight,Version,Path Length,Expanded,Necessary,Elapsed,Reopens,Generated\n");
    PancakePuzzle<N> pancake(gap);
    PancakePuzzleState<N> start;
    PancakePuzzleState<N> goal;
    goal.Reset();
    std::vector<PancakePuzzleState<N>> path;
    Timer timer;
    srandom(instance);
    std::vector<int> perm = PermutationPuzzle::PermutationPuzzleEnvironment<PancakePuzzleState<N>, PancakePuzzleAction>::Get_Random_Permutation(
            N);

    // construct puzzle
    for (unsigned i = 0; i < N; i++) {
        start.puzzle[i] = perm[i];
    }


    float weights[13] = {1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2, 3, 4, 5};
    for (int version = 0; version < 3; version++) {
        TemplateAStar<PancakePuzzleState<N>, PancakePuzzleAction, PancakePuzzle<N>> astar;
        astar.SetWeight(weight);
        astar.setVersion(version);
        astar.SetReopenNodes(true);
        timer.StartTimer();
        astar.GetPath(&pancake, start, goal, path);
        timer.EndTimer();
        std::cout << gap << "," << instance << "," << start << "," << weight << "," << version << ","
                  << pancake.GetPathLength(path) << "," << astar.GetNodesExpanded() << ","
                  << astar.GetNecessaryExpansions() << "," << timer.GetElapsedTime() << ","
                  << astar.getReopenCount() << "," << astar.GetNodesTouched() << std::endl;
    }


}