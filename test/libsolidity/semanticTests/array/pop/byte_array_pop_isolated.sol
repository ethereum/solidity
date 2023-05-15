// This tests that the compiler knows the correct size of the function on the stack.
contract c {
    bytes data;

    function test() public returns (uint256 x) {
        x = 2;
        data.pop;
        x = 3;
    }
}
// ----
// test() -> 3
