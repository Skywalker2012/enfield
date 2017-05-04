#ifndef __EFD_NODES_H__
#define __EFD_NODES_H__

#include <iostream>
#include <vector>
#include <memory>

namespace efd {
    class Node {
        protected:
            bool mIsEmpty;

        public:
            typedef std::vector< std::shared_ptr<Node> >::iterator iterator;
            typedef std::vector< std::shared_ptr<Node> >::const_iterator const_iterator;

            Node(bool empty);

            std::vector< std::shared_ptr<Node> > mChild;

            iterator begin();
            iterator end();

            const_iterator begin() const;
            const_iterator end() const;

            void print(std::ostream& O = std::cout, bool endl = false);
            void print(bool endl = false);

            bool isEmpty() const;

            virtual std::string getOperation() const;
            virtual std::string toString(bool endl = false) const;
    };

    class NDDecl : public Node {
        public:
            enum Type {
                CONCRETE,
                QUANTUM
            };

        private:
            enum ChildType {
                I_ID = 0,
                I_SIZE
            };

            Type mT;

        public:
            NDDecl(Type t);

            bool isCReg() const;
            bool isQReg() const;

            std::string getOperation() const override;
            std::string toString(bool endl) const override;
    };

    class NDGateDecl : public Node {
        private:
            enum ChildType {
                I_ID = 0,
                I_ARGS,
                I_QARGS,
                I_GOPLIST
            };

        public:
            NDGateDecl();
            std::string getOperation() const override;
            std::string toString(bool endl) const override;
    };

    class NDGOpList : public Node {
        public:
            NDGOpList();
            std::string toString(bool endl) const override;
    };

    class NDOpaque : public Node {
        private:
            enum ChildType {
                I_ID = 0,
                I_ARGS,
                I_QARGS
            };

        public:
            NDOpaque();
            std::string getOperation() const override;
            std::string toString(bool endl) const override;
    };

    class NDQOp : public Node {
        public:
            enum QOpType {
                QOP_RESET,
                QOP_BARRIER,
                QOP_MEASURE,
                QOP_U,
                QOP_CX,
                QOP_GENERIC
            };

        private:
            enum ChildType {
                I_ID = 0,
                I_ARGS,
                I_QARGS
            };

            QOpType mT;

        public:
            NDQOp(QOpType type);

            QOpType getQOpType() const;

            virtual bool isReset() const;
            virtual bool isBarrier() const;
            virtual bool isMeasure() const;
            virtual bool isU() const;
            virtual bool isGeneric() const;

            std::string getOperation() const override;
            std::string toString(bool endl) const override;
    };

    class NDQOpMeasure : public NDQOp {
        private:
            enum ChildType {
                I_QBIT = 0,
                I_CBIT
            };

        public:
            NDQOpMeasure();
            std::string getOperation() const override;
            std::string toString(bool endl) const override;
    };

    class NDQOpReset : public NDQOp {
        public:
            NDQOpReset();
            std::string getOperation() const override;
    };

    class NDQOpBarrier : public NDQOp {
        public:
            NDQOpBarrier();
            std::string getOperation() const override;
    };

    class NDQOpGeneric : public NDQOp {
        private:
            enum ChildType {
                I_QBIT = 0,
                I_CBIT
            };

        public:
            NDQOpGeneric();
    };
};

#endif
