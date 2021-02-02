contract C {
    error E(uint a, uint8 b);
    function f() public returns (uint, uint) {
        try this.f() {

        } catch E(uint8 x, uint y) {

        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// TypeError 63958: (132-139): Expected a parameter of type "uint256" but got "uint8"
// TypeError 63958: (141-147): Expected a parameter of type "uint8" but got "uint256"
