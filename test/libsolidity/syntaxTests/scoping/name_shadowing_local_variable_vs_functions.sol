function e() {}
contract test {
    function f() pure public { uint e; uint g; uint h; e = g = h = 0; }
    function g() pure public {}
}
function h() {}
// ----
// Warning 2519: (63-69='uint e'): This declaration shadows an existing declaration.
// Warning 2519: (71-77='uint g'): This declaration shadows an existing declaration.
// Warning 2519: (79-85='uint h'): This declaration shadows an existing declaration.
