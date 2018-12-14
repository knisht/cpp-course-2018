#ifndef LIBRARIAN_TASK_CONTEXT_H
#define LIBRARIAN_TASK_CONTEXT_H
#include <memory>

template <class Caller, typename... Arg>
struct TaskContext {
    size_t transactionalId;
    Caller *caller;
    void (Caller::*callOnSuccess)(Arg...);

    bool isTaskCancelled()
    {
        return caller->getTransactionalId() != transactionalId;
    }
};

#endif
