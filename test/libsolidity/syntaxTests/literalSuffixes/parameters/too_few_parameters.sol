function zero() pure suffix returns (uint) { return 1; }

contract C {
    function f() public pure {
        1 zero;
        1.1 zero;
        "a" zero;
    }
}
// ----
// TypeError 9128: (13-15): Functions that take no arguments cannot be used as literal suffixes.
