contract C {
    bytes32 constant x = x;
    function f() public pure returns (uint t) {
        assembly {
            // Reference to a circular member
            t := x
        }
    }
}
// ----
// TypeError 3558: (171-172): Constant variable is circular.
