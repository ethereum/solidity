function suffix(int32 x) pure suffix returns (int32) {
    return loadNegator().negate(x);
}

interface INegatorPure {
    function negate(int32) external pure returns (int32);
}

interface INegatorView {
    function negate(int32) external view returns (int32);
}

contract Negator is INegatorView {
    function negate(int32 x) external view override returns (int32) {
        return -x;
    }
}

function storeNegator(INegatorView negator) pure {
    assembly {
        // this test would also work without assembly if we could hard-code an address here.
        mstore(0, negator)
    }
}

function loadNegator() pure returns (INegatorPure negator) {
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
