function nullSuffix(uint) pure suffix {}

contract C {
    function f() public pure {
        return 1 nullSuffix;
    }
}
// ----
// TypeError 7848: (38-38): Literal suffix functions must return exactly one value.
