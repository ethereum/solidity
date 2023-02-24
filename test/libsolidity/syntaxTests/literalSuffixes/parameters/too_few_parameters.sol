function zero() pure suffix returns (uint) { return 1; }

contract C {
    function f() public pure {
        1 zero;   // No error here, only at suffix definition.
        1.1 zero; // No error here, only at suffix definition.
        "a" zero; // No error here, only at suffix definition.
    }
}
// ----
// TypeError 9128: (13-15): Only functions that take one or two arguments can be used as literal suffixes.
