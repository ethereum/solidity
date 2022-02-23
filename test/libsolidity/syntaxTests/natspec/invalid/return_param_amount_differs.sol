interface IThing {
    /// @return x a number
    /// @return y another number
    function value() external view returns (uint128 x, uint128 y);
}

contract Thing is IThing {
    struct Value {
        uint128 x;
        uint128 y;
    }

    Value public override value;
}
