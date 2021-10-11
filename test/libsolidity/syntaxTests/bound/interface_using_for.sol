library L {
    function f() public {}
}

interface I {
    using L for int;
    function g() external;
}
// ----
// SyntaxError 9088: (60-76): The "using for" directive is not allowed inside interfaces.
