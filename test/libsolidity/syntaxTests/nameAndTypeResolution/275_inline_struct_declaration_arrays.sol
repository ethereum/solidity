contract C {
    struct S {
        uint a;
        string b;
    }
    function f() public {
        S[2] memory x = [S({a: 1, b: "fish"}), S({a: 2, b: "fish"})];
    }
}
// ----
// Warning: (102-115): Unused local variable.
// Warning: (72-169): Function state mutability can be restricted to pure
