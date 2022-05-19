contract C {
    function g(bool b) public pure returns (uint, uint) {
        require(b, "message");
        return (1, 2);
    }
    function f(bool b) public returns (uint x, uint y, bytes memory txt) {
        try this.g(b) returns (uint a, uint b) {
            (x, y) = (a, b);
        } catch (bytes memory s) {
            txt = s;
        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// f(bool): true -> 1, 2, 96, 0
// f(bool): false -> 0, 0, 96, 100, 0x8c379a000000000000000000000000000000000000000000000000000000000, 0x2000000000000000000000000000000000000000000000000000000000, 0x76d657373616765000000000000000000000000000000000000000000, 0
