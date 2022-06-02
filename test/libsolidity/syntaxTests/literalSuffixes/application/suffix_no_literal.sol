function suffix(uint x) pure returns (uint) { return x; }

contract C {
    function f() public pure {
        suffix;
    }
}
