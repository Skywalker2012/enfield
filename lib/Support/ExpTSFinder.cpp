#include "enfield/Support/ExpTSFinder.h"

#include <algorithm>
#include <queue>
#include <cassert>

void efd::ExpTSFinder::genAllAssigns(uint32_t n) {
    efd::Assign assign(n, 0);
    for (uint32_t i = 0; i < n; ++i) assign[i] = i;

    mAssigns.clear();

    do {
        mAssigns.push_back(assign);
    } while (std::next_permutation(assign.begin(), assign.end()));
}

uint32_t efd::ExpTSFinder::getTargetId(Assign source, Assign target) {
    assert(source.size() == target.size() && "The assignment map must be of same size.");

    int size = source.size();

    Assign translator(size, 0);
    Assign realtgt(size, 0);

    for (int i = 0; i < size; ++i) {
        translator[source[i]] = i;
    }

    for (int i = 0; i < size; ++i) {
        realtgt[i] = translator[target[i]];
    }

    return mMapId[realtgt];
}

// Pre-process the architechture graph, calculating the optimal swaps from every
// permutation.
void efd::ExpTSFinder::preprocess(Graph::Ref graph) {
    uint32_t size = graph->size();
    genAllAssigns(size);

    mMapId.clear();
    for (uint32_t i = 0, e = mAssigns.size(); i < e; ++i) {
        mMapId.insert(std::make_pair(mAssigns[i], i));
    }

    std::vector<bool> inserted(mAssigns.size(), false);
    mSwaps.assign(mAssigns.size(), SwapSeq());

    std::vector<uint32_t> cur;
    std::queue<uint32_t> q;

    // Initial permutation [0, 1, 2, 3, 4]
    q.push(0);
    inserted[0] = true;
    while (!q.empty()) {
        auto aId = q.front();
        q.pop();

        auto cur = mAssigns[aId];

        for (uint32_t u = 0; u < size; ++u) {
            for (uint32_t v : graph->succ(u)) {
                auto copy = cur;
                std::swap(copy[u], copy[v]);

                int cId = mMapId[copy];
                if (!inserted[cId]) {
                    inserted[cId] = true;
                    mSwaps[cId] = mSwaps[aId];
                    mSwaps[cId].push_back(Swap { u, v });
                    q.push(cId);
                }
            }

            for (uint32_t v : graph->pred(u)) {
                auto copy = cur;
                std::swap(copy[u], copy[v]);

                int cId = mMapId[copy];
                if (!inserted[cId]) {
                    inserted[cId] = true;
                    mSwaps[cId] = mSwaps[aId];
                    mSwaps[cId].push_back(Swap { u, v });
                    q.push(cId);
                }
            }
        }
    }
}

efd::ExpTSFinder::ExpTSFinder(Graph::sRef graph) : TokenSwapFinder(graph) {
    preprocess(graph.get());
}

efd::SwapSeq efd::ExpTSFinder::find(Assign from, Assign to) {
    assert(mG.get() != nullptr && "Trying to find a swap seq. but no graph given.");
    return mSwaps[getTargetId(from, to)];
}

efd::SwapSeq efd::ExpTSFinder::find(Graph::Ref graph, Assign from, Assign to) {
    if (mG.get()) mG.reset();
    preprocess(graph);
    return mSwaps[getTargetId(from, to)];
}

efd::ExpTSFinder::uRef efd::ExpTSFinder::Create(Graph::sRef graph) {
    return uRef(new ExpTSFinder(graph));
}
