contract C {
    function f() public returns (uint, uint) {
        try this.f() {

        } catch Error(string memory x) {
            x;
        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// Warning 6321: (46-50): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6321: (52-56): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
