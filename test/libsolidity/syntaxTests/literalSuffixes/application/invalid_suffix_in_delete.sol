function suffix(string memory s) pure suffix returns (string memory) { return s; }

contract C {
    function f() public pure {
        delete "abc" suffix;
    }
}
// ----
// TypeError 4247: (143-155): Expression has to be an lvalue.
