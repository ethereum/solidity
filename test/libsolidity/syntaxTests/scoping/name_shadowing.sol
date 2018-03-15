contract test {
    uint256 variable;
    function f() pure public { uint32 variable; variable = 2; }
}
// ----
// Warning: This declaration shadows an existing declaration.

