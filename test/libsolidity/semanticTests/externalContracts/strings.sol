==== ExternalSource: _stringutils/stringutils.sol ====
==== Source: strings.sol ====
pragma abicoder v2;
import "_stringutils/stringutils.sol";

contract test {
    using strings for bytes32;
    using strings for string;
    using strings for strings.slice;

    function toSlice(string memory a) external pure returns (strings.slice memory) {
        return a.toSlice();
    }

    function roundtrip(string memory a) external pure returns (string memory) {
        return a.toSlice().toString();
    }

    function utf8len(string memory a) external pure returns (uint) {
        return a.toSlice().len();
    }

    function multiconcat(string memory a, uint count) public pure returns (string memory) {
        strings.slice memory s = a.toSlice();
        for (uint i = 0; i < count; i++) {
            s = s.concat(s).toSlice();
        }
        return s.toString();
    }

    function benchmark(string memory text, bytes32 seed) external pure returns (uint) {
        // Grow text.
        text = multiconcat(text, 10);

        strings.slice memory a = text.toSlice();
        strings.slice memory b = seed.toSliceB32();

        // Some heavy computation.
        bool c = b.equals(a) || b.startsWith(a);

        // Join as a list.
        strings.slice memory delim = c ? string(",").toSlice() : string(";").toSlice();
        strings.slice[] memory parts = new strings.slice[](2);
        parts[0] = a;
        parts[1] = b;
        string memory d = delim.join(parts);
        return d.toSlice().len();
    }
}
// ----
// constructor()
// gas irOptimized: 667123
// gas legacy: 1092225
// gas legacyOptimized: 735488
// toSlice(string): 0x20, 11, "hello world" -> 11, 0xa0
// gas irOptimized: 22660
// gas legacy: 23190
// gas legacyOptimized: 22508
// roundtrip(string): 0x20, 11, "hello world" -> 0x20, 11, "hello world"
// gas irOptimized: 23408
// gas legacy: 23820
// gas legacyOptimized: 23123
// utf8len(string): 0x20, 16, "\xf0\x9f\x98\x83\xf0\x9f\x98\x83\xf0\x9f\x98\x83\xf0\x9f\x98\x83" -> 4 # Input: "😃😃😃😃" #
// gas irOptimized: 24026
// gas legacy: 25716
// gas legacyOptimized: 24115
// multiconcat(string,uint256): 0x40, 3, 11, "hello world" -> 0x20, 0x58, 0x68656c6c6f20776f726c6468656c6c6f20776f726c6468656c6c6f20776f726c, 0x6468656c6c6f20776f726c6468656c6c6f20776f726c6468656c6c6f20776f72, 49027192869463622675296414541903001712009715982962058146354235762728281047040 # concatenating 3 times #
// gas irOptimized: 28440
// gas legacy: 31621
// gas legacyOptimized: 27914
// benchmark(string,bytes32): 0x40, 0x0842021, 8, "solidity" -> 0x2020
// gas irOptimized: 2016994
// gas legacy: 4292872
// gas legacyOptimized: 2327029
