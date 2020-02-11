// This tests that the compiler knows the correct size of the function on the stack.
contract c {
    uint[] data;

    function test() public returns(uint x) {
        x = 2;
        data.pop;
        x = 3;
    }
}

// ----
// test() -> 3
// test():"" -> "3"
