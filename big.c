#include "big.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int sign;        // +1 or -1 (zero는 항상 +1로 유지)
    size_t len;      // digits length (>=1)
    uint8_t* d;      // base10 digits, little-endian (d[0] = 1의 자리)
} Big;

static void big_init(Big* x) {
    x->sign = 1;
    x->len = 0;
    x->d = NULL;
}

static void big_free(Big* x) {
    free(x->d);
    x->d = NULL;
    x->len = 0;
    x->sign = 1;
}

static int big_is_zero(const Big* x) {
    return x->len == 1 && x->d[0] == 0;
}

static void big_trim(Big* x) {
    while (x->len > 1 && x->d[x->len - 1] == 0) {
        x->len--;
    }
    if (big_is_zero(x)) x->sign = 1; // -0 방지
}

static int big_set_zero(Big* x) {
    x->sign = 1;
    x->len = 1;
    x->d = (uint8_t*)malloc(1);
    if (!x->d) {
        errno = ENOMEM;
        return -1;
    }
    x->d[0] = 0;
    return 0;
}

// 입력 파싱: "+..." 금지, "-" 허용, 숫자만 허용.
// 성공: 0, 실패: -1 (errno 설정)
static int big_parse(Big* out, const char* s) {
    big_init(out);

    if (!s || s[0] == '\0') {
        errno = EINVAL;
        return -1;
    }
    if (s[0] == '+') { // 금지
        errno = EINVAL;
        return -1;
    }

    int sign = 1;
    size_t i = 0;
    if (s[0] == '-') {
        sign = -1;
        i = 1;
        if (s[i] == '\0') { // "-" 단독 금지
            errno = EINVAL;
            return -1;
        }
    }

    // 숫자만인지 체크 + leading zero 스킵
    size_t start = i;
    while (s[i] == '0') i++;

    size_t first_nonzero = i;

    for (; s[i] != '\0'; i++) {
        if (s[i] < '0' || s[i] > '9') {
            errno = EINVAL;
            return -1;
        }
    }

    // 모두 0인 경우 ("0", "000", "-000" 등)
    if (first_nonzero == i) {
        return big_set_zero(out);
    }

    size_t ndigits = i - first_nonzero;
    out->d = (uint8_t*)malloc(ndigits);
    if (!out->d) {
        errno = ENOMEM;
        return -1;
    }
    out->len = ndigits;
    out->sign = sign;

    // reverse 저장
    for (size_t k = 0; k < ndigits; k++) {
        out->d[k] = (uint8_t)(s[i - 1 - k] - '0');
    }

    big_trim(out);
    return 0;
}

static int big_cmp_mag(const Big* a, const Big* b) {
    if (a->len != b->len) return (a->len > b->len) ? 1 : -1;
    for (size_t i = a->len; i-- > 0;) {
        if (a->d[i] != b->d[i]) return (a->d[i] > b->d[i]) ? 1 : -1;
    }
    return 0;
}

static int big_add_mag(Big* out, const Big* a, const Big* b) {
    size_t n = (a->len > b->len) ? a->len : b->len;
    out->d = (uint8_t*)malloc(n + 1);
    if (!out->d) {
        errno = ENOMEM;
        return -1;
    }
    out->len = n + 1;
    out->sign = 1;

    uint16_t carry = 0;
    for (size_t i = 0; i < n; i++) {
        uint16_t av = (i < a->len) ? a->d[i] : 0;
        uint16_t bv = (i < b->len) ? b->d[i] : 0;
        uint16_t sum = av + bv + carry;
        out->d[i] = (uint8_t)(sum % 10);
        carry = (uint16_t)(sum / 10);
    }
    out->d[n] = (uint8_t)carry;

    big_trim(out);
    return 0;
}

// out = a - b (|a|>=|b| 가정), magnitude subtraction
static int big_sub_mag(Big* out, const Big* a, const Big* b) {
    out->d = (uint8_t*)malloc(a->len);
    if (!out->d) {
        errno = ENOMEM;
        return -1;
    }
    out->len = a->len;
    out->sign = 1;

    int borrow = 0;
    for (size_t i = 0; i < a->len; i++) {
        int av = (int)a->d[i];
        int bv = (i < b->len) ? (int)b->d[i] : 0;
        int diff = av - bv - borrow;
        if (diff < 0) {
            diff += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }
        out->d[i] = (uint8_t)diff;
    }

    big_trim(out);
    return 0;
}

static char* big_to_cstr(const Big* x) {
    if (big_is_zero(x)) {
        char* z = (char*)malloc(2);
        if (!z) {
            errno = ENOMEM;
            return NULL;
        }
        z[0] = '0';
        z[1] = '\0';
        return z;
    }

    size_t neg = (x->sign < 0) ? 1 : 0;
    size_t n = x->len + neg + 1;
    char* s = (char*)malloc(n);
    if (!s) {
        errno = ENOMEM;
        return NULL;
    }

    size_t pos = 0;
    if (neg) s[pos++] = '-';

    // high->low 출력
    for (size_t i = 0; i < x->len; i++) {
        s[pos + i] = (char)('0' + x->d[x->len - 1 - i]);
    }
    s[pos + x->len] = '\0';
    return s;
}


char* big_add(const char* a, const char* b) {
    Big A, B, R;
    big_init(&A); big_init(&B); big_init(&R);

    if (big_parse(&A, a) != 0) return NULL;
    if (big_parse(&B, b) != 0) { big_free(&A); return NULL; }

    int same = (A.sign == B.sign);

    if (same) {
        if (big_add_mag(&R, &A, &B) != 0) { big_free(&A); big_free(&B); return NULL; }
        R.sign = A.sign;
    } else {
        int c = big_cmp_mag(&A, &B);
        if (c == 0) {
            if (big_set_zero(&R) != 0) { big_free(&A); big_free(&B); return NULL; }
        } else if (c > 0) {
            if (big_sub_mag(&R, &A, &B) != 0) { big_free(&A); big_free(&B); return NULL; }
            R.sign = A.sign;
        } else {
            if (big_sub_mag(&R, &B, &A) != 0) { big_free(&A); big_free(&B); return NULL; }
            R.sign = B.sign;
        }
    }

    big_trim(&R);
    char* s = big_to_cstr(&R);

    big_free(&A); big_free(&B); big_free(&R);
    return s;
}

char* big_sub(const char* a, const char* b) {
    Big A, B, R;
    big_init(&A); big_init(&B); big_init(&R);

    if (big_parse(&A, a) != 0) return NULL;
    if (big_parse(&B, b) != 0) { big_free(&A); return NULL; }

    // a - b  ==  a + (-b)
    if (!big_is_zero(&B)) B.sign = -B.sign;

    // add 로직 재사용
    if (A.sign == B.sign) {
        if (big_add_mag(&R, &A, &B) != 0) { big_free(&A); big_free(&B); return NULL; }
        R.sign = A.sign;
    } else {
        int c = big_cmp_mag(&A, &B);
        if (c == 0) {
            if (big_set_zero(&R) != 0) { big_free(&A); big_free(&B); return NULL; }
        } else if (c > 0) {
            if (big_sub_mag(&R, &A, &B) != 0) { big_free(&A); big_free(&B); return NULL; }
            R.sign = A.sign;
        } else {
            if (big_sub_mag(&R, &B, &A) != 0) { big_free(&A); big_free(&B); return NULL; }
            R.sign = B.sign;
        }
    }

    big_trim(&R);
    char* s = big_to_cstr(&R);

    big_free(&A); big_free(&B); big_free(&R);
    return s;
}

