contract C {
    function g(bool x) external pure {
        require(x);
    }
    function f(bool x) public returns (uint) {
        try this.g(x) {
            return 1;
        } catch {
            return 2;
        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// f(bool): true -> 1
// f(bool): false -> 2
