library L { function l() public {} }
contract test {
    function f() public {
        L.l.value;
    }
}
// ----
// TypeError: (87-96): Member "value" is not allowed in delegated calls due to "msg.value" persisting.
