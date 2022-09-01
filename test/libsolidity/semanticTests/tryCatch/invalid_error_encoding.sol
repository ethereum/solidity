contract C {
    function g(bytes memory revertMsg) public pure returns (uint, uint) {
        assembly { revert(add(revertMsg, 0x20), mload(revertMsg)) }
    }
    function f1() public returns (uint x) {
        // Invalid signature
        try this.g(abi.encodeWithSelector(0x12345678, uint(0), uint(0), uint(0))) returns (uint a, uint b) {
            return 0;
        } catch Error(string memory) {
            return 1;
        } catch (bytes memory) {
            return 2;
        }
    }
    function f1a() public returns (uint x) {
        // Invalid signature
        try this.g(abi.encodeWithSelector(0x12345678, uint(0), uint(0), uint(0))) returns (uint a, uint b) {
            return 0;
        } catch Error(string memory) {
            return 1;
        } catch {
            return 2;
        }
    }
    function f1b() public returns (uint x) {
        // Invalid signature
        try this.g(abi.encodeWithSelector(0x12345678, uint(0), uint(0), uint(0))) returns (uint a, uint b) {
            return 0;
        } catch Error(string memory) {
            return 1;
        }
    }
    function f1c() public returns (uint x) {
        // Invalid signature
        try this.g(abi.encodeWithSelector(0x12345678, uint(0), uint(0), uint(0))) returns (uint a, uint b) {
            return 0;
        } catch {
            return 2;
        }
    }
    function f2() public returns (uint x) {
        // Valid signature but illegal offset
        try this.g(abi.encodeWithSignature("Error(string)", uint(0x100), uint(0), uint(0))) returns (uint a, uint b) {
            return 0;
        } catch Error(string memory) {
            return 1;
        } catch (bytes memory) {
            return 2;
        }
    }
    function f2a() public returns (uint x) {
        // Valid signature but illegal offset
        try this.g(abi.encodeWithSignature("Error(string)", uint(0x100), uint(0), uint(0))) returns (uint a, uint b) {
            return 0;
        } catch Error(string memory) {
            return 1;
        } catch {
            return 2;
        }
    }
    function f2b() public returns (uint x) {
        // Valid signature but illegal offset
        try this.g(abi.encodeWithSignature("Error(string)", uint(0x100), uint(0), uint(0))) returns (uint a, uint b) {
            return 0;
        } catch Error(string memory) {
            return 1;
        }
    }
    function f2c() public returns (uint x) {
        // Valid signature but illegal offset
        try this.g(abi.encodeWithSignature("Error(string)", uint(0x100), uint(0), uint(0))) returns (uint a, uint b) {
            return 0;
        } catch {
            return 1;
        }
    }
    function f3() public returns (uint x) {
        // Valid up to length
        try this.g(abi.encodeWithSignature("Error(string)", uint(0x20), uint(0x30), uint(0))) returns (uint a, uint b) {
            return 0;
        } catch Error(string memory) {
            return 1;
        } catch (bytes memory) {
            return 2;
        }
    }
    function f3a() public returns (uint x) {
        // Valid up to length
        try this.g(abi.encodeWithSignature("Error(string)", uint(0x20), uint(0x30), uint(0))) returns (uint a, uint b) {
            return 0;
        } catch Error(string memory) {
            return 1;
        } catch (bytes memory) {
            return 2;
        }
    }
    function f3b() public returns (uint x) {
        // Valid up to length
        try this.g(abi.encodeWithSignature("Error(string)", uint(0x20), uint(0x30), uint(0))) returns (uint a, uint b) {
            return 0;
        } catch Error(string memory) {
            return 1;
        }
    }
    function f3c() public returns (uint x) {
        // Valid up to length
        try this.g(abi.encodeWithSignature("Error(string)", uint(0x20), uint(0x30), uint(0))) returns (uint a, uint b) {
            return 0;
        } catch {
            return 1;
        }
    }
    function f4() public returns (uint x) {
        // Fully valid
        try this.g(abi.encodeWithSignature("Error(string)", uint(0x20), uint(0x7), bytes7("abcdefg"))) returns (uint a, uint b) {
            return 0;
        } catch Error(string memory) {
            return 1;
        } catch (bytes memory) {
            return 2;
        }
    }
    function f4a() public returns (uint x) {
        // Fully valid
        try this.g(abi.encodeWithSignature("Error(string)", uint(0x20), uint(0x7), bytes7("abcdefg"))) returns (uint a, uint b) {
            return 0;
        } catch Error(string memory) {
            return 1;
        } catch {
            return 2;
        }
    }
    function f4b() public returns (uint x) {
        // Fully valid
        try this.g(abi.encodeWithSignature("Error(string)", uint(0x20), uint(0x7), bytes7("abcdefg"))) returns (uint a, uint b) {
            return 0;
        } catch Error(string memory) {
            return 1;
        }
    }
    function f4c() public returns (uint x) {
        // Fully valid
        try this.g(abi.encodeWithSignature("Error(string)", uint(0x20), uint(0x7), bytes7("abcdefg"))) returns (uint a, uint b) {
            return 0;
        } catch {
            return 1;
        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// f1() -> 2
// f1a() -> 2
// f1b() -> FAILURE, hex"12345678", 0x0, 0, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
// f1c() -> 2
// f2() -> 2
// f2a() -> 2
// f2b() -> FAILURE, hex"08c379a0", 0x100, 0, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
// f2c() -> 1
// f3() -> 2
// f3a() -> 2
// f3b() -> FAILURE, hex"08c379a0", 0x20, 48, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
// f3c() -> 1
// f4() -> 1
// f4a() -> 1
// f4b() -> 1
// f4c() -> 1
