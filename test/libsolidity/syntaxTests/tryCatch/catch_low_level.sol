contract C {
    function f() public returns (uint, uint) {
        try this.f() {

        } catch (bytes memory x) {
            x;
        }
    }
}
// ====
// EVMVersion: >=byzantium