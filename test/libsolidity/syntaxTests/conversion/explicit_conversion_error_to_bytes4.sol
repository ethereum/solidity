interface MyInterface {
    error MyCustomError(uint256, bool);
}

contract Test {
    function test() public returns(bytes4) {
        return bytes4(MyInterface.MyCustomError);
    }
}
// ----
// TypeError 9640: (143-176): Explicit type conversion not allowed from "error MyCustomError(uint256,bool)" to "bytes4".
