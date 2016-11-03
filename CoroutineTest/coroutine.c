//
//  coroutine.c
//  CoroutineTest
//
//  Created by Thomas Chen on 2016/11/2.
//  Copyright © 2016年 Thomas Chen. All rights reserved.
//

#include "coroutine.h"
#include <stdlib.h>

#define STACK_SIZE 16384

typedef struct CoroutineOpaque {
    // esp must be the first member because inline asm accesses it by offset 0.
    unsigned int esp;
    void *stack;
    struct CoroutineOpaque *prev;
    struct CoroutineOpaque *next;
} CoroutineOpaque;

static CoroutineOpaque *current;
static CoroutineOpaque looper;

#ifdef _MSC_VER

static void __declspec(naked) _switch_to(CoroutineOpaque *from, CoroutineOpaque *to) {
    __asm {
        push ebp
        mov eax, 8[esp]
        mov [eax], esp
        mov eax, 12[esp]
        mov esp, [eax]
        pop ebp
        ret
    };
}

#else

static void __attribute__ ((naked)) _switch_to(CoroutineOpaque *from, CoroutineOpaque *to) {
    asm ("push %ebp\n"
         "mov 8(%esp), %eax\n"
         "mov %esp, (%eax)\n"
         "mov 12(%esp), %eax\n"
         "mov (%eax), %esp\n"
         "pop %ebp\n"
         "ret\n"
         );
}

#endif

static void switch_to_current() {
    _switch_to(&looper, current);
}

static void switch_to_looper() {
    _switch_to(current, &looper);
    // never return
}

void yield() {
    current = current->next;
    _switch_to(current->prev, current);
}

int cocreate(Coroutine *co, CoFunction f, void *arg) {
    if (co == 0) {
        return -1;
    }
    
    CoroutineOpaque *obj = malloc(sizeof(CoroutineOpaque));
    if (obj == 0) {
        return -2;
    }
    
    void *stack = (void *)malloc(STACK_SIZE);
    if (stack == 0) {
        free(obj);
        return -3;
    }
    // 16 bytes alignment for mac
    *(unsigned int*)((char *)stack + STACK_SIZE - 16) = (unsigned int)arg;
    *(unsigned int*)((char *)stack + STACK_SIZE - 20) = (unsigned int)switch_to_looper;
    *(unsigned int*)((char *)stack + STACK_SIZE - 24) = (unsigned int)f;
    *(unsigned int*)((char *)stack + STACK_SIZE - 28) = 0;
    obj->esp = (unsigned int)stack + STACK_SIZE - 28;
    obj->stack = stack;
    
    if (current == 0) {
        obj->prev = obj;
        obj->next = obj;
        current = obj;
    } else {
        obj->prev = current->prev;
        obj->next = current;
        current->prev->next = obj;
        current->prev = obj;
    }
    
    *co = obj;
    return 0;
}

static void codestroy(CoroutineOpaque *co) {
    if (co->next != co) {
        co->prev->next = co->next;
        co->next->prev = co->prev;
        if (co == current) {
            current = co->next;
        }
    } else {
        current = 0;
    }
    free(co->stack);
    free(co);
}

int coloop() {
    while (current != 0) {
        switch_to_current();
        codestroy(current);
    }

    return 0;
}
