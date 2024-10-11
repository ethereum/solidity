object "t" {
	code {
		sstore(0, datasize("name_that_is_longer_than_32_bytes_name_that_is_longer_than_32_bytes_name_that_is_longer_than_32_bytes"))
	}
	object "name_that_is_longer_than_32_bytes_name_that_is_longer_than_32_bytes_name_that_is_longer_than_32_bytes" {
		code {}
	}
}
// ====
// EVMVersion: >=shanghai
// optimizationPreset: full
// ----
// Assembly:
//     /* "source":56:169   */
//   dataSize(sub_0)
//     /* "source":53:54   */
//   0x00
//     /* "source":46:170   */
//   sstore
//     /* "source":22:186   */
//   stop
// stop
//
// sub_0: assembly {
//         /* "source":317:324   */
//       stop
// }
// Bytecode: 60015f5500fe
// Opcodes: PUSH1 0x1 PUSH0 SSTORE STOP INVALID
// SourceMappings: 56:113:0:-:0;53:1;46:124;22:164
