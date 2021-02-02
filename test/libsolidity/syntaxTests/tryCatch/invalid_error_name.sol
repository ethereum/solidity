contract C {
    function f() public returns (uint, uint) {
        try this.f() {
        } catch Error2() {
        } catch abc() {
        }
    }
}
// ----
// DeclarationError 7920: (99-105): Identifier not found or not unique.
