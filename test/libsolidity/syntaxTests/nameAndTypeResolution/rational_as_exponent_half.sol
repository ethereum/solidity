contract test {
    function f() public {
        2 ** (1/2);
    }
}
// ----
// TypeError: (50-60): Operator ** not compatible with types int_const 2 and rational_const 1 / 2
