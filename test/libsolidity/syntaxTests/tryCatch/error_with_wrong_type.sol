contract C {
    function f() public returns (uint, uint) {
        try this.f() {

        } catch Error(uint) {
        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// TypeError: (94-123): Expected `catch Error(string memory ...) { ... }`.
