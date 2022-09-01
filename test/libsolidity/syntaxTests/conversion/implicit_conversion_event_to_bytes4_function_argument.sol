contract MyContract {
    event MyCustomEvent(uint256);
    function f(bytes4 arg) public {}
    function test() public {
        f(MyCustomEvent);
    }
}
// ----
// TypeError 9553: (132-145): Invalid type for argument in function call. Invalid implicit conversion from event MyCustomEvent(uint256) to bytes4 requested.
