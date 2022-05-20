contract C {
    function f(uint size) public pure {
        assembly {
            mstore(0, 0x4e487b7100000000000000000000000000000000000000000000000000000000)
            mstore(4, 0x43)
            revert(0, size)
        }
    }
    function a() public returns (uint) {
        try this.f(3) {
            assert(false);
        } catch Panic(uint) {
            assert(false);
        }
        // Error will be re-thrown, since there is no low-level catch clause
        assert(false);
    }
    function b() public returns (uint) {
        try this.f(6) {
            assert(false);
        } catch Panic(uint) {
            assert(false);
        }
        // Error will be re-thrown, since there is no low-level catch clause
        assert(false);
    }
    function c() public returns (uint) {
        try this.f(0x24) {
            assert(false);
        } catch Panic(uint c) {
            assert(true);
            return c;
        }
        assert(false);
    }
    function d() public returns (uint) {
        try this.f(0x100) {
            assert(false);
        } catch Panic(uint c) {
            assert(true);
            return c;
        }
        assert(false);
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// a() -> FAILURE, hex"4e487b"
// b() -> FAILURE, hex"4e487b710000"
// c() -> 0x43
// d() -> 0x43
