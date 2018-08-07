// This used to crash in certain compiler versions.
contract CrashContract {
       struct S { uint a; }
       S x;
       function f() public {
               (x, x) = 1(x, x);
       }
}
// ----
// TypeError: (170-177): Type is not callable
// TypeError: (170-177): Type tuple() is not implicitly convertible to expected type tuple(struct CrashContract.S storage ref,struct CrashContract.S storage ref).
