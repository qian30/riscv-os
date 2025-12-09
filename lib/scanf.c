#include <stdarg.h>

extern int uart_putc(char);
extern void uart_puts(char *);
extern char uart_getc();

static void read_from_uart(char *buf, int maxlen)
{
    int pos = 0;
    while (pos < maxlen - 1) {
        char ch = uart_getc();

        if (ch == ' ') {
            uart_putc(' ');
            break;
        } else if (ch == '\n' || ch == '\r') {
            uart_putc('\n');
            break;
        } else if (ch == 0x7f || ch == '\b') {
            uart_puts("\b \b");
            if (pos > 0)
                pos--;
        } else {
            buf[pos] = ch; /* 把輸入的字存進 buf */
            pos++;
            uart_putc(ch); /* 把輸入的字，印在螢幕上 */
        }
    }
    buf[pos] = '\0';
}

static void parse_str(char *str)
{
    read_from_uart(str, 128);
}

static void parse_int(int *ptr)
{
    char buf[128];
    read_from_uart(buf, 128);
    int sign = 1;
    int val = 0;
    int idx = 0;
    if (buf[idx] == '-') {
        sign = -1;
        idx++;
    }
    while (buf[idx] != '\0') {
        val = val * 10 + (buf[idx] - '0');
        idx++;
    }
    *ptr = val * sign;
}

static int _vscanf(const char *fmt, va_list ap)
{
    int read_cnt = 0;
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;

            /* 偵測到 %s : 處理字串 */
            if (*fmt == 's') {
                char *str = va_arg(ap, char *);
                parse_str(str);
                read_cnt++;
            }
            /* 偵測到 %d : 處理數字 */
            else if (*fmt == 'd') {
                int *p = va_arg(ap, int *);
                parse_int(p);
                read_cnt++;
            }
        }
        fmt++;
    }
    return read_cnt;
}

int kscanf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int read_cnt = _vscanf(fmt, ap);

    va_end(ap);
    return read_cnt;
}