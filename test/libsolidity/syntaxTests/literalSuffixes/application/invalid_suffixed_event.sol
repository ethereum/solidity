function suffix(uint x) pure returns (uint) { return x; }

contract C {
    event E();

    function f() public {
        emit E suffix;
    }
}
// ----
// ParserError 2314: (129-135): Expected '(' but got identifier
