contract C {
    function f() public {
        try this.f() {

        } catch Error(string memory) {

        }
    }
}
// ====
// EVMVersion: <byzantium
// ----
// TypeError: (73-112): This catch clause type cannot be used on the selected EVM version (homestead). You need at least a Byzantium-compatible EVM or use `catch { ... }`.
