function suffix(uint, uint) pure returns (uint) {}

contract C {
    function f() public {
        (1, 2) suffix;
    }
}
// ----
// ParserError 2314: (106-112): Expected ';' but got identifier
