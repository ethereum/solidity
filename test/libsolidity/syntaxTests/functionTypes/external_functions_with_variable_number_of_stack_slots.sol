contract C {
    function f (address) external returns (bool) {
        this.f{gas: 42}.address;
    }
}
// ----
// Warning 6321: (56-60): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 2018: (17-102): Function state mutability can be restricted to view
