object "main" {
	code {
	    let a := calldataload(0)
	    let b := calldataload(32)
	    if calldataload(64) {
	        revert(0,0)
	    }
	    sstore(b, a)
	}
}
// ====
// stackOptimization: true
// ----
//     /* "":51:52   */
//   0x00
//     /* "":38:53   */
//   calldataload
//     /* "":81:83   */
//   0x20
//     /* "":68:84   */
//   calldataload
//     /* "":106:108   */
//   0x40
//     /* "":93:109   */
//   calldataload
//     /* "":90:139   */
//   tag_1
//   jumpi
//     /* "":22:160   */
// tag_2:
//     /* "":145:157   */
//   sstore
//     /* "":22:160   */
//   stop
//     /* "":110:139   */
// tag_1:
//     /* "":130:131   */
//   0x00
//     /* "":121:132   */
//   dup1
//   revert
