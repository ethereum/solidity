pragma abicoder               v2;
contract C {
    bytes[] a;

    function f() public {
        a.push("abc");
        a.push("abcdefghabcdefghabcdefghabcdefgh");
        a.push("abcdefghabcdefghabcdefghabcdefghabcdefghabcdefghabcdefghabcdefgh");
        assert(a[0][0] == "a");
        assert(a[1][31] == "h");
        assert(a[2][32] == "a");
    }
}
// ----
// f() ->
// gas irOptimized: 179534
// gas legacy: 181013
// gas legacyOptimized: 180397
