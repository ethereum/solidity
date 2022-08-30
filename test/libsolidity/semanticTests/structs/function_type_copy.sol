pragma abicoder v2;
struct S {
    function () external[] functions;
}

contract C {
    function f(function () external[] calldata functions) external returns (S memory) {
        S memory s;
        s.functions = functions;
        return s;
    }
}

contract Test {
    C immutable c = new C();

    function test() external returns (bool) {
        function() external[] memory functions = new function() external[](3);

        functions[0] = this.random1;
        functions[1] = this.random2;
        functions[2] = this.random3;

        S memory ret = c.f(functions);

        assert(ret.functions.length == 3);
        assert(ret.functions[0] == this.random1);
        assert(ret.functions[1] == this.random2);
        assert(ret.functions[2] == this.random3);

        return true;
    }
    function random1() external {
    }
    function random2() external {
    }
    function random3() external {
    }
}
// ====
// EVMVersion: >homestead
// ----
// test() -> true
