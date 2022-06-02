function suffix(address a) pure returns (address) { return a; }
function payableSuffix(address payable a) pure returns (address payable) { return a; }

contract C {
    function f() public pure {
        0x0000000000000000000000000000000000000000 suffix;
        0xFFfFfFffFFfffFFfFFfFFFFFffFFFffffFfFFFfF suffix;
        0x1234567890123456789012345678901234567890 suffix;

        0x1234567890_1234567890_1234567890_1234567890 suffix;
    }
}
