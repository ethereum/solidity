contract test1 {
    function test1() constant {}
}
contract test2 {
    function test2() view {}
}
contract test3 {
    function test3() pure {}
}
// ----
// Warning: (21-49): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// Warning: (73-97): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// Warning: (121-145): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// TypeError: (21-49): Constructor must be payable or non-payable, but is "view".
// TypeError: (73-97): Constructor must be payable or non-payable, but is "view".
// TypeError: (121-145): Constructor must be payable or non-payable, but is "pure".
