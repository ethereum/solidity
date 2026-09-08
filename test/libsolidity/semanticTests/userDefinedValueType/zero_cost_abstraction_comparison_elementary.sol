// a test to compare the cost between using user defined value types and elementary type. See the
// test zero_cost_abstraction_userdefined.sol for a comparison.

pragma abicoder v2;

contract C {
    int x;
    function setX(int _x) external {
        x = _x;
    }
    function getX() view external returns (int) {
        return x;
    }
    function add(int a, int b) view external returns (int) {
        return a + b;
    }
}
// ----
// getX() -> 0
// gas irOptimized: 23308
// gas legacy: 23473
// gas legacyOptimized: 23309
// setX(int256): 5 ->
// gas irOptimized: 43461
// gas legacy: 43718
// gas legacyOptimized: 43513
// getX() -> 5
// gas irOptimized: 23308
// gas legacy: 23473
// gas legacyOptimized: 23309
// add(int256,int256): 200, 99 -> 299
// gas irOptimized: 21620
// gas legacy: 22354
// gas legacyOptimized: 21768
