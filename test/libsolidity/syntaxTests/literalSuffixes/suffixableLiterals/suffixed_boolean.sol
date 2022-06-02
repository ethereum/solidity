function suffix(bool x) pure returns (bool) {}

contract C {
    function f() public pure {
        true suffix;
        false suffix;
    }
}
