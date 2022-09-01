==== Source: A.sol ====
import "B.sol" as B;

uint constant X = 1;
uint constant Y = B.Y;

==== Source: B.sol ====
import "A.sol" as A;

uint constant X = A.X;
uint constant Y = 2;
