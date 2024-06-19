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
// step: fullInliner
//
// {
//     {
//         let verylongvariablename2_1 := 3
//         let verylongvariablename_1 := verylongvariablename2_1
//         let verylongvariablename2_2 := 0
//         verylongvariablename2_2 := add(verylongvariablename_1, verylongvariablename_1)
//         mstore(0, verylongvariablename2_2)
//         mstore(1, verylongvariablename2_1)
//     }
//     function verylongfunctionname(verylongvariablename) -> verylongvariablename2
//     {
//         verylongvariablename2 := add(verylongvariablename, verylongvariablename)
//     }
// }
