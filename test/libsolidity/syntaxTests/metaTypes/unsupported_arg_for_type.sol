contract Test {
    function f() public pure {
        // Unsupported for now, but might be supported in the future
        type(bytes32);
    }
}
// ----
// TypeError 4259: (154-155): Invalid type for argument in the function call. An enum type, contract type, struct type or an integer type is required, but type(struct Test.S storage pointer) provided.
