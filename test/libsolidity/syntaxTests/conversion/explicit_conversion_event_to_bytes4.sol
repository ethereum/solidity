contract Test {
    event MyCustomEvent(uint256);

    function test() public returns(bytes4) {
        return bytes4(MyCustomEvent);
    }
}
// ----
// TypeError 9640: (111-132): Explicit type conversion not allowed from "event MyCustomEvent(uint256)" to "bytes4".
