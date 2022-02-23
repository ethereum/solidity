pragma abicoder v1;
contract C {
	struct s { uint a; uint b; }
    function f() pure public {
        abi.decode("", (s));
    }
}
// ----
// TypeError 9611: (118-119): Decoding type struct C.s memory not supported.
