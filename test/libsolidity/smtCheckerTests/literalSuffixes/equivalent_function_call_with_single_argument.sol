function addressSuffix(address x) pure suffix returns (bytes20) { return ~bytes20(x); }

function boolSuffix(bool x) suffix pure returns (bool) { return !x; }

function bytesSuffix(bytes memory value) pure suffix returns (uint) { return value.length; }

function intSuffix(int x) pure suffix returns (int) { return x + 1; }

function stringSuffix(string memory) suffix pure returns (uint) { return 1; }

contract C {
    function testAddressSuffix() public pure {
        assert(
            0x0000000000000000000000000000000000000001 addressSuffix ==
            addressSuffix(0x0000000000000000000000000000000000000001)
        );
    }

    function testBoolSuffix() public pure {
        assert(true boolSuffix == boolSuffix(true));
        assert(false boolSuffix == boolSuffix(false));
    }

    function testBytes() public pure {
        assert("zyx" bytesSuffix == bytesSuffix("zyx"));
        assert(hex"0123456789abcdef" bytesSuffix == bytesSuffix(hex"0123456789abcdef"));
        assert(unicode"😃" bytesSuffix == bytesSuffix(unicode"😃"));
    }

    function testStringSuffix() public pure {
        assert("a" stringSuffix == stringSuffix("a"));
        assert("abcdef" stringSuffix == stringSuffix("abcdef"));
        assert("" stringSuffix == stringSuffix(""));
    }

    function testIntSuffix() public pure {
        assert(1 intSuffix == intSuffix(1));
        assert(1 intSuffix + 1 intSuffix == intSuffix(1) + intSuffix(1));
    }
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 14 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
