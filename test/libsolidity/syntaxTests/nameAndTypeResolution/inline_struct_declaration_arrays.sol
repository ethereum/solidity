contract C {
    struct S {
        uint a;
        string b;
    }
    function f() {
        S[2] memory x = [S({a: 1, b: "fish"}), S({a: 2, b: "fish"})];
    }
}
// ----
// Warning: (72-162): No visibility specified. Defaulting to "public". 
// Warning: (95-108): Unused local variable.
// Warning: (72-162): Function state mutability can be restricted to pure
