function suffix(int32 x) pure suffix returns (int32) {
    return loadNegator().negate(x);
}

interface INegator {
    function negate(int32) external pure returns (int32);
}

contract Negator is INegator {
    function negate(int32 x) external pure override returns (int32) {
        return -x;
    }
}

function storeNegator(INegator negator) pure {
    assembly {
        // this test would also work without assembly if we could hard-code an address here.
        mstore(0, negator)
    }
}

function loadNegator() pure returns (INegator negator) {
    assembly {
        negator := mload(0)
    }
}

contract C {
    function testSuffix() public returns (int32) {
        storeNegator(new Negator());

        return 10 suffix;
    }
}
// ----
// testSuffix() -> -10
// gas legacy: 131793
