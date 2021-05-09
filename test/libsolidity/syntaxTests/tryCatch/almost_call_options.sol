contract C {
    struct gas { uint a; }
    function f() public returns (uint, uint) {
        try this.f() {
            gas memory x;
        } catch Error(string memory) {

        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// Warning 6321: (73-77): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6321: (79-83): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 2072: (122-134): Unused local variable.
