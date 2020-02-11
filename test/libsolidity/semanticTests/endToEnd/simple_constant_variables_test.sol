contract Foo {
    function getX() public returns(uint r) {
        return x;
    }
    uint constant x = 56;
}

// ----
// getX() -> 56
