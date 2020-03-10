contract C {
    function f() external returns (address) {
        return this.f.address;
    }
    function g() external returns (bool) {
      return this.f.address == address(this);
    }
    function h(function() external a) public returns (address) {
      return a.address;
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 0x0fdd67305928fcac8d213d1e47bfa6165cd0b87b
// g() -> true
// h(function): left(0x1122334400112233445566778899AABBCCDDEEFF42424242) -> 0x1122334400112233445566778899AABBCCDDEEFF
