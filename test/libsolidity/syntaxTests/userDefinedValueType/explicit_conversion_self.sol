type MyInt is int;
function f(MyInt a) pure returns (MyInt b) {
    b = MyInt(a);
}
