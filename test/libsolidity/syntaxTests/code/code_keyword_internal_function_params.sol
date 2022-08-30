contract X {
    function foo(string code input) internal pure {
        string code test = input;
    }
    function g(string code) internal {}
    function i(string code str) internal {
        g(str);
    }
}
// ----
// SyntaxError 2397: (30-47): Usage of "code" as a data location is not yet supported.
// SyntaxError 2397: (73-89): Usage of "code" as a data location is not yet supported.
// SyntaxError 2397: (120-131): Usage of "code" as a data location is not yet supported.
// SyntaxError 2397: (160-175): Usage of "code" as a data location is not yet supported.
// TypeError 6651: (30-47): Data location must be "storage", "memory" or "calldata" for parameter in function, but "code" was given.
// TypeError 6651: (73-89): Data location must be "storage", "memory" or "calldata" for variable, but "code" was given.
// TypeError 6651: (120-131): Data location must be "storage", "memory" or "calldata" for parameter in function, but "code" was given.
// TypeError 6651: (160-175): Data location must be "storage", "memory" or "calldata" for parameter in function, but "code" was given.
