contract C {
    function g(bool b) public pure returns (uint, uint) {
        require(b, "message longer than 32 bytes 32 bytes 32 bytes 32 bytes 32 bytes 32 bytes 32 bytes");
        return (1, 2);
    }
    function f(bool cond) public returns (uint x, uint y, bytes memory txt) {
        try this.g(cond) returns (uint a, uint b) {
            (x, y) = (a, b);
            txt = "success";
        } catch Error(string memory s) {
            x = 99;
            txt = bytes(s);
        } catch (bytes memory s) {
            x = 98;
            txt = s;
        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// f(bool): true -> 1, 2, 96, 7, "success"
// f(bool): false -> 99, 0, 96, 82, "message longer than 32 bytes 32 ", "bytes 32 bytes 32 bytes 32 bytes", " 32 bytes 32 bytes"
