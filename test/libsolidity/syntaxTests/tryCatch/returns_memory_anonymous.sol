contract C {
    function f() public returns (uint[] memory, uint) {
        try this.f() returns (uint[] memory, uint) {

        } catch {

        }
    }
}
// ====
// EVMVersion: >=byzantium