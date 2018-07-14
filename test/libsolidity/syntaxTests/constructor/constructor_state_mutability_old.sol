contract test1 {
    function test1() public view {}
}
contract test2 {
    function test2() public pure {}
}
// ----
// SyntaxError: (21-52): Functions are not allowed to have the same name as the contract. If you intend this to be a constructor, use "constructor(...) { ... }" to define it.
// SyntaxError: (76-107): Functions are not allowed to have the same name as the contract. If you intend this to be a constructor, use "constructor(...) { ... }" to define it.
// TypeError: (21-52): Constructor must be payable or non-payable, but is "view".
// TypeError: (76-107): Constructor must be payable or non-payable, but is "pure".
