contract Test {
    struct A {
        mapping(uint=>uint) m;
    }
    struct B {
        mapping(uint=>uint) m;
        uint x;
    }
    struct C {
        mapping(uint=>uint)[] ma;
    }
    struct D {
        A[] a;
    }
    A storageA;
    B storageB;
    C storageC;
    D storageD;
    constructor() public {
        storageA.m[1] = 2;
        storageB.m[3] = 4;
        storageB.x = 5;
        storageC.ma.length = 6;
        storageD.a.length = 7;
    }
    function run() public returns (uint, uint, uint, uint, uint, uint) {
        A memory memoryA = A();
        B memory memoryB = B(42);
        C memory memoryC = C();
        D memory memoryD1 = D(new A[](999));
        D memory memoryD2 = storageD;
        storageA = memoryA;
        storageB = memoryB;
        storageC = memoryC;
        // the following line does not compile because unimplemented
        // storageD = memoryD1;
        return (
            storageA.m[1],
            storageB.x,
            memoryB.x,
            storageC.ma.length,
            memoryD1.a.length,
            memoryD2.a.length
        );
    }
}
// ----
// run() -> 2, 42, 42, 6, 999, 7
