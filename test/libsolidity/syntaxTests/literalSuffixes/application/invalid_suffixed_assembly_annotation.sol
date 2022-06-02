function suffix(uint x) pure returns (uint) { return x; }

contract C {
    function f() pure public {
        assembly ("memory-safe" suffix) {
            pop(0)
        }
    }
}
// ----
// ParserError 2314: (135-141): Expected ')' but got identifier
