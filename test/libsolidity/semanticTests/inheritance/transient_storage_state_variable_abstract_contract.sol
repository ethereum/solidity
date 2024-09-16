abstract contract A {
    uint transient x;
    int y;
    function f() public virtual returns (uint, int, uint, int);
}

contract C is A {
    uint w;
    int transient z;

    function g() public {
        w += 2;
        z += 2;
    }

    function f() public override returns (uint, int, uint, int) {
        x += 1;
        y += 1;
        g();
        return (x, y, w, z);
    }
}
// ====
// EVMVersion: >=cancun
// ----
// f() -> 1, 1, 2, 2
