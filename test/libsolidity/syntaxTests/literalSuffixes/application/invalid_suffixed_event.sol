function suffix(uint x) pure suffix returns (uint) { return x; }

contract C {
    event E();

    function f() public {
        emit E suffix;
    }
}
// ----
// ParserError 2314: (136-142): Expected '(' but got identifier
