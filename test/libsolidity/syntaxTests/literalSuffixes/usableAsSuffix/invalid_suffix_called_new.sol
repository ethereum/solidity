function new(uint) pure suffix returns (uint) {}

contract C {
    uint x = 1 new;
}
// ----
// ParserError 2314: (9-12): Expected identifier but got 'new'
