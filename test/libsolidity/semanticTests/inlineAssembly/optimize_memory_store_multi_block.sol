contract C {
	function f() external returns (uint256 x) {
		assembly {
			mstore(0, 0x42)
		}
		assembly {
			x := mload(0)
		}
	}
	function g() external returns (bool) {
		uint initialFreeMemoryPointer;
		assembly {
			initialFreeMemoryPointer := mload(0x40)
		}
		assembly {
			let ptr := mload(0x40)
			mstore(0x40, add(ptr, 0x20))
		}
		uint finalFreeMemoryPointer;
		assembly {
			finalFreeMemoryPointer := mload(0x40)
		}
		assert(initialFreeMemoryPointer != finalFreeMemoryPointer);
		return true;
	}
}
// ----
// f() -> 0x42
// g() -> true
