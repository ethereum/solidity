library L {
    // Used to cause internal error
    function g(function(uint) internal returns (uint)[] storage x) public { }
}
// ----
// TypeError 4103: (63-113): Internal type is not allowed for public or external functions.
