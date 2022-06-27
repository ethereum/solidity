pragma abicoder               v2;

contract C {
    function get() public view returns (uint[][] memory) {}

    function test() public view returns (bool) {
        uint[][] memory x = this.get();
    }
}
// ====
// EVMVersion: <byzantium
// ----
// TypeError 9574: (166-196): Type inaccessible dynamic type is not implicitly convertible to expected type uint256[][] memory.
