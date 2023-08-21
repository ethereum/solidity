interface IThing {
    /// @param v value to search for
    /// @return x a number
    /// @return y another number
    function value(uint256 v) external view returns (uint128 x, uint128 y);
}

contract Thing is IThing {
    struct Value {
        uint128 x;
        uint128 y;
    }

    mapping(uint256=>Value) public override value;
}
// ----
