contract C {
    error E(uint x);
    function f() public returns (uint, uint) {
        try this.f() {

        } catch E() {

        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// TypeError 4873: (115-137): Expected 1 parameters for error "E" but got 0.
