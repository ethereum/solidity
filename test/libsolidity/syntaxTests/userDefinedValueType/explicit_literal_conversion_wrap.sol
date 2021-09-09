type MyAddress is address;
type MyUInt8 is uint8;

function f() pure {
    MyAddress.wrap(address(5));
    MyUInt8.wrap(5);
    MyUInt8.wrap(50);
}
// ----
