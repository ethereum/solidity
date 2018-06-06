contract test {
    function f() public {
        42 ** (-1/4);
    }
}
// ----
// TypeError: (50-62): Operator ** not compatible with types int_const 42 and rational_const -1 / 4
