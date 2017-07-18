#ifndef __EFD_COMMAND_LINE_H__
#define __EFD_COMMAND_LINE_H__

#include <string>
#include <vector>
#include <cassert>
#include <memory>

namespace efd {
    class ArgsParser;

    /// \brief Base class for implementing command line options.
    class OptBase {
        private:
            bool mIsRequired;
            bool mIsParsed;

            std::shared_ptr<ArgsParser> mParser;

        protected:
            /// \brief Type sensitive parsing of the arguments itself.
            virtual void parseImpl(const int argc, const char **argv) = 0;

        public:
            /// \brief Command line string that represents the option.
            std::string mName;
            /// \brief Description of the command line option.
            std::string mDescription;

            /// \brief Constructs and inserts itself in the command line options container.
            OptBase(std::string name, std::string description, bool isRequired);
            /// \brief Destructs and removes itself from the command line options container.
            virtual ~OptBase();

            /// \brief True if this option has already been parsed.
            bool isParsed();
            /// \brief True if the option was constructed as 'required'.
            bool isRequired();

            /// \brief True if this is a boolean option.
            virtual bool isBoolean() = 0;
            /// \brief Calls the \em parseImpl function, which is type-dependent.
            void parse(const int argc, const char **argv);
    };

    /// \brief Class used to declare the command line options available.
    template <typename T>
    class Opt : OptBase {
        private:
            T mVal;

        protected:
            void parseImpl(const int argc, const char **argv) override;

        public:
            Opt(std::string name, std::string description, bool isRequired = false);
            Opt(std::string name, std::string description, T def, bool isRequired = false);

            /// \brief Gets a const reference to the value of this command line option.
            const T& getVal() const;
            bool isBoolean() override;
    };

    /// \brief Parses the command line arguments (this function should be used in the main
    /// function).
    void ParseArguments(const int argc, const char **argv);
    void ParseArguments(int argc, char **argv);

    template class Opt<bool>;
    template <> bool Opt<bool>::isBoolean();
    template <> void Opt<bool>::parseImpl(const int argc, const char **argv);

    template class Opt<int>;
    template <> void Opt<int>::parseImpl(const int argc, const char **argv);

    template class Opt<unsigned>;
    template <> void Opt<unsigned>::parseImpl(const int argc, const char **argv);

    template class Opt<long long>;
    template <> void Opt<long long>::parseImpl(const int argc, const char **argv);

    template class Opt<unsigned long long>;
    template <> void Opt<unsigned long long>::parseImpl(const int argc, const char **argv);

    template class Opt<float>;
    template <> void Opt<float>::parseImpl(const int argc, const char **argv);

    template class Opt<double>;
    template <> void Opt<double>::parseImpl(const int argc, const char **argv);

    template class Opt<std::string>;
    template <> void Opt<std::string>::parseImpl(const int argc, const char **argv);

    template class Opt<std::vector<std::string>>;
    template <> void Opt<std::vector<std::string>>::parseImpl(const int argc, const char **argv);
};

template <typename T>
efd::Opt<T>::Opt(std::string name, std::string description, bool isRequired) :
    OptBase(name, description, isRequired) {}

template <typename T>
efd::Opt<T>::Opt(std::string name, std::string description, T def, bool isRequired) :
    OptBase(name, description, isRequired), mVal(def) {}

template <typename T>
const T& efd::Opt<T>::getVal() const { return mVal; }

template <typename T>
void efd::Opt<T>::parseImpl(const int argc, const char **argv) {
    assert(false && "Option with 'parse' function not implemented.");
}

template <typename T>
bool efd::Opt<T>::isBoolean() { return false; }

#endif
