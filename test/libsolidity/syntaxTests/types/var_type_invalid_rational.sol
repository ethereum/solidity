contract C {
    function f() internal pure {
        var i = 31415999999999999999999999999999999999999999999999999999999999999999933**3;
        var unreachable = 123;
    }
}
// ----
// TypeError: (62-136): Invalid rational int_const 3100...(204 digits omitted)...9237 (absolute value too large or division by zero).
