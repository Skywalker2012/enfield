#include "enfield/Transform/Allocators/QbitterSolBuilder.h"
#include "enfield/Support/BFSPathFinder.h"

efd::Solution efd::QbitterSolBuilder::build
(Mapping initial, DepsSet& deps, ArchGraph::Ref g) {
    auto mapping = initial;
    auto assign = GenAssignment(g->size(), mapping);
    auto finder = BFSPathFinder::Create();

    Solution solution { initial, Solution::OpSequences(deps.size()), 0 };

    for (uint32_t i = 0, e = deps.size(); i < e; ++i) {
        auto dep = deps[i][0];

        auto& ops = solution.mOpSeqs[i];
        ops.first = deps[i].mCallPoint;

        // (u, v) edge in Arch
        uint32_t a = dep.mFrom, b = dep.mTo;
        uint32_t u = mapping[a], v = mapping[b];

        Operation operation;
        if (g->hasEdge(u, v)) {
            operation = { Operation::K_OP_CNOT, a, b };
        } else if (g->isReverseEdge(u, v)) {
            solution.mCost += RevCost.getVal();
            operation = { Operation::K_OP_REV, a, b };
        } else {
            solution.mCost += LCXCost.getVal();
            operation = { Operation::K_OP_LCNOT, a, b };

            auto path = finder->find(g, u, v);
            assert(path.size() == 3 && "Can't apply a long cnot.");
            operation.mW = assign[path[1]];
        }
        ops.second.push_back(operation);
    }

    return solution;
}

efd::QbitterSolBuilder::uRef efd::QbitterSolBuilder::Create() {
    return uRef(new QbitterSolBuilder());
}
