contract C {
    function set(uint value) private {
        assembly {
            tstore(0, value)
        }
    }

    function get() private view returns (uint value) {
        assembly {
            value := tload(0)
        }
    }

    function f() external {
        assembly {
            tstore(0, 42)
        }
    }

    function g() external view returns(uint r) {
        assembly {
            r := tload(0)
        }
    }

    function h() external returns (uint x) {
        set(99);
        x = get();
    }
}
// ====
// EVMVersion: >=cancun
// ----
// g() -> 0
// f() ->
// g() -> 0
// h() -> 0x63
// g() -> 0
