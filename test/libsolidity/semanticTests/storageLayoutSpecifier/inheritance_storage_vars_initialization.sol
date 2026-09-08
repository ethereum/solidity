contract A { uint public x = 1; }
contract B is A { uint public y = x * 10; }
contract C1 is B layout at 2**256 - 2**40 { uint public z = y + 5; }
contract C2 is B { uint public z = y + 5; }
contract F {
    function withSpecifier() public returns (uint, uint, uint) {
        C1 c = new C1();
        return (c.x(), c.y(), c.z());
    }
    function withoutSpecifier() public returns (uint, uint, uint) {
        C2 c = new C2();
        return (c.x(), c.y(), c.z());
    }
}
// ----
// withSpecifier() -> 1, 10, 15
// gas irOptimized: 121822
// gas irOptimized code: 30800
// gas legacy: 123715
// gas legacy code: 63400
// gas legacyOptimized: 121966
// gas legacyOptimized code: 23800
// gas ssaCFGOptimized: 121770
// gas ssaCFGOptimized code: 29600
// withoutSpecifier() -> 1, 10, 15
// gas irOptimized: 121768
// gas irOptimized code: 27600
// gas legacy: 123599
// gas legacy code: 40400
// gas legacyOptimized: 121916
// gas legacyOptimized code: 20600
// gas ssaCFGOptimized: 121724
// gas ssaCFGOptimized code: 26400
