contract Test {
    function f() public pure {
        // Unsupported for now, but might be supported in the future
        type(bytes32);
    }
}
// ----
// TypeError 4259: (129-136): Invalid type for argument in the function call. An enum type, contract type, struct type or an integer type is required, but type(bytes32) provided.
