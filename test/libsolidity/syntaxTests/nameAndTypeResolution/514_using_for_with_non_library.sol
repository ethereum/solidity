// This tests a crash that was resolved by making the first error fatal.
library L {
    struct S { uint d; }
    using S for S;
    function f(S memory _s) internal {
        _s.d = 1;
    }
}
// ----
// TypeError: (120-121): Library name expected.
