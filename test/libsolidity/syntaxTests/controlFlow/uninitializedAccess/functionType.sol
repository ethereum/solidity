contract C {
    // Make sure function parameters and return values are not considered
    // for uninitialized return detection in the control flow analysis.
    function f(function(uint[] storage) internal returns (uint[] storage)) internal pure
    returns (function(uint[] storage) internal returns (uint[] storage))
    {
    }
}
