contract Test {
    function f() public pure {
        type(type(type(Test)));
    }
}
// ----
// TypeError: (65-75): Invalid type for argument in function call. Contract type required, but type(contract Test) provided.
// TypeError: (60-76): Invalid type for argument in function call. Contract type required, but tuple() provided.
