contract C {
    function f() public returns (uint, uint) {
        try f() {
        } catch {
        }
    }
}
// ----
// TypeError: (72-75): Try can only be used with external function calls and contract creation calls.
