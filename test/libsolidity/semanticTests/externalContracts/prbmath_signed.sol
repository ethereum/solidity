==== ExternalSource: _prbmath/PRBMathCommon.sol ====
==== ExternalSource: _prbmath/PRBMathSD59x18.sol ====
==== Source: prbmath.sol ====
import "_prbmath/PRBMathSD59x18.sol";

contract test {
    using PRBMathSD59x18 for int256;

    function div(int256 x, int256 y) external pure returns (int256 ret) {
        ret = x.div(y);
    }
    function exp(int256 x) external pure returns (int256 ret) {
        ret = x.exp();
    }
    function exp2(int256 x) external pure returns (int256 ret) {
        ret = x.exp2();
    }
    function gm(int256 x, int256 y) external pure returns (int256 ret) {
        ret = x.gm(y);
    }
    function log10(int256 x) external pure returns (int256 ret) {
        ret = x.log10();
    }
    function log2(int256 x) external pure returns (int256 ret) {
        ret = x.log2();
    }
    function mul(int256 x, int256 y) external pure returns (int256 ret) {
        ret = x.mul(y);
    }
    function pow(int256 x, uint256 y) external pure returns (int256 ret) {
        ret = x.pow(y);
    }
    function sqrt(int256 x) external pure returns (int256 ret) {
        ret = x.sqrt();
    }
    function benchmark(int256 x) external pure returns (int256 ret, int256 z1, int256 z2) {
        int256 y = x.mul(3).ceil();
        int256 z = y.div(x);
        for (uint i = 0; i < 10; i++)
            z = z.sqrt();
        ret = z;

        // Check precision
        z1 = z.ceil();
        z2 = z.sqrt().pow(2).ceil();
        assert(z1 == z2);
    }
}
// ----
// constructor()
// gas irOptimized: 177875
// gas irOptimized code: 1674200
// gas legacy: 209723
// gas legacy code: 2205000
// gas legacyOptimized: 178012
// gas legacyOptimized code: 1669600
// gas ssaCFGOptimized: 175515
// gas ssaCFGOptimized code: 1645200
// div(int256,int256): 3141592653589793238, 88714123 -> 35412542528203691288251815328
// gas irOptimized: 22042
// gas legacy: 22736
// gas legacyOptimized: 22264
// exp(int256): 3141592653589793238 -> 23140692632779268978
// gas irOptimized: 24449
// gas legacy: 25124
// gas legacyOptimized: 24351
// exp2(int256): 3141592653589793238 -> 8824977827076287620
// gas irOptimized: 24159
// gas legacy: 24787
// gas legacyOptimized: 24105
// gm(int256,int256): 3141592653589793238, 88714123 -> 16694419339601
// gas irOptimized: 22802
// gas legacy: 23202
// gas legacyOptimized: 22685
// log10(int256): 3141592653589793238 -> 4971498726941338506
// gas irOptimized: 30297
// gas legacy: 32841
// gas legacyOptimized: 30249
// log2(int256): 3141592653589793238 -> 1651496129472318782
// gas irOptimized: 28563
// gas legacy: 30979
// gas legacyOptimized: 28357
// mul(int256,int256): 3141592653589793238, 88714123 -> 278703637
// gas irOptimized: 22144
// gas legacy: 22775
// gas legacyOptimized: 22288
// pow(int256,uint256): 3141592653589793238, 5 -> 306019684785281453040
// gas irOptimized: 22485
// gas legacy: 23453
// gas legacyOptimized: 22921
// sqrt(int256): 3141592653589793238 -> 1772453850905516027
// gas irOptimized: 22455
// gas legacy: 22784
// gas legacyOptimized: 22420
// benchmark(int256): 3141592653589793238 -> 998882724338592125, 1000000000000000000, 1000000000000000000
// gas irOptimized: 34890
// gas legacy: 35244
// gas legacyOptimized: 33996
