contract test1 {
    function test1() public constant {}
}
contract test2 {
    function test2() public view {}
}
contract test3 {
    function test3() public pure {}
}
// ----
// Warning: (21-56): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// Warning: (80-111): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// Warning: (135-166): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// TypeError: (21-56): Constructor must be payable or non-payable, but is "view".
// TypeError: (80-111): Constructor must be payable or non-payable, but is "view".
// TypeError: (135-166): Constructor must be payable or non-payable, but is "pure".
