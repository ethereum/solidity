pragma solidity ^99.99.0;
contract C {
    function f() {}
}
// ----
// SyntaxError 3997: (0-25): Source file requires different compiler version (current compiler is ....
// SyntaxError 4937: (43-58): No visibility specified. Did you intend to add "public"?
