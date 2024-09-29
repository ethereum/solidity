contract A {
    uint24 transient x;
    int24 y;
}

contract C is A {
    uint24 w;
    int24 transient z;

    function f() public returns (uint24, int24, uint24, int24) {
        x += 1;
        y += 2;
        w += 3;
        z += 4;

        return (x, y, w, z);
    }
}
// ====
// EVMVersion: >=cancun
// ----
// f() -> 1, 2, 3, 4
