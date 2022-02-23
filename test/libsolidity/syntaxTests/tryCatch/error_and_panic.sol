contract C {
    function f() public {
        try this.f() {
        } catch Error(string memory) {
        } catch Panic(uint) {
        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
