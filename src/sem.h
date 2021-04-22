#ifndef _SEM_H_
#define _SEM_H_

/**
 * This source code is a solution for sem_init() deprecated on Mac OS X, which is portable both on darwin and linux
 * It references from https://stackoverflow.com/questions/27736618/why-are-sem-init-sem-getvalue-sem-destroy-deprecated-on-mac-os-x-and-w
*/
#ifdef __APPLE__
#include <dispatch/dispatch.h>
#else
#include <semaphore.h>
#endif

struct rk_sema {
#ifdef __APPLE__
    dispatch_semaphore_t    sem;
#else
    sem_t                   sem;
#endif
};

void rk_sema_init(struct rk_sema *s, uint32_t value);
void rk_sema_wait(struct rk_sema *s);
void rk_sema_post(struct rk_sema *s);


#endif // !_SEM_H_