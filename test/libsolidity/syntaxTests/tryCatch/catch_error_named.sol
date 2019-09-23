contract C {
    function f() public returns (uint, uint) {
        try this.f() {

        } catch Error(string memory x) {
            x;
        }
    }
}
// ====
// EVMVersion: >=byzantium