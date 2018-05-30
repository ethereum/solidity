contract test {
    struct S { uint x; }
    function test(uint k) public { S[k]; }
}
// ----
// Warning: (45-83): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// TypeError: (78-79): Integer constant expected.
