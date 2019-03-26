library L {
    // Used to cause internal error
    function f(function(uint) internal returns (uint)[] memory x) public { }
}
// ----
// TypeError: (63-112): Internal type is not allowed for public or external functions.
