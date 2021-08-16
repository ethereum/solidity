type MyAddress is address;
function f() {
    MyAddress a = MyAddress(5, 2);
}
// ----
// TypeError 2558: (60-75): Exactly one argument expected for explicit type conversion.
