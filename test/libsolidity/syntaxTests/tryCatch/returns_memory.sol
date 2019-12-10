contract C {
    function f() public returns (uint[] memory, uint) {
        try this.f() returns (uint[] memory x, uint y) {
            return (x, y);
        } catch {

        }
    }
}
// ====
// EVMVersion: >=byzantium
