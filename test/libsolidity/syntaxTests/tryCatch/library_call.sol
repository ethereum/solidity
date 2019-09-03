library L {
    struct S { uint x; }
    function integer(uint t, bool b) public pure returns (uint) {
        if (b) {
            return t;
        } else {
            revert("failure");
        }
    }
    function stru(S storage t, bool b) public view returns (uint) {
        if (b) {
            return t.x;
        } else {
            revert("failure");
        }
    }
}
contract C {
    using L for L.S;
    L.S t;
    function f(bool b) public pure returns (uint, string memory) {
        uint x = 8;
        try L.integer(x, b) returns (uint _x) {
            return (_x, "");
        } catch Error(string memory message) {
            return (18, message);
        }
    }
    function g(bool b) public returns (uint, string memory) {
        t.x = 9;
        try t.stru(b) returns (uint x) {
            return (x, "");
        } catch Error(string memory message) {
            return (19, message);
        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
