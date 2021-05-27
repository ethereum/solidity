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
// ====
// compileViaYul: also
// ----
// constructor()
// gas irOptimized: 2184349
// gas legacy: 2491609
// gas legacyOptimized: 1879872
// div(int256,int256): 3141592653589793238, 88714123 -> 35412542528203691288251815328
// gas irOptimized: 22324
// gas legacy: 22762
// gas legacyOptimized: 22277
// exp(int256): 3141592653589793238 -> 23140692632779268978
// gas irOptimized: 25091
// gas legacy: 25198
// gas legacyOptimized: 24352
// exp2(int256): 3141592653589793238 -> 8824977827076287620
// gas irOptimized: 24805
// gas legacy: 24859
// gas legacyOptimized: 24105
// gm(int256,int256): 3141592653589793238, 88714123 -> 16694419339601
// gas irOptimized: 23570
// gas legacy: 23223
// gas legacyOptimized: 22678
// log10(int256): 3141592653589793238 -> 4971498726941338506
// gas irOptimized: 31325
// gas legacy: 32929
// gas legacyOptimized: 30318
// log2(int256): 3141592653589793238 -> 1651496129472318782
// gas irOptimized: 29481
// gas legacy: 31062
// gas legacyOptimized: 28421
// mul(int256,int256): 3141592653589793238, 88714123 -> 278703637
// gas irOptimized: 22411
// gas legacy: 22802
// gas legacyOptimized: 22290
// pow(int256,uint256): 3141592653589793238, 5 -> 306019684785281453040
// gas irOptimized: 22971
// gas legacy: 23503
// gas legacyOptimized: 22916
// sqrt(int256): 3141592653589793238 -> 1772453850905516027
// gas irOptimized: 23243
// gas legacy: 22797
// gas legacyOptimized: 22417
// benchmark(int256): 3141592653589793238 -> 998882724338592125, 1000000000000000000, 1000000000000000000
// gas irOptimized: 43320
// gas legacy: 36668
// gas legacyOptimized: 34724
