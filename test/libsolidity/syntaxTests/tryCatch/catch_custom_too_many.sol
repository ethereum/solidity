contract C {
    error E();
    function f() public returns (uint, uint) {
        try this.f() {

        } catch E(uint a) {

        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// TypeError 4873: (109-137): Expected 0 parameters for error "E" but got 1.
