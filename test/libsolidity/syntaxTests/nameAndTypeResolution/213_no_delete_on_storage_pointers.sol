contract C {
    uint[] data;
    function f() public {
        uint[] storage x = data;
        delete x;
    }
}
// ----
// TypeError 9767: (97-105='delete x'): Unary operator delete cannot be applied to type uint256[] storage pointer
