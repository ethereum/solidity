error MyCustomError(uint, bool);
contract C {
    function f() public {
        bytes memory a;
        bytes memory b = type(MyCustomError).concat(a);
    }
}
// ----
// TypeError 4259: (126-139): Invalid type for argument in the function call. An enum type, contract type or an integer type is required, but error MyCustomError(uint256,bool) provided.
