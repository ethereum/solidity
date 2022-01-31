contract C {
    event ev0(bytes1 indexed);
    constructor() {
        emit ev0(bytes1(bytes16(0x31313131313131313131313131313131)));
    }
    function j() external {
        bytes1 x;
        assembly { x := 0x3131313131313131313131313131313131313131313131313131313131313131 }
        emit ev0(x);
    }
}
// ====
// compileViaYul: also
// ----
// constructor() ->
// ~ emit ev0(bytes1): #"1"
// gas legacy: 168735
// j() ->
// ~ emit ev0(bytes1): #"1"
