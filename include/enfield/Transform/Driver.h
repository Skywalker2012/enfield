#ifndef __EFD_DRIVER_H__
#define __EFD_DRIVER_H__

#include "enfield/Transform/QModule.h"
#include "enfield/Arch/Architectures.h"
#include "enfield/Transform/Allocators/Allocators.h"

namespace efd {
    /// \brief Required information in order to compile a \em QModule.
    struct CompilationSettings {
        ArchGraph::sRef archGraph;
        EnumAllocator allocator;
        std::vector<std::string> basis;
        bool reorder;
        bool verify;
        bool force;
    };

    /// \brief Compile \p qmod, and return the compiled version.
    ///
    /// Transform \p qmod according to \p settings. It shall specify the architecture,
    /// the allocator to use, the basis vector and whether to reorder the program or not.
    QModule::uRef Compile(QModule::uRef qmod, CompilationSettings settings);

    /// \brief Parse file in the path \p filepath.
    QModule::uRef ParseFile(std::string filepath);

    /// \brief Print \p qmod to an standard output stream \p o.
    void PrintToStream(QModule::Ref qmod, std::ostream& o = std::cout, bool pretty = true);

    /// \brief Print the dependency graph of the \p qmod in \p o.
    void PrintDependencyGraph(QModule::Ref qmod, std::ostream& o = std::cout);
}

#endif
