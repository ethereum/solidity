// This tests a swap in storage which does not work as one
// might expect because we do not have temporary storage.
// (x, y) = (y, x) is the same as
// y = x;
// x = y;
contract c {
    struct S {
        uint256 a;
        uint256 b;
    }
    S public x;
    S public y;

    function set() public {
        x.a = 1;
        x.b = 2;
        y.a = 3;
        y.b = 4;
    }

    function swap() public {
        (x, y) = (y, x);
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// x() -> 0, 0
// y() -> 0, 0
// set() ->
// gas irOptimized: 109733
// gas legacy: 109732
// gas legacyOptimized: 109682
// x() -> 1, 2
// y() -> 3, 4
// swap() ->
// x() -> 1, 2
// y() -> 1, 2
