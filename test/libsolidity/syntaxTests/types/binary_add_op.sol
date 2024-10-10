contract C {
    function f(int32 x) external pure returns (int32)
    {
        x = 1 + 1;
        (x /= 1) + 1;
        (x = ++x) + 1;
        (0) + 1;
        return x;
    }
}
// ----
// Warning 6133: (145-152): Statement has no effect.
