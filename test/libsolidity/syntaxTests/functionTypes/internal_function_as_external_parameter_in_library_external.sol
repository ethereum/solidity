library L {
    function f(function(uint) internal returns (uint) x) public {
    }
}
// ----
// TypeError: (27-67): Internal or recursive type is not allowed for public or external functions.
