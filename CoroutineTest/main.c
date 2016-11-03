//
//  main.c
//  CoroutineTest
//
//  Created by Thomas Chen on 2016/11/1.
//  Copyright © 2016年 Thomas Chen. All rights reserved.
//

#include "coroutine.h"
#include <stdio.h>

void c(void *arg) {
    int i = (int)arg;
    printf("c %d\n", i);
    if (i < 1000) {
        Coroutine co;
        cocreate(&co, c, (void *)(i + 1));
    }
}

void e(void *arg) {
    for (int i = 0; i < 2; i++) {
        printf("e %d %d\n", (int)arg, i);
        yield();
    }
}

void f(void *arg) {
    for (int a = 0; a < 10; a++) {
        printf("f %d %d\n", (int)arg, a);
        yield();
    }
    printf("f done\n");
}

void g(void *arg) {
    for (int a = 0; a < 10; a++) {
        printf("g %d %d\n", (int)arg, a);
        yield();
    }
    printf("g done\n");
}

int main(int argc, const char *argv[]) {
    Coroutine h;
    Coroutine h1;
    Coroutine h2;
    
    cocreate(&h, c, 0);
    
    coloop();
    
    cocreate(&h1, f, 0);
    cocreate(&h2, g, (void *)1);
    
    coloop();
    
    for (int i = 0; i < 10000; i++) {
        cocreate(&h, e, (void *)i);
    }
    
    coloop();

    printf("done\n");
}
