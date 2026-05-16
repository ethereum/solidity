contract A layout at uint(0xff) {
    int[uint(0x0000000000000001)] a;
    int[uint(0xAbCdEf123456)] b;
    int[uint(0xff_ff)] c;
}
// ----
