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
// compileToEOF: true
// EVMVersion: >=prague
// ----
// f() -> 0x1a7b7ed5ae36cd8c4f6da702d8409d6cf9bd1f6d
// g() -> true
// h(function): left(0x1122334400112233445566778899AABBCCDDEEFF42424242) -> 0x1122334400112233445566778899AABBCCDDEEFF
