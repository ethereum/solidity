function suffix(uint, uint) pure suffix returns (uint) {}

contract C {
    function f() public {
        (1, 2) suffix;
    }
}
// ----
// ParserError 2314: (113-119): Expected ';' but got identifier
