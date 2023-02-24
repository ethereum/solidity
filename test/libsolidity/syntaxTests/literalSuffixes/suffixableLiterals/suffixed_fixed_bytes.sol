function suffix4(bytes4) pure suffix returns (bytes4) {}
function suffix19(bytes19) pure suffix returns (bytes19) {}
function suffix20(bytes20) pure suffix returns (bytes20) {}
function suffix21(bytes21) pure suffix returns (bytes21) {}

contract C {
    function f() public pure {
        0 suffix4;
        0x0 suffix4;
        0x00 suffix4;
        hex"00" suffix4;
        0.0 suffix4;

        0x11223344 suffix4;
        hex"11223344" suffix4;

        0xfFfFfFfF suffix4;
        hex"fFfFfFfF" suffix4;

        0x1234_abcd suffix4;
        hex"1234_abcd" suffix4;

        "a" suffix4;
        "abcd" suffix4;
        unicode"a" suffix4;
        unicode"abcd" suffix4;

        0x12345678901234567890123456789012345678 suffix19;
        //0x1234567890123456789012345678901234567890 suffix20; // Wrong. This is an address literal.
        0x123456789012345678901234567890123456789012 suffix21;

        hex"12345678901234567890123456789012345678" suffix19;
        hex"1234567890123456789012345678901234567890" suffix20;
        hex"123456789012345678901234567890123456789012" suffix21;
    }
}
