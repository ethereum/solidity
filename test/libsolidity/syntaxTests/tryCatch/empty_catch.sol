contract C {
    function f() public returns (uint, uint) {
        try this.f() {

        } catch () {

        }
    }
}
// ----
// TypeError 6231: (94-115): Expected `catch (bytes memory ...) { ... }` or `catch { ... }`.
