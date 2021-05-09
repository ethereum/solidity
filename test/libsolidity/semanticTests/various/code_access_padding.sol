contract D {
    function f() public pure returns (uint256) {
        return 7;
    }
}


contract C {
    function diff() public pure returns (uint256 remainder) {
        bytes memory a = type(D).creationCode;
        bytes memory b = type(D).runtimeCode;
        assembly {
            remainder := mod(sub(b, a), 0x20)
        }
    }
}
// ====
// compileViaYul: also
// ----
// diff() -> 0 # This checks that the allocation function pads to multiples of 32 bytes #
