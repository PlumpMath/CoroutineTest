//
//  coroutine.h
//  CoroutineTest
//
//  Created by Thomas Chen on 2016/11/2.
//  Copyright © 2016年 Thomas Chen. All rights reserved.
//

#ifndef coroutine_h
#define coroutine_h

struct CoroutineOpaque;
typedef struct CoroutineOpaque *Coroutine;
typedef void (*CoFunction)(void *);

void yield();

int cocreate(Coroutine *co, CoFunction f, void *arg);

int coloop();

#endif /* coroutine_h */
