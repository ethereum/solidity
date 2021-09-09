type MyUint is uint;
type MyAddress is address;

function f() pure {
    MyUint.wrap(5);
    MyAddress.wrap(address(5));
}
// ----
