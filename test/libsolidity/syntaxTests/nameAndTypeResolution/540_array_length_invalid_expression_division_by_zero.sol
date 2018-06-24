contract C {
    uint[3/0] ids;
}
// ----
// TypeError: (22-25): Operator / not compatible with types int_const 3 and int_const 0
