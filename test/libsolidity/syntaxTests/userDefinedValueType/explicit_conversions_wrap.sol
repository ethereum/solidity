type MyUint is uint;
type MyAddress is address;

function f() pure {
    MyUint.wrap(5);
    MyAddress.wrap(address(5));
}
// ----
// Warning 6133: (73-87): Statement has no effect.
// Warning 6133: (93-119): Statement has no effect.
