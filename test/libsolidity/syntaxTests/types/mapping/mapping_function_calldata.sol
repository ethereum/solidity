contract test {
	struct S {
		mapping (uint => uint) s;
	}
    function f(S calldata b) external {
    }
}
// ----
// TypeError: (74-86): Types containing (nested) mappings can only be used in storage.
// TypeError: (74-86): Only libraries are allowed to use a (nested) mapping type in public or external functions.
