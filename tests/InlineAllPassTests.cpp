
#include "gtest/gtest.h"

#include "enfield/Transform/InlineAllPass.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Support/uRefCast.h"

#include <string>

using namespace efd;

TEST(InlineAllPassTests, NoBasisInline) {
    {
        const std::string program =
"\
qreg q[5];\
cx q[0], q[1];\
";
        const std::string result =
"\
include \"qelib1.inc\";\
qreg q[5];\
CX q[0], q[1];\
";
        auto qmod = toShared(std::move(QModule::ParseString(program, true)));
        InlineAllPass::uRef pass = InlineAllPass::Create(qmod);
        qmod->runPass(pass.get());
        ASSERT_EQ(qmod->toString(), result);
    }
    {
        const std::string program =
"\
qreg q[5];\
gate somegate(a, b) x, y {\
    cx x, y;\
    cx y, x;\
    U(a, b, 10) x;\
    U(a, b, 10) y;\
}\
somegate(10, 20) q[0], q[1];\
";
        const std::string result =
"\
include \"qelib1.inc\";\
qreg q[5];\
gate somegate(a, b) x, y {\
cx x, y;\
cx y, x;\
U(a, b, 10) x;\
U(a, b, 10) y;\
}\
cx q[0], q[1];\
cx q[1], q[0];\
U(10, 20, 10) q[0];\
U(10, 20, 10) q[1];\
";
        auto qmod = toShared(std::move(QModule::ParseString(program, true)));
        InlineAllPass::uRef pass = InlineAllPass::Create(qmod);
        qmod->runPass(pass.get());
        ASSERT_EQ(qmod->toString(), result);
    }
}

TEST(InlineAllPassTests, BasisInline) {
    {
        const std::string program =
"\
qreg q[5];\
cx q[0], q[1];\
";
        const std::string result =
"\
include \"qelib1.inc\";\
qreg q[5];\
cx q[0], q[1];\
";
        auto qmod = toShared(std::move(QModule::ParseString(program, true)));
        InlineAllPass::uRef pass = InlineAllPass::Create(qmod, { "cx" });
        qmod->runPass(pass.get());
        ASSERT_EQ(qmod->toString(), result);
    }
    {
        const std::string program =
"\
qreg q[5];\
gate somegate(a, b) x, y {\
    cx x, y;\
    cx y, x;\
    U(a, b, 10) x;\
    U(a, b, 10) y;\
}\
somegate(10, 20) q[0], q[1];\
";
        const std::string result =
"\
include \"qelib1.inc\";\
qreg q[5];\
gate somegate(a, b) x, y {\
cx x, y;\
cx y, x;\
U(a, b, 10) x;\
U(a, b, 10) y;\
}\
somegate(10, 20) q[0], q[1];\
";
        auto qmod = toShared(std::move(QModule::ParseString(program, true)));
        InlineAllPass::uRef pass = InlineAllPass::Create(qmod, { "somegate" });
        qmod->runPass(pass.get());
        ASSERT_EQ(qmod->toString(), result);
    }
}
