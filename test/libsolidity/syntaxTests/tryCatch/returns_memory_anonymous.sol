contract C {
    function f() public returns (uint[] memory, uint) {
        try this.f() returns (uint[] memory, uint) {

        } catch {

        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// Warning 6321: (46-59): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6321: (61-65): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
