==== Source: A ====
uint[] storage a;
==== Source: B ====
uint[] memory b;
==== Source: C ====
uint[] calldata c;
// ----
// ParserError 2314: (A:7-14): Expected identifier but got 'storage'
// ParserError 2314: (B:7-13): Expected identifier but got 'memory'
// ParserError 2314: (C:7-15): Expected identifier but got 'calldata'
