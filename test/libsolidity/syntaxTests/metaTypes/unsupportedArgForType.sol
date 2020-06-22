contract Test {
    struct S { uint x; }
    function f() public pure {
        // Unsupported for now, but might be supported in the future
        type(S);
    }
}
// ----
// TypeError 4259: (154-155): Invalid type for argument in the function call. A contract type or an integer type is required, but type(struct Test.S) provided.
