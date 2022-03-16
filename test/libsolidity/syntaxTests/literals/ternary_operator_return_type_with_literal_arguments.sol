contract TestTernary
{
    function g() pure public
    {
        bool t = true;
        bool f = false;
        uint8 v255 = 255;
        uint8 v63 = 63;
        uint8 a;

        // Currently none of these should produce errors or warnings.
        // The result of the operator is always a limited-precision integer, even if all arguments are literals.

        a = (t ? 63 : 255) + (f ? 63 : 255);
        a = (t ? 0x3f : 0xff) + (f ? 0x3f : 0xff);
        a = (t ? uint8(63) : 255) + (f ? 63 : uint8(255));
        a = (t ? v63 : 255) + (f ? 63 : v255);

        a = (true ? 63 : 255) + (false ? 63 : 255);
        a = (true ? 0x3f : 0xff) + (false ? 0x3f : 0xff);
        a = (true ? uint8(63) : 255) + (false ? 63 : uint8(255));
        a = (true ? v63 : 255) + (false ? 63 : v255);

        a = (t ? 63 : 255) - (f ? 63 : 255);
        a = (t ? 63 : 255) * (f ? 63 : 255);
        a = (t ? 63 : 255) / (f ? 63 : 255);

        a = (t ? (true ? 63 : 255) : (false ? 63 : 255)) + (f ? (t ? 63 : 255) : (f ? 63 : 255));
        a = uint8(t ? 63 : 255) + uint8(f ? 63 : 255);

    }
}
// ----
