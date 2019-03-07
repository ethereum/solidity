// It should not be possible to return internal functions from external functions.
contract C {
    function f() public returns (function(uint) internal returns (uint) x) {
    }
}
// ----
// TypeError: (129-169): Internal type is not allowed for public or external functions.
