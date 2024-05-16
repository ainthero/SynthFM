#include "ModulationMatrix.h"
#include <stdexcept>
#include <iostream>
#include <queue>

ModulationMatrix::ModulationMatrix(std::vector<Oscillator*>& oscillators)
    : oscillators(oscillators), modulationDepths(4, std::vector<float>(4, 0.0)), connections(4, std::vector<bool>(4, false)) {
    if (oscillators.size() != 4) {
        throw std::invalid_argument("There must be exactly four oscillators.");
    }
}

bool ModulationMatrix::setModulation(int modulatorIdx, int carrierIdx, float modulationDepth) {
    if (carrierIdx == modulatorIdx) {
        throw std::invalid_argument("An oscillator cannot modulate itself.");
    }
    modulationDepths[modulatorIdx][carrierIdx] = modulationDepth;
    connections[modulatorIdx][carrierIdx] = true;
    if (isCyclic()) {
        connections[modulatorIdx][carrierIdx] = false;
        modulationDepths[modulatorIdx][carrierIdx] = 0.0;
        return false;
    }
    return true;
}

bool ModulationMatrix::removeModulation(int modulatorIdx, int carrierIdx) {
    modulationDepths[modulatorIdx][carrierIdx] = 0.0;
    connections[modulatorIdx][carrierIdx] = false;
    return true;
}

float ModulationMatrix::process() {
    // Получаем порядок обработки осцилляторов с помощью топологической сортировки
    std::vector<int> order = topologicalSort();
    std::vector<float> outputs(4, 0.0);
    std::vector<float> originalFrequencies(4);
    // Сохраняем исходные частоты каждого осциллятора
    for (int i = 0; i < 4; ++i) {
        originalFrequencies[i] = oscillators[i]->getFrequency();
    }
    // Обходим осцилляторы в порядке, определенном топологической сортировкой
    for (int modulatorIdx : order) {
        // Получаем следующий сэмпл от текущего осциллятора (модулятора)
        outputs[modulatorIdx] = oscillators[modulatorIdx]->nextSample();
        // Проходим по всем возможным осцилляторам-носителям
        for (int carrierIdx = 0; carrierIdx < 4; ++carrierIdx) {
            // Проверяем, существует ли связь модуляции между текущими осцилляторами
            if (connections[modulatorIdx][carrierIdx]) {
                float modulationEffect = outputs[modulatorIdx] * modulationDepths[modulatorIdx][carrierIdx];
                // Применяем модуляцию, изменяя частоту осциллятора-носителя
                oscillators[carrierIdx]->setFrequency(oscillators[carrierIdx]->getFrequency() + modulationEffect);
            }
        }
    }
    // Вычисляем итоговый результат, суммируя выходные значения, умноженные на уровень каждого осциллятора
    float res = 0.0;
    for (int idx = 0; idx < 4; ++idx) {
        res += outputs[idx] * oscillators[idx]->getLevel();
        // Восстанавливаем исходные частоты осцилляторов после обработки
        oscillators[idx]->setFrequency(originalFrequencies[idx]);
    }
    return res * level;
}



void ModulationMatrix::setLevel(float newLevel) {
    level = newLevel;
}

bool ModulationMatrix::isCyclic() {
    std::vector<int> visited(4, 0);
    for (int i = 0; i < 4; ++i) {
        if (visited[i] == 0 && dfs(i, visited)) {
            return true;
        }
    }
    return false;
}

bool ModulationMatrix::dfs(int v, std::vector<int>& visited) {
    if (visited[v] == 1) return true;
    if (visited[v] == 2) return false;

    visited[v] = 1;
    for (int i = 0; i < 4; ++i) {
        if (connections[v][i] && dfs(i, visited)) {
            return true;
        }
    }
    visited[v] = 2;
    return false;
}

std::vector<int> ModulationMatrix::topologicalSort() {
    std::vector<int> inDegree(4, 0);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (connections[i][j]) inDegree[j]++;
        }
    }

    std::queue<int> q;
    for (int i = 0; i < 4; ++i) {
        if (inDegree[i] == 0) q.push(i);
    }

    std::vector<int> result;
    while (!q.empty()) {
        int u = q.front(); q.pop();
        result.push_back(u);
        for (int i = 0; i < 4; ++i) {
            if (connections[u][i] && (--inDegree[i] == 0)) {
                q.push(i);
            }
        }
    }

    if (result.size() != 4) throw std::logic_error("Cyclic dependency detected, cannot perform topological sort");

    return result;
}
