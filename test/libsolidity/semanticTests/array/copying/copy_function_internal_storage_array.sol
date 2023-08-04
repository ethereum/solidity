contract C {
    function() internal returns (uint)[] x;
    function() internal returns (uint)[] y;

    function test() public returns (uint256) {
        x = new function() internal returns (uint)[](10);
        x[9] = a;
        y = x;
        return y[9]();
    }

    function a() public returns (uint256) {
        return 7;
    }
}
// ----
// test() -> 7
// gas irOptimized: 122455
// gas legacy: 205176
// gas legacyOptimized: 204971
