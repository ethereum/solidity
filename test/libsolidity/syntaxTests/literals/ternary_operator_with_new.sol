contract C {
    function f(bool cond, bytes calldata cbytes) public view {
        bytes1[2] memory marray;
        bytes1[2] storage sarray;
	uint8[2] memory uint8Array;
        int16[2] memory int16Array;
        bytes memory mbytes;

        // OK
        bytes1[2] memory marray1 = cond ? sarray : marray;
        bytes1[2] memory marray2 = cond ? marray : sarray;
        bytes memory mbytes1 = cond ? cbytes[1:2] : mbytes;
        bytes memory mbytes2 = cond ? mbytes : cbytes[1:2];

        // Errors
        bytes1[2] storage sarray1 = cond ? sarray : marray;
        bytes1[2] storage sarray2 = cond ? marray : sarray;

        cond ? uint8Array : int16Array;
        cond ? [1, 2] : [-1, -3];
    }
}
// ----
// TypeError 9574: (517-567): Type bytes1[2] memory is not implicitly convertible to expected type bytes1[2] storage pointer.
// TypeError 9574: (577-627): Type bytes1[2] memory is not implicitly convertible to expected type bytes1[2] storage pointer.
// TypeError 1080: (638-668): True expression's type uint8[2] memory does not match false expression's type int16[2] memory.
// TypeError 1080: (678-702): True expression's type uint8[2] memory does not match false expression's type int8[2] memory.
