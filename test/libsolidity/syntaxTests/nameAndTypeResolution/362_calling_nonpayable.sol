contract receiver { function nopay() public {} }
contract test {
    function f() public { (new receiver()).nopay{value: 10}(); }
    function g() public { (new receiver()).nopay.value(10)(); }
}
// ----
// TypeError 7006: (91-124): Cannot set option "value" on a non-payable function type.
// TypeError 8820: (156-184): Member "value" is only available for payable functions.
