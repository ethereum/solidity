library L {
    event E();
}
library S {
    event E(uint);
}
library T {
    event E();
}

contract D {
    event F();
}

contract C is D {
    function test1() external pure returns (bytes32, bytes32) {
        assert(L.E.selector == T.E.selector);

        assert(L.E.selector != S.E.selector);
        assert(T.E.selector != S.E.selector);

        return (L.E.selector, S.E.selector);
    }

    bytes32 s1 = L.E.selector;
    bytes32 s2 = S.E.selector;
    bytes32 s3 = T.E.selector;
    function test2() external returns (bytes32, bytes32, bytes32) {
        return (s1, s2, s3);
    }

    function test3() external returns (bytes32) {
        return (F.selector);
    }
}
// ====
// compileViaYul: also
// ----
// test1() -> 0x92bbf6e823a631f3c8e09b1c8df90f378fb56f7fbc9701827e1ff8aad7f6a028, 0x2ff0672f372fbe844b353429d4510ea5e43683af134c54f75f789ff57bc0c0
// test2() -> 0x92bbf6e823a631f3c8e09b1c8df90f378fb56f7fbc9701827e1ff8aad7f6a028, 0x2ff0672f372fbe844b353429d4510ea5e43683af134c54f75f789ff57bc0c0, 0x92bbf6e823a631f3c8e09b1c8df90f378fb56f7fbc9701827e1ff8aad7f6a028
// test3() -> 0x28811f5935c16a099486acb976b3a6b4942950a1425a74e9eb3e9b7f7135e12a
