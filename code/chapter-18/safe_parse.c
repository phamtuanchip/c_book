// safe_parse.c
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PAYLOAD 4096

typedef struct {
    uint8_t  type;
    uint16_t len;                       // độ dài payload do đối phương khai
    uint8_t  payload[MAX_PAYLOAD];
} Message;

typedef enum { P_OK = 0, P_SHORT, P_TOO_BIG, P_BAD_TYPE } ParseResult;

// data/size: gói tin nhận được (KHÔNG tin cậy). Trả kết quả phân tích vào *out.
ParseResult parse_message(const uint8_t *data, size_t size, Message *out) {
    memset(out, 0, sizeof *out);                   // khởi tạo sạch, không để lộ byte cũ

    if (size < 3) return P_SHORT;                  // cần ít nhất header 3 byte: type(1) + len(2)

    uint8_t  type = data[0];
    uint16_t len  = (uint16_t)((data[1] << 8) | data[2]);     // big-endian, đọc từng byte (không ép kiểu con trỏ)

    if (type > 3)            return P_BAD_TYPE;    // allow-list các giá trị hợp lệ
    if (len > MAX_PAYLOAD)   return P_TOO_BIG;     // giới hạn cứng
    if ((size_t)len > size - 3) return P_SHORT;    // độ dài khai báo KHÔNG được vượt quá dữ liệu thực có
                                                   // (viết size - 3 thay vì 3 + len để tránh tràn)
    out->type = type;
    out->len  = len;
    memcpy(out->payload, data + 3, len);
    return P_OK;
}

int main(void) {
    uint8_t good[] = { 1, 0x00, 0x05, 'h', 'e', 'l', 'l', 'o' };
    uint8_t liar[] = { 1, 0xFF, 0xFF, 'x' };       // khai 65535 byte nhưng chỉ có 1
    Message m;
    printf("good: %d\n", parse_message(good, sizeof good, &m));   // 0
    printf("liar: %d\n", parse_message(liar, sizeof liar, &m));   // 2 (P_TOO_BIG) chặn ngay
    return 0;
}
