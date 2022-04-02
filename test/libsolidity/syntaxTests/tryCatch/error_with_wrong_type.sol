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
// TypeError 2943: (94-123='catch Error(uint) {         }'): Expected `catch Error(string memory ...) { ... }`.
