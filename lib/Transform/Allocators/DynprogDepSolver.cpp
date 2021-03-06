#include "enfield/Transform/Allocators/DynprogDepSolver.h"
#include "enfield/Support/BFSPathFinder.h"
#include "enfield/Support/CommandLine.h"
#include "enfield/Support/ExpTSFinder.h"

#include <unordered_map>
#include <limits>
#include <queue>
#include <iostream>
#include <algorithm>

const uint32_t UNREACH = std::numeric_limits<uint32_t>::max();

struct Val {
    uint32_t pId;
    Val* parent;
    uint32_t cost;
};

struct PermVal {
    uint32_t idx;
    std::vector<uint32_t> perm;
};

static inline uint32_t min(uint32_t a, uint32_t b) {
    if (a == UNREACH && b == UNREACH)
        return UNREACH;

    if (a == UNREACH) return b;
    if (b == UNREACH) return a;

    return (a < b) ? a : b;
}

static inline Val minVal(Val& a, Val& b) {
    uint32_t cost = min(a.cost, b.cost);
    if (cost == a.cost) return a;
    else return b;
}

uint32_t efd::DynprogDepSolver::getIntermediateV(uint32_t u, uint32_t v) {
    auto& succ = mArchGraph->succ(u);

    for (auto& w : succ) {
        for (auto& z : mArchGraph->succ(w))
            if (z == v) return w;
        for (auto& z : mArchGraph->pred(w))
            if (z == v) return w;
    }

    return UNREACH;
}

efd::Solution efd::DynprogDepSolver::solve(DepsSet& deps) {
    ExpTSFinder tsp(mArchGraph);
    auto permutations = tsp.mAssigns;

    uint32_t archQ = mArchGraph->size();
    const uint32_t SWAP_COST = SwapCost.getVal();
    const uint32_t REV_COST = RevCost.getVal();
    const uint32_t LCX_COST = LCXCost.getVal();

    uint32_t permN = permutations.size();
    uint32_t depN = deps.size();

    auto finder = BFSPathFinder::Create();

    // std::vector<std::vector<uint32_t>*> permIdMap(permN, nullptr);
    // for (auto &pair : PermMap)
    //     permIdMap[pair.second.idx] = &pair.second.perm;

    // Map with the minimum number of vals at time 'i'.
    Val vals[permN][depN + 1];

    for (uint32_t i = 0; i < permN; ++i)
        vals[i][0] = { i, nullptr, 0 };

    for (uint32_t i = 0; i < permN; ++i)
        for (uint32_t j = 1; j <= depN; ++j)
            vals[i][j] = { i, nullptr, UNREACH };

    for (uint32_t i = 1; i <= depN; ++i) {
        assert(deps[i-1].getSize() == 1 &&
                "Trying to allocate qbits to a gate with more than one dependency.");
        efd::Dep dep = deps[i-1].mDeps[0];

        for (uint32_t tgt = 0; tgt < permN; ++tgt) {
            // Check if target tgtPermutation has the dependency required.
            auto& tgtPerm = permutations[tgt];
            // Arch qubit interaction (u, v)
            uint32_t u = tgtPerm[dep.mFrom], v = tgtPerm[dep.mTo];

            // We don't use this configuration if (u, v) is neither a norma edge
            // nor a reverse edge of the physical graph nor is at a 2-edge distance
            // (u -> w -> v).
            bool hasEdge = mArchGraph->hasEdge(u, v);
            bool isReverse = mArchGraph->isReverseEdge(u, v);
            bool is2Dist = finder->find(mArchGraph.get(), u, v).size() == 3;
            if (!hasEdge && !isReverse && !is2Dist)
                continue;

            Val minimum { tgt, nullptr, UNREACH };

            for (uint32_t src = 0; src < permN; ++src) {
                Val& srcVal = vals[src][i-1];
                if (srcVal.cost == UNREACH)
                    continue;

                uint32_t finalCost = srcVal.cost;

                if (tgt != src) {
                    auto srcAssign = GenAssignment(archQ, permutations[src]);
                    auto tgtAssign = GenAssignment(archQ, tgtPerm);
                    finalCost += tsp.find(srcAssign, tgtAssign).size() * SWAP_COST;
                }

                if (!hasEdge) {
                    // Increase cost if using reverse edge.
                    if (isReverse)
                        finalCost += REV_COST;
                    // Else, increase cost if using long cnot gate.
                    else if (is2Dist)
                        finalCost += LCX_COST;
                }

                Val thisVal { tgt, &srcVal, finalCost };
                minimum = minVal(minimum, thisVal);
            }

            vals[tgt][i] = minimum;
        }
    }

    // Get the minimum cost setup.
    Val* val = &vals[0][depN];
    for (uint32_t i = 1; i < permN; ++i) {
        uint32_t minCost = min(val->cost, vals[i][depN].cost);
        val = (minCost == val->cost) ? val : &vals[i][depN];
    }

    Solution solution;
    solution.mCost = val->cost;
    solution.mOpSeqs.assign(depN, std::pair<Node::Ref, Solution::OpVector>());

    // Get the target mappings for each dependency (with its id).
    std::vector<std::pair<uint32_t, Mapping>> mappings(depN);

    for (int i = depN-1; i >= 0; --i) {
        assert(val->parent != nullptr && "Nullptr reached too soon.");
        mappings[i] = std::make_pair(val->pId, permutations[val->pId]);
        val = val->parent;
    }

    if (depN == 0) {
        for (uint32_t i = 0; i < archQ; ++i)
            solution.mInitial.push_back(i);
        solution.mCost = 0;
    } else {
        solution.mInitial = mappings[0].second;
        solution.mOpSeqs[0].first = deps[0].mCallPoint;
        for (uint32_t i = 1; i < depN; ++i) {
            uint32_t srcId = mappings[i-1].first, tgtId = mappings[i].first;

            auto& ops = solution.mOpSeqs[i];
            auto& src = mappings[i-1].second;
            auto& tgt = mappings[i].second;

            if (srcId != tgtId) {
                auto srcAssign = GenAssignment(archQ, src);
                auto tgtAssign = GenAssignment(archQ, tgt);

                auto swaps = tsp.find(srcAssign, tgtAssign);
                for (auto swp : swaps) {
                    uint32_t u = swp.u, v = swp.v;

                    if (mArchGraph->isReverseEdge(u, v))
                        std::swap(u, v);

                    ops.second.push_back({ Operation::K_OP_SWAP, srcAssign[u], srcAssign[v] });
                    std::swap(srcAssign[u], srcAssign[v]);
                }
            }

            auto dep = deps[i][0];
            uint32_t a = dep.mFrom, b = dep.mTo;
            uint32_t u = tgt[a], v = tgt[b];
            auto assign = GenAssignment(mArchGraph->size(), tgt);

            Operation operation;

            if (mArchGraph->hasEdge(u, v))
                operation = { Operation::K_OP_CNOT, a, b };
            else if (mArchGraph->isReverseEdge(u, v))
                operation = { Operation::K_OP_REV, a, b };
            else {
                auto path = finder->find(mArchGraph.get(), u, v);
                assert(path.size() == 3 && "Can't apply a long cnot.");
                operation = { Operation::K_OP_LCNOT, a, b };
                operation.mW = assign[path[1]];
            }

            ops.first = deps[i].mCallPoint;
            ops.second.push_back(operation);
        }
    }

    return solution;
}

efd::DynprogDepSolver::DynprogDepSolver(ArchGraph::sRef pGraph) 
    : DepSolverQAllocator(pGraph) {
}

efd::DynprogDepSolver::uRef efd::DynprogDepSolver::Create
(ArchGraph::sRef archGraph) {
    return uRef(new DynprogDepSolver(archGraph));
}
