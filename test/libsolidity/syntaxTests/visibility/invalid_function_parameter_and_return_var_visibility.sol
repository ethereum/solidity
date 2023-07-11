==== Source: A ====
contract test {
    function f(uint external a) public returns (uint) { }
}
==== Source: B ====
contract test {
    function f(uint internal a) public returns (uint) { }
}
==== Source: C ====
contract test {
    function f(uint public a) public returns (uint) { }
}
==== Source: D ====
contract test {
    function f(uint private a) public returns (uint) { }
}
==== Source: E ====
contract test {
    function f(uint a) public returns (uint external) { }
}
==== Source: F ====
contract test {
    function f(uint a) public returns (uint internal) { }
}
==== Source: G ====
contract test {
    function f(uint a) public returns (uint public) { }
}
==== Source: H ====
contract test {
    function f(uint a) public returns (uint private) { }
}
// ----
// ParserError 2314: (A:36-44): Expected ',' but got 'external'
// ParserError 2314: (B:36-44): Expected ',' but got 'internal'
// ParserError 2314: (C:36-42): Expected ',' but got 'public'
// ParserError 2314: (D:36-43): Expected ',' but got 'private'
// ParserError 2314: (E:60-68): Expected ',' but got 'external'
// ParserError 2314: (F:60-68): Expected ',' but got 'internal'
// ParserError 2314: (G:60-66): Expected ',' but got 'public'
// ParserError 2314: (H:60-67): Expected ',' but got 'private'
