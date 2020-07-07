// This used to crash in certain compiler versions.
contract CrashContract {
       struct S { uint a; }
       S x;
       function f() public {
               (x, x) = 1(x, x);
       }
}
// ----
// TypeError 5704: (170-177): Type is not callable
