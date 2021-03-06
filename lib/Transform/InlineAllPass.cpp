#include "enfield/Transform/InlineAllPass.h"
#include "enfield/Analysis/NodeVisitor.h"

uint8_t efd::InlineAllPass::ID = 0;

namespace efd {
    class InlineAllVisitor : public NodeVisitor {
        private:
            std::set<std::string> mBasis;

        public:
            std::vector<NDQOp::Ref> mInlineVector;

            InlineAllVisitor(std::set<std::string> basis) : mBasis(basis) {}

            void visit(NDQOpGen::Ref ref) override;
            void visit(NDIfStmt::Ref ref) override;
    };
}

void efd::InlineAllVisitor::visit(NDQOpGen::Ref ref) {
    if (mBasis.find(ref->getId()->getVal()) == mBasis.end()) {
        mInlineVector.push_back(ref);
    }
}

void efd::InlineAllVisitor::visit(NDIfStmt::Ref ref) {
    ref->getQOp()->apply(this);
}

efd::InlineAllPass::InlineAllPass(std::vector<std::string> basis) {
    mBasis = std::set<std::string>(basis.begin(), basis.end());
}

bool efd::InlineAllPass::run(QModule::Ref qmod) {
    InlineAllVisitor visitor(mBasis);

    bool changed = false;

    do {
        visitor.mInlineVector.clear();
        // Inline until we can't inline anything anymore.
        for (auto it = qmod->stmt_begin(), e = qmod->stmt_end(); it != e; ++it) {
            (*it)->apply(&visitor);
        }

        for (auto call : visitor.mInlineVector) {
            auto sign = qmod->getQGate(call->getId()->getVal());

            // Inline only non-opaque gates.
            if (!sign->isOpaque()) {
                qmod->inlineCall(call);
            }
        }

        if (!visitor.mInlineVector.empty()) changed = true;
    } while (!visitor.mInlineVector.empty());

    return changed;
}

efd::InlineAllPass::uRef efd::InlineAllPass::Create(std::vector<std::string> basis) {
    return uRef(new InlineAllPass(basis));
}
