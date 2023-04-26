library Lib {
    struct Data { uint a; uint[] b; }
    function set(Data storage _s) public
    {
        _s.a = 7;
        while (_s.b.length < 20)
            _s.b.push();
        _s.b[19] = 8;
    }
}
contract Test {
    mapping(string => Lib.Data) data;
    function f() public returns (uint a, uint b)
    {
        Lib.set(data["abc"]);
        a = data["abc"].a;
        b = data["abc"].b[19];
    }
}
// ====
// compileToEwasm: false
// ----
// library: Lib
// f() -> 7, 8
// gas irOptimized: 101818
// gas legacy: 101429
