contract Test {
    function f() public pure {
        type(type(type(Test)));
    }
}
// ----
// TypeError 4259: (65-75): Invalid type for argument in the function call. An enum type, contract type or an integer type is required, but type(contract Test) provided.
