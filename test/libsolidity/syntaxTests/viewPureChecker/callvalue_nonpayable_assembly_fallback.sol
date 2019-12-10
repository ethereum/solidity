contract C
{
    fallback() external {
        uint x;
        assembly {
            x := callvalue()
        }
    }
}
// ----
