function suffix(uint) pure suffix returns (function () external) {}

contract C {
    address x = 1000 suffix.address;
}
// ----
// ParserError 2314: (110-117): Expected identifier but got 'address'
