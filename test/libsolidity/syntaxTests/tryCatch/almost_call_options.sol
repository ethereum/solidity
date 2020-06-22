contract C {
    struct gas { uint a; }
    function f() public returns (uint, uint) {
        try this.f() {
            gas memory x;
        } catch Error(string memory) {

        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// Warning 2072: (122-134): Unused local variable.
