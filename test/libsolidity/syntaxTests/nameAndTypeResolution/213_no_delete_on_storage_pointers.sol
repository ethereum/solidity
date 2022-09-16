contract C {
    uint[] data;
    function f() public {
        uint[] storage x = data;
        delete x;
    }
}
// ----
// TypeError 9767: (97-105): Built-in unary operator delete cannot be applied to type uint256[] storage pointer.
