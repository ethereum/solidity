contract test1 {
    function test1() public view {}
}
contract test2 {
    function test2() public pure {}
}
// ----
// Warning: (21-52): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// Warning: (76-107): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// TypeError: (21-52): Constructor must be payable or non-payable, but is "view".
// TypeError: (76-107): Constructor must be payable or non-payable, but is "pure".
