contract test {
    function f() public returns (uint d) { return 2 ** 10000000000; }
}
// ----
// TypeError 2271: (66-82): Binary operator ** not compatible with types int_const 2 and int_const 10000000000.
