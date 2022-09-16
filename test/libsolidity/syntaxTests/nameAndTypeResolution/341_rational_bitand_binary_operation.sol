contract test {
    function f() public {
        fixed(1.75) & 3;
    }
}
// ----
// TypeError 2271: (50-65): Built-in binary operator & cannot be applied to types fixed128x18 and int_const 3.
