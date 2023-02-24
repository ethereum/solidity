function x(uint v) pure suffix returns (uint) { return v; }

pragma solidity 1 x;
// ----
// ParserError 5333: (61-81): Source file requires different compiler version (current compiler is ....
