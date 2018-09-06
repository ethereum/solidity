contract test {
    function f() public {
        ufixed c = 42 ** fixed(-1/4);
    }
}
// ----
// TypeError: (61-78): Operator ** not compatible with types int_const 42 and fixed128x18
