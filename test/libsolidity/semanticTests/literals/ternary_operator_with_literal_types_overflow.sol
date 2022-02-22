contract TestTernary
{
    function h() pure public returns (uint16 b)
    {
        b = (true ? 63 : 255) + (false ? 63 : 255);
    }

    function g() pure public returns (uint16 a)
    {
        bool t = true;
        bool f = false;
        a = (t ? 63 : 255) + (f ? 63 : 255);
    }
}
// ====
// compileViaYul: also
// ----
// g() -> FAILURE, hex"4e487b71", 0x11
// h() -> FAILURE, hex"4e487b71", 0x11
