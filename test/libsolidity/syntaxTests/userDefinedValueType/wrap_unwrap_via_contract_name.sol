contract C { type T is uint; }
library L { type T is uint; }
interface I { type T is uint; }
contract D
{
    C.T x = C.T.wrap(uint(1));
    L.T y = L.T.wrap(uint(1));
    I.T z = I.T.wrap(uint(1));
}
