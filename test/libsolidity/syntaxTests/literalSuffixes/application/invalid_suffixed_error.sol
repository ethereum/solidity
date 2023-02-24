function suffix(uint x) pure returns (uint) { return x; }

contract C {
    error E();

    function f() public {
        revert E suffix;
    }
}
// ----
// ParserError 2314: (131-137): Expected '(' but got identifier
