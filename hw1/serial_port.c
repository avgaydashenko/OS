#include "serial_port.h"

void serial_port_init() {

	out8(LINE_CONTROL_REGISTER, 3); // 3_{10} = 11_{2}, т.е. биты 0 и 1 LCR теперь
									// равны 1 => 8 бит данных в формате кадра
	out8(INTERRUPT_ENABLE_REGISTER, 0);
}

void serial_port_write_char(char c) {
	
	while (((in8(LINE_STATUS_REGISTER) >> 5) & 1) == 0) {}; // если пятый бит LST
		// установлен, значит, можно писать следующий байт, иначе нужно ждать
	out8(DATA_REGISTER, c);
}

void serial_port_write_line(char* s) {
	for (; *s != 0; ++s)
		serial_port_write_char(*s);
}

void serial_port_write_num(uint64_t x) {

	if (x == 0) {
        serial_port_write_line("0");
        return;
    }

    char str[30];
    char res[30];
    int i = 0;
    while (x > 0) {
        str[i] = x % 10 + '0';
        x /= 10;
        ++i;
    }
    for (int j = 0; j < i; j++) {
        res[j] = str[i - j - 1];
    }
    res[i] = 0;
    serial_port_write_line(res);
}