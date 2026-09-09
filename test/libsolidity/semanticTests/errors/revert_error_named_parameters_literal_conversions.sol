error IntWidening(uint256 a, uint256 b);
error TwoStrings(string a, string b);
error StringAndUint(string a, uint256 b);
error BytesAndBool(bytes a, bool b);
error MixedDynamic(string a, uint256 b, bytes c, bool d);
error AddressConversion(address a, uint256 b);
error SmallInts(uint8 a, uint16 b, uint256 c);

contract C {
    function intWidening() external pure {
        revert IntWidening({b: 200, a: 1});
    }

    function twoStringsUnordered() external pure {
        revert TwoStrings({b: "longer string value", a: "short"});
    }
    function twoStringsOrdered() external pure {
        revert TwoStrings({a: "short", b: "longer string value"});
    }

    function stringAndUint() external pure {
        revert StringAndUint({b: 42, a: "hello"});
    }

    function bytesAndBool() external pure {
        revert BytesAndBool({b: true, a: hex"deadbeef"});
    }

    function mixedDynamic() external pure {
        revert MixedDynamic({d: true, c: hex"cafe", b: 7, a: "test"});
    }

    function addressConversion() external pure {
        revert AddressConversion({b: 99, a: address(0x1234)});
    }

    function smallInts() external pure {
        revert SmallInts({c: 300, b: 200, a: 1});
    }
}

// ----
// intWidening() -> FAILURE, hex"57685228", 1, 200
// twoStringsUnordered() -> FAILURE, hex"16b9a491", 0x40, 0x80, 5, "short", 19, "longer string value"
// twoStringsOrdered() -> FAILURE, hex"16b9a491", 0x40, 0x80, 5, "short", 19, "longer string value"
// stringAndUint() -> FAILURE, hex"81a3bbac", 0x40, 42, 5, "hello"
// bytesAndBool() -> FAILURE, hex"ea504c15", 0x40, true, 4, left(0xdeadbeef)
// mixedDynamic() -> FAILURE, hex"e45ff7f0", 0x80, 7, 0xc0, true, 4, "test", 2, left(0xcafe)
// addressConversion() -> FAILURE, hex"6366476b", 0x1234, 99
// smallInts() -> FAILURE, hex"641a758d", 1, 200, 300
