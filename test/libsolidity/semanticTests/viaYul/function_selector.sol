contract C {
    function f() external returns (bytes4) {
        return this.f.selector;
    }
    function h(function() external a) public returns (bytes4) {
        return a.selector;
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f() -> left(0x26121ff0)
// h(function): left(0x1122334400112233445566778899AABBCCDDEEFF42424242) -> left(0x42424242)
