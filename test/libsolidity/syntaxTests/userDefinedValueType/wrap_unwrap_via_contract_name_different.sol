contract C { type T is uint; }
library L { type T is uint; }
contract D
{
    C.T x = L.T.wrap(uint(1));
}
// ----
// TypeError 7407: (86-103): Type L.T is not implicitly convertible to expected type C.T.
