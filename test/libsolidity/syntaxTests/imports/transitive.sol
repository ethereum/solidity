==== Source: a ====
struct S { uint[2] mS; }
==== Source: b ====
import "a" as A;
struct T { A.S[2] mT; }
==== Source: c ====
pragma experimental ABIEncoderV2;
import "b" as B;
contract C {
    function f(B.T memory y, B.A.S memory z) public pure returns (uint, uint) {
        z = B.A.S([uint(2), 3]);
        y = B.T([z, z]);
        return (y.mT[0].mS[0], z.mS[0]);
    }
}
