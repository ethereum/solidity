contract test1 {
    function test1() view {}
}
contract test2 {
    function test2() pure {}
}
// ----
// Warning: (21-45): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// Warning: (69-93): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// TypeError: (21-45): Constructor must be payable or non-payable, but is "view".
// TypeError: (69-93): Constructor must be payable or non-payable, but is "pure".
