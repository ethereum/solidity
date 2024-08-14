pragma abicoder v2;

struct S {
    uint256 a;
    bool b;
    string s;
}

error E();
error E1(uint256);
error E2(string);
error E3(S);
error E4(address);
error E5(function() external pure);

contract C {
    function a() external pure {
        require(false, E());
    }
    function b() external pure {
        require(false, E1(1));
    }
    function c() external pure {
        require(false, E2("string literal"));
    }
    function d() external pure {
        require(false, E3(S(1, true, "string literal")));
    }
    function e() external view {
        require(false, E4(address(this)));
    }
    function f() external view {
        require(false, E5(this.a));
    }
}

// ====
// compileToEOF: true
// EVMVersion: >=prague
// ----
// a() -> FAILURE, hex"92bbf6e8"
// b() -> FAILURE, hex"47e26897", hex"0000000000000000000000000000000000000000000000000000000000000001"
// c() -> FAILURE, hex"8f372c34", hex"0000000000000000000000000000000000000000000000000000000000000020", hex"000000000000000000000000000000000000000000000000000000000000000e", hex"737472696e67206c69746572616c000000000000000000000000000000000000"
// d() -> FAILURE, hex"5717173e", hex"0000000000000000000000000000000000000000000000000000000000000020", hex"0000000000000000000000000000000000000000000000000000000000000001", hex"0000000000000000000000000000000000000000000000000000000000000001", hex"0000000000000000000000000000000000000000000000000000000000000060", hex"000000000000000000000000000000000000000000000000000000000000000e", hex"737472696e67206c69746572616c000000000000000000000000000000000000"
// e() -> FAILURE, hex"7efef9ea", hex"0000000000000000000000008e3f661b8facaa0fa7aa0113847501029db6517e"
// f() -> FAILURE, hex"0c3f12eb", hex"8e3f661b8facaa0fa7aa0113847501029db6517e0dbe671f0000000000000000"
