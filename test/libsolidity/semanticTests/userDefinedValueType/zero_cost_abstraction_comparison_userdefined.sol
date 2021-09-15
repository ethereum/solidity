// a test to compare the cost between using user defined value types and elementary type. See the
// test zero_cost_abstraction_elementary.sol for comparison.

pragma abicoder v2;

type MyInt is int;
contract C {
    int x;
    function setX(MyInt _x) external {
        x = MyInt.unwrap(_x);
    }
    function getX() view external returns (MyInt) {
        return MyInt.wrap(x);
    }
    function add(MyInt a, MyInt b) pure external returns(MyInt) {
        return MyInt.wrap(MyInt.unwrap(a) + MyInt.unwrap(b));
    }
}

// ====
// compileViaYul: also
// ----
// getX() -> 0
// gas irOptimized: 23379
// gas legacy: 23608
// gas legacyOptimized: 23311
// setX(int256): 5 ->
// gas irOptimized: 43510
// gas legacy: 43724
// gas legacyOptimized: 43516
// getX() -> 5
// gas irOptimized: 23379
// gas legacy: 23608
// gas legacyOptimized: 23311
// add(int256,int256): 200, 99 -> 299
// gas irOptimized: 21764
// gas legacy: 22523
// gas legacyOptimized: 21813
