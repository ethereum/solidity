contract C {
    function f() public returns (uint, uint) {
        try this.f() returns (uint a) {
            a = 1;
        } catch {

        }
    }
}
// ----
// TypeError 2800: (81-128): Function returns 2 values, but returns clause has 1 variables.
