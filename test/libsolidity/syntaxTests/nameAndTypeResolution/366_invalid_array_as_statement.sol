contract test {
    struct S { uint x; }
    constructor(uint k) public { S[k]; }
}
// ----
// TypeError: (76-77): Integer constant expected.
