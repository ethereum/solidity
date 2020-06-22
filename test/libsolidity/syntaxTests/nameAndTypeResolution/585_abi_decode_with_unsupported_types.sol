contract C {
	struct s { uint a; uint b; }
    function f() pure public {
        abi.decode("", (s));
    }
}
// ----
// TypeError 9611: (98-99): Decoding type struct C.s memory not supported.
