contract C {
    string x;
    uint[4] y;
    event E(string indexed r, uint[4] indexed t);
    function deposit() public {
        for (uint i = 0; i < 90; i++)
            bytes(x).push(0);
        for (uint8 i = 0; i < 90; i++)
            bytes(x)[i] = bytes1(i);
        y[0] = 4;
        y[1] = 5;
        y[2] = 6;
        y[3] = 7;
        emit E(x, y);
    }
}
// ====
// compileViaYul: also
// ----
// deposit() ->
// ~ emit E(string,uint256[4]): #0xa7fb06bb999a5eb9aff9e0779953f4e1e4ce58044936c2f51c7fb879b85c08bd, #0xe755d8cc1a8cde16a2a31160dcd8017ac32d7e2f13215b29a23cdae40a78aa81
// gas irOptimized: 343396
// gas legacy: 390742
// gas legacyOptimized: 372772
