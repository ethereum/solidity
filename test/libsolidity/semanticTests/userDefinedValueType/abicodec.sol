// A test to see if `abi.encodeWithSelector(..., (CustomType))` works as intended.
contract C {
    type MyInt is int;
    function f(MyInt x) external returns(MyInt a, MyInt b, MyInt c, MyInt d) {
        a = MyInt.wrap(-1);
        b = MyInt.wrap(0);
        c = MyInt.wrap(1);
        d = x;
    }
    function g() external returns(bool) {
        (bool success1, bytes memory ret1) =  address(this).call(abi.encodeWithSelector(this.f.selector, MyInt.wrap(5)));
        assert(success1);

        (MyInt a1, MyInt b1, MyInt c1, MyInt d1) = abi.decode(ret1, (MyInt, MyInt, MyInt, MyInt));
        assert(MyInt.unwrap(a1) == -1);
        assert(MyInt.unwrap(b1) == 0);
        assert(MyInt.unwrap(c1) == 1);
        assert(MyInt.unwrap(d1) == 5);

        (bool success2, bytes memory ret2) = address(this).call(abi.encodeWithSelector(this.f.selector, int(-5)));
        assert(success2);

        (int a2, int b2, int c2, int d2) = abi.decode(ret2, (int, int, int, int));
        assert(a2 == -1);
        assert(b2 == 0);
        assert(c2 == 1);
        assert(d2 == -5);

        return true;
    }
}
// ====
// compileViaYul: also
// EVMVersion: >=byzantium
// ----
// g() -> true
