function suffix(uint x) pure suffix returns (uint) { return x; }

contract C {
    function f() pure public {
        assembly ("memory-safe" suffix) {
            pop(0)
        }
    }
}
// ----
// ParserError 2314: (142-148): Expected ')' but got identifier
