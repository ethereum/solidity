contract Test {
    event MyCustomEvent(uint256);

    function test() public returns(bytes4) {
        return (MyCustomEvent);
    }
}
// ----
// TypeError 6359: (111-126): Return argument type event MyCustomEvent(uint256) is not implicitly convertible to expected type (type of first return variable) bytes4.
