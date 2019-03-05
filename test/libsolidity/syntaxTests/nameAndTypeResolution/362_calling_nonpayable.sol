contract receiver { function nopay() public {} }
contract test {
    function f() public { (new receiver()).nopay.value(10)(); }
}
// ----
// TypeError: (91-119): Member "value" is only available for payable functions.
