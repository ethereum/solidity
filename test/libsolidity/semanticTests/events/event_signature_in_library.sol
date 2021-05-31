pragma abicoder v2;
library L {
    struct S {
        uint8 a;
        int16 b;
    }
    event E(S indexed, S);
    function f() internal {
        S memory s;
        emit E(s, s);
    }
}
contract C {
    constructor() {
        L.f();
    }
}
// ====
// compileViaYul: also
// ----
// constructor()
// ~ emit E((uint8,int16),(uint8,int16)): #0xad3228b676f7d3cd4284a5443f17f1962b36e491b30a40b2405849e597ba5fb5, 0x00, 0x00
// gas legacy: 150662
