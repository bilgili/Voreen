#ifndef VRN_GRAPHVISITOR_H
#define VRN_GRAPHVISITOR_H

#include <typeinfo>

namespace voreen {

class Processor;

/**
 * Pure virtual base class for visitor design-pattern used by <code>NetworkGraph</code> class.
 * Visitors used by NetworkGraph have to implement this interface.
 */
class GraphVisitor {
public:
    virtual ~GraphVisitor() {}

    /**
     * Intended to perfom the visit on the given Processor object. The return
     * value shall indicate possible failures during the visit, e.g. a passed
     * NULL-pointer to a processor object, etc..
     *
     * @return  true shall be returned if the visit has been successful, false
     *          otherwise.
     */
    virtual bool visit(Processor* const processor) = 0;
};

// ----------------------------------------------------------------------------

/**
 * Template class to perform visits on pointers to Processor objects in
 * NetworkGraph objects by calling a certain method with return value RET on
 * processors of class PROC. If the visited processor is not of class PROC, nothing
 * will be happen. The return value of the method call is not employed at all.
 */
template<typename RET = void, class PROC = Processor>
class CallbackGraphVisitor : public GraphVisitor{
public:
    /**
     * Ctor.
     *
     * @param   *method Pointer to the method in class PROC with a return value of
     *                  type RET which shall be called on visits.
     */
    CallbackGraphVisitor(RET (PROC::*method)() const)
        : GraphVisitor(),
        method_(method)
    {
    }

    /**
     * Performs the visit on the given processor, i.e. that the method, specified
     * during construction of this object, is called on the given Processor pointer,
     * if it is of class PROC and not NULL. The value returned from the call to the
     * specified method is not employed.
     *
     * @return  true if the pointer to the Processor obejct is castable to PROC and
     *          if it is not NULL, and if the pointer to the method was not NULL either
     *          so that it can be called. Otherwise, if any pointer is NULL, false
     *          will be returned.
     */
    virtual bool visit(Processor* const processor) {
        PROC* const proc = dynamic_cast<PROC* const>(processor);
        if ((proc != 0) && (method_ != 0)) {
            (proc->*method_)();
            return true;
        }
        return false;
    }

private:
    /** Pointer to a method returning values of RET in Processor class PROC. */
    RET (PROC::*method_)() const;
};

// ----------------------------------------------------------------------------

/**
 * Template class to perform visits on pointers to Processor objects in
 * NetworkGraph objects by calling a certain method taking one argument of type
 * ARG and with return value RET on processors of class PROC. If the visited
 * processor is not of class PROC, nothing will be happen. The return value of
 * the method call is not employed at all.
 */
template<typename RET, class PROC, typename ARG>
class Call1ArgGraphVisitor : public GraphVisitor{
public:
    /**
     * Ctor.
     *
     * @param   *method Pointer to the method in class PROC with a return value of
     *                  type RET taking one argument of type ARG which shall be called
     *                  on visits.
     * @param   arg The argument which shall be passed on method calls to the Processor
     *              objects.
     */
    Call1ArgGraphVisitor(RET (PROC::*method)(ARG), ARG arg)
        : GraphVisitor(),
        method_(method),
        arg_(arg)
    {
    }

    /**
     * Performs the visit on the given processor, i.e. that the method, specified
     * during construction of this object, is called on the given Processor pointer,
     * if it is of class PROC and not NULL. The value returned from the call to the
     * specified method is not employed.
     *
     * @return  true if the pointer to the Processor obejct is castable to PROC and
     *          if it is not NULL, and if the pointer to the method was not NULL either
     *          so that it can be called. Otherwise, if any pointer is NULL, false
     *          will be returned.
     */
    virtual bool visit(Processor* const processor) {
        PROC* const proc = dynamic_cast<PROC* const>(processor);
        if ((proc != 0) && (method_ != 0)) {
            (proc->*method_)(arg_);
            return true;
        }
        return false;
    }

private:
    /**
     * Pointer to a method returning values of RET and expecting an argument
     * of type ARG in Processor class PROC.
     */
    RET (PROC::*method_)(ARG);

    /** The argument which is passed via method pointers to the Processor objects. */
    ARG arg_;
};

// ----------------------------------------------------------------------------

}   // namespace

#endif  // VRN_GRAPHVISITOR_H
