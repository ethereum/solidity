contract C {
    function f() public {
        bytes memory a;
        bytes memory b = type(bytes).concat(a);
    }
}
// ----
// TypeError 4259: (93-98): Invalid type for argument in the function call. An enum type, contract type or an integer type is required, but type(bytes storage pointer) provided.
