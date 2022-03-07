pragma abicoder v2;

struct S { uint value; }

contract Test {
    S[][] a;
    S[] b;

    constructor() {
        a.push();
        a[0].push(S(1));
        a[0].push(S(2));
        a[0].push(S(3));

        b.push(S(4));
        b.push(S(5));
        b.push(S(6));
        b.push(S(7));
    }

    function test1() external returns (bool) {
        a.push();
        a[1] = b;

        assert(a.length == 2);
        assert(a[0].length == 3);
        assert(a[1].length == 4);
        assert(a[1][0].value == 4);
        assert(a[1][1].value == 5);
        assert(a[1][2].value == 6);
        assert(a[1][3].value == 7);

        return true;
    }

    function test2() external returns (bool) {
        S[][] memory temp = new S[][](2);

        temp = a;

        assert(temp.length == 2);
        assert(temp[0].length == 3);
        assert(temp[1].length == 4);
        assert(temp[1][0].value == 4);
        assert(temp[1][1].value == 5);
        assert(temp[1][2].value == 6);
        assert(temp[1][3].value == 7);

        return true;
    }

    function test3() external returns (bool) {
        S[][] memory temp = new S[][](2);

        temp[0] = a[0];
        temp[1] = a[1];

        assert(temp.length == 2);
        assert(temp[0].length == 3);
        assert(temp[1].length == 4);
        assert(temp[1][0].value == 4);
        assert(temp[1][1].value == 5);
        assert(temp[1][2].value == 6);
        assert(temp[1][3].value == 7);

        return true;
    }

    function test4() external returns (bool) {
        S[][] memory temp = new S[][](2);

        temp[0] = a[0];
        temp[1] = b;

        assert(temp.length == 2);
        assert(temp[0].length == 3);
        assert(temp[1].length == 4);
        assert(temp[1][0].value == 4);
        assert(temp[1][1].value == 5);
        assert(temp[1][2].value == 6);
        assert(temp[1][3].value == 7);

        return true;
    }
}
// ====
// EVMVersion: >homestead
// compileViaYul: also
// ----
// test1() -> true
// gas irOptimized: 150533
// gas legacy: 150266
// gas legacyOptimized: 149875
// test2() -> true
// test3() -> true
// test4() -> true
