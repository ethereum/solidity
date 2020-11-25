contract C {
    string public a;
    string public b;
    bytes public c;
    constructor() {
        a = "hello world";
        b = hex"41424344";
        c = hex"ff077fff";
    }
}
// ----
// a() -> 0x20, 11, "hello world"
// b() -> 0x20, 4, "ABCD"
// c() -> 0x20, 4, -439061522557375173052089223601630338202760422010735733633791622124826263552
