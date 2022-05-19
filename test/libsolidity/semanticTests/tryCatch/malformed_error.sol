contract C {
    function f(uint size) public pure {
        assembly {
            mstore(0, 0x08c379a000000000000000000000000000000000000000000000000000000000)
            mstore(4, 0x20)
            mstore(0x24, 7)
            mstore(0x44, "abcdefg")
            revert(0, size)
        }
    }
    function a() public returns (uint) {
        try this.f(3) {
            assert(false);
        } catch Panic(uint) {
            assert(false);
        } catch Error(string memory) {
            assert(false);
        } catch {
            assert(true);
        }
    }
    function b() public returns (uint) {
        try this.f(6) {
            assert(false);
        } catch Panic(uint) {
            assert(false);
        } catch Error(string memory) {
            assert(false);
        } catch {
            assert(true);
        }
    }
    function b2() public returns (uint) {
        try this.f(0x43) {
            assert(false);
        } catch Panic(uint) {
            assert(false);
        } catch Error(string memory) {
            assert(false);
        } catch {
            assert(true);
        }
    }
    function b3() public returns (string memory) {
        try this.f(0x4a) {
            assert(false);
        } catch Panic(uint) {
            assert(false);
        } catch Error(string memory) {
            assert(false);
        } catch {
            assert(true);
        }
    }
    function c() public returns (string memory) {
        try this.f(0x4b) {
            assert(false);
        } catch Panic(uint) {
            assert(false);
        } catch Error(string memory er) {
            assert(true);
            return er;
        } catch {
            assert(false);
        }
    }
    function d() public returns (string memory) {
        try this.f(0x100) {
            assert(false);
        } catch Panic(uint) {
            assert(false);
        } catch Error(string memory er) {
            assert(true);
            return er;
        } catch {
            assert(false);
        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// a() -> 0x00
// b() -> 0x00
// b2() -> 0x00
// b3() -> 0x20, 0x00
// c() -> 0x20, 7, "abcdefg"
// d() -> 0x20, 7, "abcdefg"
