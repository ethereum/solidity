contract C {
    function g(bool b) public pure returns (uint x, uint y) {
        require(b);
        return (1, 2);
    }
    function f(bool flag) public view returns (uint x, uint y) {
        try this.g(flag) returns (uint a, uint b) {
            (x, y) = (a, b);
        } catch {
            (x, y) = (9, 10);
        }
    }
}
// ====
// EVMVersion: >=byzantium
// compileViaYul: also
// ----
// f(bool): true -> 1, 2
// f(bool): false -> 9, 10
