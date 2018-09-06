contract C {
    function constructor() public;
}
// ----
// ParserError: (26-37): This function is named "constructor" but is not the constructor of the contract. If you intend this to be a constructor, use "constructor(...) { ... }" without the "function" keyword to define it.
