type MyInt is uint;
type MyAddress is address;
function f() pure {
    MyInt a;
    MyInt b = a;
    MyAddress c;
    MyAddress d = c;
    b;
    d;
}

function g(MyInt a) pure returns (MyInt) {
    return a;
}
