function suffix(uint x) pure suffix returns (uint) { return x; }

contract C {
    function f() pure public {
        assembly {
            pop(0 suffix)
        }
    }
}
// ----
// ParserError 2314: (147-153): Expected ',' but got identifier
