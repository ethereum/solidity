contract Test {
    struct S { uint x; }
    function f() public pure {
        // Unsupported for now, but might be supported in the future
        type(S);
    }
}
// ----
// TypeError: (154-155): Invalid type for argument in function call. Contract type required, but type(struct Test.S) provided.
