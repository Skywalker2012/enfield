
#include "gtest/gtest.h"

#include "enfield/Transform/Allocators/BoundedSIDepSolver.h"
#include "enfield/Transform/SemanticVerifierPass.h"
#include "enfield/Transform/ArchVerifierPass.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Arch/ArchGraph.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/uRefCast.h"

#include <string>

using namespace efd;

static ArchGraph::sRef createGraph() {
    ArchGraph::sRef g(nullptr);
    const std::string gStr =
"\
1 5\n\
q 5\n\
q[0] q[1]\n\
q[1] q[2]\n\
q[0] q[2]\n\
q[3] q[2]\n\
q[4] q[2]\n\
q[3] q[4]\n\
";

    g = toShared(ArchGraph::ReadString(gStr));
    return g;
}

void TestAllocation(const std::string program) {
    static ArchGraph::sRef g(nullptr);
    if (g.get() == nullptr) g = createGraph();

    auto qmod = QModule::ParseString(program);
    auto qmodCopy = qmod->clone();

    auto allocator = BoundedSIDepSolver::Create(g);
    allocator->setInlineAll({ "cx" });
    allocator->run(qmod.get());

    auto mapping = allocator->getData().mInitial;

    qmod->print(std::cout, true);
    qmodCopy->print(std::cout, true);

    auto aVerifierPass = ArchVerifierPass::Create(g);
    PassCache::Run(qmod.get(), aVerifierPass.get());
    EXPECT_TRUE(aVerifierPass->getData());

    auto sVerifierPass = SemanticVerifierPass::Create(std::move(qmodCopy), mapping);
    sVerifierPass->setInlineAll({ "cx" });
    PassCache::Run(qmod.get(), sVerifierPass.get());
    EXPECT_TRUE(sVerifierPass->getData());
}

TEST(BoundedSIDepSolverTests, SimpleNoSwapProgram) {
    {
        const std::string program =
"\
qreg q[5];\
CX q[0], q[1];\
";
        TestAllocation(program);
    }

    {
        const std::string program =
"\
qreg q[5];\
CX q[2], q[1];\
CX q[2], q[0];\
CX q[1], q[0];\
CX q[4], q[3];\
CX q[4], q[0];\
CX q[3], q[0];\
";
        // Expected [ 2 1 0 4 3 ]
        TestAllocation(program);
    }
}

TEST(BoundedSIDepSolverTests, GatesTest) {
    {
        const std::string program =
"\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
";
        TestAllocation(program);
    }

    {
        const std::string program =
"\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
test q[3], q[4], q[2];\
";
        TestAllocation(program);
    }
}

TEST(BoundedSIDepSolverTests, GatesSwapTest) {
    {
        const std::string program =
"\
qreg q[5];\
gate test a, b, c {CX a, b;CX a, c;CX b, c;}\
test q[0], q[1], q[2];\
test q[4], q[1], q[0];\
";
        // Expected mapping: [ 0 2 1 3 4 ]
        TestAllocation(program);
    }
}
