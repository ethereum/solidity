contract D {
}
contract C {
    function f() public {
        try new D() {
        } catch (bytes memory x) {
            x;
        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
