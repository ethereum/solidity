contract C {
    function f() public returns (uint, uint) {
        try this.f() {

        } catch (uint) {
        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// TypeError: (94-118): Expected `catch (bytes memory ...) { ... }` or `catch { ... }`.
