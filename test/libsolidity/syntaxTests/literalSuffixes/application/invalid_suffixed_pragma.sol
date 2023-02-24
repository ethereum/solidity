function x(uint v) pure suffix returns (uint) { return v; }

pragma solidity >= 0 x;
// ----
// ParserError 5333: (61-84): Source file requires different compiler version (current compiler is ....
