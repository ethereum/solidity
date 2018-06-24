contract test {
    function f() public {
        fixed g = 2 ** -2.2;
    }
}
// ----
// TypeError: (60-69): Operator ** not compatible with types int_const 2 and rational_const -11 / 5
