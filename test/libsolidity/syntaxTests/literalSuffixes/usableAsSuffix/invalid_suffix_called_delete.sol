function delete(uint) pure suffix returns (uint) {}

contract C {
    uint x = 1 delete;
}
// ----
// ParserError 2314: (9-15): Expected identifier but got 'delete'
