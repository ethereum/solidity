object "main" {
	code {
		codecopy(0, dataoffset("deployed"), datasize("deployed"))
		return(0, datasize("deployed"))
	}
	object "deployed" {
		code {
			for {}
			add(delegatecall(delegatecall(call(selfbalance(), 0x0, 0x0, 0x0, 0x0, 0x0, 0x0), 0x0, 0x0, 0x0, 0x0, 0x0), 0x0, 0x0, 0x0, 0x0, 0x0),0x0)
			{}
			{}
		}
	}
}
// ====
// stackOptimization: true
// ----
//     /* "":62:82   */
//   dataSize(sub_0)
//     /* "":38:60   */
//   dataOffset(sub_0)
//     /* "":35:36   */
//   0x00
//     /* "":26:83   */
//   codecopy
//     /* "":96:116   */
//   dataSize(sub_0)
//     /* "":93:94   */
//   0x00
//     /* "":86:117   */
//   return
// stop
//
// sub_0: assembly {
//     tag_1:
//         /* "":296:299   */
//       0x00
//       dup1
//       dup1
//       dup1
//       dup1
//       dup1
//       dup1
//       dup1
//       dup1
//       dup1
//       dup1
//       dup1
//       dup1
//       dup1
//       dup1
//       dup1
//       dup1
//         /* "":199:212   */
//       selfbalance
//         /* "":194:243   */
//       call
//         /* "":181:269   */
//       delegatecall
//         /* "":168:295   */
//       delegatecall
//         /* "":164:300   */
//       add
//       tag_2
//       jumpi
//     tag_3:
//       stop
//     tag_2:
//       jump(tag_1)
// }
