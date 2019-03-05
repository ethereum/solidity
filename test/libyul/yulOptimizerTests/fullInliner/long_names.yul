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
//         let verylongvariablename_4 := verylongvariablename2_1
//         let verylongvariablename2_5 := 0
//         verylongvariablename2_5 := add(verylongvariablename_4, verylongvariablename_4)
//         mstore(0, verylongvariablename2_5)
//         mstore(1, verylongvariablename2_1)
//     }
//     function verylongfunctionname(verylongvariablename) -> verylongvariablename2
//     {
//         verylongvariablename2 := add(verylongvariablename, verylongvariablename)
//     }
// }
