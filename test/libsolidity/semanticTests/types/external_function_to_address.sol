contract C {
    function f() public returns (bool) {
        return this.f.address == address(this);
    }
    function g(function() external cb) public returns (address) {
        return cb.address;
    }
}
// ----
// f() -> true
// g(function): hex"00000000000000000000000000000000000004226121ff00000000000000000" -> 0x42
