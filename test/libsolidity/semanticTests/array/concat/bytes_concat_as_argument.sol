contract C {
    function f(bytes memory a, bytes memory b) public returns (bytes32) {
        return keccak256(bytes.concat(a, b));
    }

    function h(bytes memory a) internal returns (uint256) {
        return a.length;
    }

    function g(bytes memory a, bytes memory b) public returns (uint256) {
        return h(bytes.concat(a, b));
    }
}
// ====
// compileViaYul: also
// ----
// f(bytes,bytes): 0x40, 0x80, 32, "abcdabcdabcdabcdabcdabcdabcdabcd", 5, "bcdef" -> 0x1106e19b6f06d1cce71c2d816754f83dfa5b95df958c5cbf12b7c472320c427c
// g(bytes,bytes): 0x40, 0x80, 32, "abcdabcdabcdabcdabcdabcdabcdabcd", 5, "bcdef" -> 37
