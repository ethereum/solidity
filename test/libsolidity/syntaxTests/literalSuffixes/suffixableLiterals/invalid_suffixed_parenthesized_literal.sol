function suffix(uint) pure suffix returns (uint) {}

contract C {
    function f() public {
        (1) suffix;
    }
}
// ----
// ParserError 2314: (104-110): Expected ';' but got identifier
