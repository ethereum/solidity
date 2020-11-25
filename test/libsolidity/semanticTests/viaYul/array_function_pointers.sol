contract C {
	function f(uint n, uint m) public {
		function() internal returns (uint)[] memory arr = new function() internal returns (uint)[](n);
		arr[m]();
	}
	function f2(uint n, uint m, uint a, uint b) public {
		function() internal returns (uint)[][] memory arr = new function() internal returns (uint)[][](n);
		for (uint i = 0; i < n; ++i)
			arr[i] = new function() internal returns (uint)[](m);
		arr[a][b]();
	}
	function g(uint n, uint m) public {
		function() external returns (uint)[] memory arr = new function() external returns (uint)[](n);
		arr[m]();
	}
	function g2(uint n, uint m, uint a, uint b) public {
		function() external returns (uint)[][] memory arr = new function() external returns (uint)[][](n);
		for (uint i = 0; i < n; ++i)
			arr[i] = new function() external returns (uint)[](m);
		arr[a][b]();
	}
}
// ====
// compileViaYul: also
// ----
// f(uint256,uint256): 1823621, 12323 -> FAILURE
// f2(uint256,uint256,uint256,uint256): 18723921, 1823621, 123, 12323 -> FAILURE
// g(uint256,uint256): 1823621, 12323 -> FAILURE
// g2(uint256,uint256,uint256,uint256): 18723921, 1823621, 123, 12323 -> FAILURE
