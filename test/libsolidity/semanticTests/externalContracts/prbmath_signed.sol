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
// gas irOptimized: 1878547
// gas legacy: 2478955
// gas legacyOptimized: 1877737
// div(int256,int256): 3141592653589793238, 88714123 -> 35412542528203691288251815328
// gas irOptimized: 22137
// gas legacy: 22767
// gas legacyOptimized: 22282
// exp(int256): 3141592653589793238 -> 23140692632779268978
// gas irOptimized: 24545
// gas legacy: 25203
// gas legacyOptimized: 24357
// exp2(int256): 3141592653589793238 -> 8824977827076287620
// gas irOptimized: 24257
// gas legacy: 24864
// gas legacyOptimized: 24110
// gm(int256,int256): 3141592653589793238, 88714123 -> 16694419339601
// gas irOptimized: 22970
// gas legacy: 23228
// gas legacyOptimized: 22683
// log10(int256): 3141592653589793238 -> 4971498726941338506
// gas irOptimized: 30609
// gas legacy: 32934
// gas legacyOptimized: 30323
// log2(int256): 3141592653589793238 -> 1651496129472318782
// gas irOptimized: 28819
// gas legacy: 31067
// gas legacyOptimized: 28426
// mul(int256,int256): 3141592653589793238, 88714123 -> 278703637
// gas irOptimized: 22225
// gas legacy: 22807
// gas legacyOptimized: 22295
// pow(int256,uint256): 3141592653589793238, 5 -> 306019684785281453040
// gas irOptimized: 22635
// gas legacy: 23508
// gas legacyOptimized: 22921
// sqrt(int256): 3141592653589793238 -> 1772453850905516027
// gas irOptimized: 22650
// gas legacy: 22802
// gas legacyOptimized: 22422
// benchmark(int256): 3141592653589793238 -> 998882724338592125, 1000000000000000000, 1000000000000000000
// gas irOptimized: 36630
// gas legacy: 36673
// gas legacyOptimized: 34729
