function suffix(uint) pure suffix returns (uint) {
    revert();
}

contract C {
    function f() public pure {
        1 suffix;
        uint a = 0;
        a;
    }
}
// ----
// Warning 5740: (138-159): Unreachable code.
