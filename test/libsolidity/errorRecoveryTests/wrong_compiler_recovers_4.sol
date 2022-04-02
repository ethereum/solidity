pragma solidity ^99.99.0;
contract C {
    function f() {}
}
// ----
// SyntaxError 3997: (0-25='pragma solidity ^99.99.0;'): Source file requires different compiler version (current compiler is ....
// SyntaxError 4937: (43-58='function f() {}'): No visibility specified. Did you intend to add "public"?
