function zero() pure returns (uint) { return 0; }

contract C {
    function f(uint z) pure external returns (uint) {
        return z.zero();
    }
    using {zero} for uint;
}
// ----
// TypeError 9582: (133-139): Member "zero" not found or not visible after argument-dependent lookup in uint256.
// TypeError 4731: (160-164): The function "zero" does not have any parameters, and therefore cannot be attached to the type "uint256".
