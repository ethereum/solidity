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
// ====
// compileViaYul: also
// ----
// f() ->
// gas irOptimized: 180067
// gas legacy: 180615
// gas legacyOptimized: 180398
