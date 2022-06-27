contract C {
    event MyCustomEvent(uint);
    function f() public {
        bytes memory a;
        bytes memory b = type(MyCustomEvent).concat(a);
    }
}
// ----
//  TypeError 4259: (124-137): Invalid type for argument in the function call. An enum type, contract type or an integer type is required, but event MyCustomEvent(uint256) provided.
