contract test {
    function f() public {
        fixed(1.75) & 3;
    }
}
// ----
// TypeError: (50-65): Operator & not compatible with types fixed128x18 and int_const 3
