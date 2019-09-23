contract C {
    function g(bool b) public pure returns (uint, uint) {
        require(b);
        return (1, 2);
    }
    function f(bool b) public returns (uint x, uint y) {
        try this.g(b) returns (uint a, uint b) {
            (x, y) = (a, b);
        } catch {
            (x, y) = (9, 10);
        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// f(bool): true -> 1, 2
// f(bool): false -> 9, 10
