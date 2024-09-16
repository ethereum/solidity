contract A {
    uint transient x;
    int y;
}

contract C is A {
    uint w;
    int transient z;

    function f() public returns (uint, int, uint, int) {
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
