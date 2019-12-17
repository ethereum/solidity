contract C {
    function f() public {
        try this.f() {

        } catch (bytes memory) {

        }
    }
}
// ====
// EVMVersion: <byzantium
// ----
// TypeError: (73-106): This catch clause type cannot be used on the selected EVM version (homestead). You need at least a Byzantium-compatible EVM or use `catch { ... }`.
