contract X {
    function bar(string code input) external pure {
        string code test = input;
    }
    function g(string code) external {}
    function i(string code str) external {
        this.g(str);
    }
}
// ----
// SyntaxError 2397: (30-47): Usage of "code" as a data location is not yet supported.
// SyntaxError 2397: (73-89): Usage of "code" as a data location is not yet supported.
// SyntaxError 2397: (120-131): Usage of "code" as a data location is not yet supported.
// SyntaxError 2397: (160-175): Usage of "code" as a data location is not yet supported.
// TypeError 6651: (30-47): Data location must be "memory" or "calldata" for parameter in external function, but "code" was given.
// TypeError 6651: (73-89): Data location must be "storage", "memory" or "calldata" for variable, but "code" was given.
// TypeError 6651: (120-131): Data location must be "memory" or "calldata" for parameter in external function, but "code" was given.
// TypeError 6651: (160-175): Data location must be "memory" or "calldata" for parameter in external function, but "code" was given.
