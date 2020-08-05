pragma experimental ABIEncoderV2;

contract C {
    struct S { uint a; S[] sub; }
    function f() public pure returns (uint, S memory) {
    }
}
// ----
// TypeError 4103: (126-134): Recursive type not allowed for public or external contract functions.
