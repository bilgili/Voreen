#ifndef VRN_GRAPHVISITOR_H
#define VRN_GRAPHVISITOR_H

namespace voreen {

class Processor;

class GraphVisitor {
public:
    virtual ~GraphVisitor() {}
    virtual bool visit(Processor* const processor) = 0;
};

// ----------------------------------------------------------------------------

template<typename RET = void, class PROC = Processor>
class CallbackGraphVisitor : public GraphVisitor{
public:
    CallbackGraphVisitor(RET (PROC::*method)() const) 
        : GraphVisitor(),
        method_(method)
    {
    }

    virtual bool visit(Processor* const processor) {
        PROC* const proc = dynamic_cast<PROC* const>(processor);
        if ((proc != 0) && (method_ != 0)) {
            (proc->*method_)();
            return true;
        }
        return false;
    }

private:
    RET (PROC::*method_)() const;
};

// ----------------------------------------------------------------------------

template<typename RET, class PROC, typename ARG>
class Call1ArgGraphVisitor : public GraphVisitor{
public:
    Call1ArgGraphVisitor(RET (PROC::*method)(ARG), ARG arg) 
        : GraphVisitor(),
        method_(method),
        arg_(arg)
    {
    }

    virtual bool visit(Processor* const processor) {
        PROC* const proc = dynamic_cast<PROC* const>(processor);
        if ((proc != 0) && (method_ != 0)) {
            (proc->*method_)(arg_);
            return true;
        }
        return false;
    }

private:
    RET (PROC::*method_)(ARG);
    ARG arg_;
};

// ----------------------------------------------------------------------------

}   // namespace

#endif  // VRN_GRAPHVISITOR_H
