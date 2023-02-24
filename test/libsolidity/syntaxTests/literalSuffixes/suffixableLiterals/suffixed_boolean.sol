function suffix(bool x) pure suffix returns (bool) {}

contract C {
    function f() public pure {
        true suffix;
        false suffix;
    }
}
