type Int32 is int32;
using {add as +, unsub as -} for Int32 global;

function add(Int32 x, Int32 y) pure returns (Int32) {
    return loadAdder().mul(x, y);
}

function unsub(Int32 x) pure returns (Int32) {
    return loadAdder().inc(x);
}

interface IAdder {
    function mul(Int32, Int32) external pure returns (Int32);
    function inc(Int32) external pure returns (Int32);
}

contract Adder is IAdder {
    function mul(Int32 x, Int32 y) external pure override returns (Int32) {
        return Int32.wrap(Int32.unwrap(x) * Int32.unwrap(y));
    }

    function inc(Int32 x) external pure override returns (Int32) {
        return Int32.wrap(Int32.unwrap(x) + 1);
    }
}

function storeAdder(IAdder adder) pure {
    assembly {
        // This test would also work without assembly if we could hard-code an address here.
        mstore(0, adder)
    }
}

function loadAdder() pure returns (IAdder adder) {
    assembly {
        adder := mload(0)
    }
}

contract C {
    function testMul(Int32 x, Int32 y) public returns (Int32) {
        storeAdder(new Adder());

        return x + y;
    }

    function testInc(Int32 x) public returns (Int32) {
        storeAdder(new Adder());

        return -x;
    }
}
// ----
// testMul(int32,int32): 42, 10 -> 420
// gas irOptimized: 102563
// gas legacy: 183981
// gas legacyOptimized: 123563
// testInc(int32): 42 -> 43
// gas irOptimized: 102386
// gas legacy: 183239
// gas legacyOptimized: 123251
