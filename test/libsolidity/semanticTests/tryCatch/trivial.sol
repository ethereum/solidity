contract C {
    function g(bool x) public pure {
        require(x);
    }
    function f(bool x) public returns (uint) {
        // Set the gas to make this work on pre-byzantium VMs
        try this.g.gas(8000)(x) {
            return 1;
        } catch {
            return 2;
        }
    }
}
// ----
// f(bool): true -> 1
// f(bool): false -> 2
