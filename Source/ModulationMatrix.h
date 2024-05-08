#pragma once

#include "Oscillator.h"  // Include the Oscillator header if it's not included globally
#include <vector>
#include <queue>

class ModulationMatrix {
public:
    ModulationMatrix() = default;
    ModulationMatrix(std::vector<Oscillator*>& oscillators);
    bool setModulation(int carrierIdx, int modulatorIdx, float modulationDepth);
    bool removeModulation(int carrierIdx, int modulatorIdx);
    float process();
    bool isCyclic();
    void setOutput(int index);
    void setLevel(float newLevel);

private:
    float level;
    std::vector<Oscillator*> oscillators;
    std::vector<std::vector<float>> modulationDepths;
    std::vector<std::vector<bool>> connections;

    bool dfs(int v, std::vector<int>& visited);
    std::vector<int> topologicalSort();
};
