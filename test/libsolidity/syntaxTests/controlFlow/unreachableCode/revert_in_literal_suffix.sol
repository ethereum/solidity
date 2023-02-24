function suffix(uint) pure suffix returns (uint) {
    revert();
}

contract C {
    function f() public pure {
        1 suffix;
        uint a = 0; // TODO: Should warn about unreachable code here
        a;
    }
}
