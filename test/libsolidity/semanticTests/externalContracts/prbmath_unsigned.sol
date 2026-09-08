==== ExternalSource: _prbmath/PRBMathCommon.sol ====
==== ExternalSource: _prbmath/PRBMathUD60x18.sol ====
==== Source: prbmath.sol ====
import "_prbmath/PRBMathUD60x18.sol";

contract test {
    using PRBMathUD60x18 for uint256;

    function div(uint256 x, uint256 y) external pure returns (uint256 ret) {
        ret = x.div(y);
    }
    function exp(uint256 x) external pure returns (uint256 ret) {
        ret = x.exp();
    }
    function exp2(uint256 x) external pure returns (uint256 ret) {
        ret = x.exp2();
    }
    function gm(uint256 x, uint256 y) external pure returns (uint256 ret) {
        ret = x.gm(y);
    }
    function log10(uint256 x) external pure returns (uint256 ret) {
        ret = x.log10();
    }
    function log2(uint256 x) external pure returns (uint256 ret) {
        ret = x.log2();
    }
    function mul(uint256 x, uint256 y) external pure returns (uint256 ret) {
        ret = x.mul(y);
    }
    function pow(uint256 x, uint256 y) external pure returns (uint256 ret) {
        ret = x.pow(y);
    }
    function sqrt(uint256 x) external pure returns (uint256 ret) {
        ret = x.sqrt();
    }
    function benchmark(uint256 x) external pure returns (uint256 ret, uint256 z1, uint256 z2) {
        uint256 y = x.mul(3).ceil();
        uint256 z = y.div(x);
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
// gas irOptimized: 170574
// gas irOptimized code: 1577200
// gas legacy: 195206
// gas legacy code: 1999000
// gas legacyOptimized: 168857
// gas legacyOptimized code: 1556200
// gas ssaCFGOptimized: 167659
// gas ssaCFGOptimized code: 1545400
// div(uint256,uint256): 3141592653589793238, 88714123 -> 35412542528203691288251815328
// gas irOptimized: 21909
// gas legacy: 22475
// gas legacyOptimized: 21998
// exp(uint256): 3141592653589793238 -> 23140692632779268978
// gas irOptimized: 24331
// gas legacy: 25026
// gas legacyOptimized: 24253
// exp2(uint256): 3141592653589793238 -> 8824977827076287620
// gas irOptimized: 24112
// gas legacy: 24738
// gas legacyOptimized: 24058
// gm(uint256,uint256): 3141592653589793238, 88714123 -> 16694419339601
// gas irOptimized: 22783
// gas legacy: 23244
// gas legacyOptimized: 22727
// log10(uint256): 3141592653589793238 -> 0x44fe4fc084a52b8a
// gas irOptimized: 30076
// gas legacy: 32808
// gas legacyOptimized: 29912
// log2(uint256): 3141592653589793238 -> 1651496129472318782
// gas irOptimized: 28104
// gas legacy: 30900
// gas legacyOptimized: 27994
// mul(uint256,uint256): 3141592653589793238, 88714123 -> 278703637
// gas irOptimized: 21985
// gas legacy: 22581
// gas legacyOptimized: 22089
// pow(uint256,uint256): 3141592653589793238, 5 -> 306019684785281453040
// gas irOptimized: 22313
// gas legacy: 23196
// gas legacyOptimized: 22652
// sqrt(uint256): 3141592653589793238 -> 1772453850905516027
// gas irOptimized: 22478
// gas legacy: 22803
// gas legacyOptimized: 22439
// benchmark(uint256): 3141592653589793238 -> 998882724338592125, 1000000000000000000, 1000000000000000000
// gas irOptimized: 33958
// gas legacy: 34006
// gas legacyOptimized: 32724
