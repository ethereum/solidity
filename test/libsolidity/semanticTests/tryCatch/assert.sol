contract C {
    function g(bool x) public pure {
        assert(x);
    }
    function f(bool x) public returns (uint) {
        try this.g(x) {
            return 1;
        } catch {
            return 2;
        }
    }
}
// ----
// f(bool): true -> 1
// f(bool): false -> 2
