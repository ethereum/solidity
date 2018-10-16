{
	function verylongfunctionname(verylongvariablename) -> verylongvariablename2 {
		verylongvariablename2 := add(verylongvariablename, verylongvariablename)
	}
	// same long name
	let verylongvariablename2 := 3
	mstore(0, verylongfunctionname(verylongvariablename2))
	mstore(1, verylongvariablename2)
}
// ----
// fullInliner
// {
//     {
//         let verylongvariablename2_1 := 3
//         let verylongfu_verylongvariablename := verylongvariablename2_1
//         let verylongfu_verylongvariablename2
//         verylongfu_verylongvariablename2 := add(verylongfu_verylongvariablename, verylongfu_verylongvariablename)
//         mstore(0, verylongfu_verylongvariablename2)
//         mstore(1, verylongvariablename2_1)
//     }
//     function verylongfunctionname(verylongvariablename) -> verylongvariablename2
//     {
//         verylongvariablename2 := add(verylongvariablename, verylongvariablename)
//     }
// }
