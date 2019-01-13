#ifndef LIBRARIAN_TASK_CONTEXT_H
#define LIBRARIAN_TASK_CONTEXT_H
#include <functional>

template <class Caller, typename... Arg>
struct TaskContext {
    size_t transactionalId;
    Caller *caller;
    void (Caller::*callOnSuccess)(Arg...);

    void apply(Arg... args) { std::invoke(callOnSuccess, caller, args...); }

    bool isTaskCancelled()
    {
        return caller->getTransactionalId() != transactionalId;
    }
};

#endif
