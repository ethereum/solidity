function e() {}
contract test {
    function f() pure public { uint e; e = 0; }
}
// ----
// Warning 2519: (63-69): This declaration shadows an existing declaration.
