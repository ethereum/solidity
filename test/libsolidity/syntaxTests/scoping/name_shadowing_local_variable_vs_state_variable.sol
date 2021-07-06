contract test {
    uint256 variable;
    function f() pure public { uint32 variable; variable = 2; }
}
// ----
// Warning 2519: (69-84): This declaration shadows an existing declaration.
