# CSE3209 시스템 프로그래밍 (2025 Fall)
## 프로젝트: Custom Library in Linux (Static / Shared / Run-time Linking)

이 프로젝트는 `long long` 범위를 넘어서는 **큰 정수(Big Integer) 덧셈/뺄셈**을 문자열 기반으로 제공하는 간단한 라이브러리입니다.

- Static Library: `libbig.a`
- Shared Library: `libbig.so`
- Run-time linking: `dlopen / dlsym / dlclose / dlerror`

환경: Ubuntu Linux x86_64

---

## 1) 제공 함수 (big.h)

```c
#ifndef BIG_H
#define BIG_H

char* big_add(const char* a, const char* b);
char* big_sub(const char* a, const char* b); // 음수 결과 지원

#endif
````

### 입력 규칙

* 허용: `"123"`, `"-123"`, `"000123"`, `"-000123"`
* 금지: `"+45"` 처럼 `+`로 시작하는 입력
* `"-0"`, `"-000"` 같은 입력은 내부적으로 `0`으로 처리

### 출력 규칙

* 결과 문자열은 `malloc()`으로 할당되어 반환됩니다.
* 호출자는 반드시 `free()` 해야 합니다.
* 결과가 0이면 항상 `"0"` (절대 `"-0"` 출력 안 함)

---

## 2) 파일 구성

* `big.c` : 라이브러리 구현
* `big.h` : 헤더
* `test_static.c` : static library 링크 테스트
* `test_shared.c` : shared library 링크 테스트
* `test_dlopen.c` : run-time linking 테스트 (dlopen/dlsym)

---

## 3) 빌드 & 실행 (Makefile 없이 gcc 사용)

### 3.1 Static Library (.a) 빌드 + 테스트

```bash
gcc -c big.c -o big.o
ar rcs libbig.a big.o

gcc test_static.c ./libbig.a -o test_static
./test_static
```

### 3.2 Shared Library (.so) 빌드 + 테스트

```bash
gcc -c -fpic big.c -o big.o
gcc -shared -o libbig.so big.o

gcc test_shared.c -L. -lbig -o test_shared
LD_LIBRARY_PATH=. ./test_shared
```

### 3.3 Run-time linking (dlopen/dlsym) 테스트

```bash
gcc test_dlopen.c -ldl -o test_dlopen
./test_dlopen
```

---

## 4) 사용 예시 (test_static.c)

```c
#include <stdio.h>
#include <stdlib.h>
#include "big.h"

static void run(const char* a, const char* b) {
    char* r1 = big_add(a, b);
    char* r2 = big_sub(a, b);

    printf("%s + %s = %s\n", a, b, r1);
    printf("%s - %s = %s\n", a, b, r2);

    free(r1);
    free(r2);
}

int main(void) {
    run("999999999999999999999999", "1");
    run("-213215312256183218", "687454354350870978");
    return 0;
}
```

---

## 5) 스크린샷

터미널에서 아래 중 하나 이상 실행 결과가 보이도록 스크린샷 1장 이상 첨부:

* `./test_static`
* `LD_LIBRARY_PATH=. ./test_shared`
* `./test_dlopen`

