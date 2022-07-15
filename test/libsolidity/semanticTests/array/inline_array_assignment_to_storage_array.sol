contract C {
    uint8[3][] x;
    uint8[][] y;

    constructor() {
        x = new uint8[3][](1);
        x[0] = [1, 2, 3];

        y = new uint8[][](1);
        y[0] = [4, 5, 6];
    }

    function f() public returns (uint8[3] memory) {
        return x[0];
    }

    function g() public returns (uint8[] memory) {
        return y[0];
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 1, 2, 3
// g() -> 0x20, 3, 4, 5, 6
