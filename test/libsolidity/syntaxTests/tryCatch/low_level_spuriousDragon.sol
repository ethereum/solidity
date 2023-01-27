contract C {
    function f() public {
        try this.f() {

        } catch (bytes memory) {

        }
    }
}
// ====
// EVMVersion: =spuriousDragon
// ----
// TypeError 9908: (73-106): This catch clause type cannot be used on the selected EVM version (spuriousDragon). You need at least a Byzantium-compatible EVM or use `catch { ... }`.
