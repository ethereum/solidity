function suffix(uint x) pure suffix returns (uint) { return x; }

contract C {
    function f() public pure {
        suffix;
    }
}
