void arg_parse(const char* restrict* fmt, va_list* args) {
    if ((*fmt)[1] == 'c') {
        char c = va_arg(*args, int);
        write(STDOUT_FILENO, &c, 1);
        *fmt += 2;
    }
}

int r_printf(const char* restrict fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int i = 0;
    while (fmt[i] != '\0') {
        if (fmt[i] == '%') {
            // arg parse
            write(STDOUT_FILENO, fmt, i);
            fmt += i;
            arg_parse(&fmt, &args);
            i = 0;
        } else {
            i++;
        }
    }
    write(STDOUT_FILENO, fmt, i);
    va_end(args);
    return 0;
}
