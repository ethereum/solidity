type MyAddress is address;
type MyUInt is uint;
function f() {
    MyAddress a;
    MyUInt b;
}
contract C {
    type MyAddress is address;
    type MyUInt is uint;
    mapping(MyAddress => MyUInt) public m;
}
// ----
