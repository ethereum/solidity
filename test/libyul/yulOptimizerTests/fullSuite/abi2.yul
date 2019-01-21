{
	// This ignores many of the encoding / decoding functions. Over time,
	// we should add them all here.

    let a, b := abi_decode_tuple_t_contract$_Module_$1038t_contract$_Module_$1038(mload(0), mload(1))
    sstore(0, a)
    let x0, x1, x2, x3, x4 := abi_decode_tuple_t_addresst_uint256t_bytes_calldata_ptrt_enum$_Operation_$1949(mload(7), mload(8))
    sstore(x1, x0)
    sstore(x3, x2)
    sstore(1, x4)
    let r := abi_encode_tuple_t_bytes32_t_address_t_uint256_t_bytes32_t_enum$_Operation_$1949_t_uint256_t_uint256_t_uint256_t_address_t_address_t_uint256__to_t_bytes32_t_address_t_uint256_t_bytes32_t_uint8_t_uint256_t_uint256_t_uint256_t_address_t_address_t_uint256_(
        mload(30),
        mload(31),
        mload(32),
        mload(33),
        mload(34),
        mload(35),
        mload(36),
        mload(37),
        mload(38),
        mload(39),
        mload(40),
        mload(41)
    )

    function abi_decode_t_address(offset, end) -> value
    {
        value := cleanup_revert_t_address(calldataload(offset))
    }
    function abi_decode_t_address_payable(offset_1, end_2) -> value_3
    {
        value_3 := cleanup_revert_t_address_payable(calldataload(offset_1))
    }
    function abi_decode_t_array$_t_address_$dyn_calldata_ptr(offset_4, end_5) -> arrayPos, length
    {
        if iszero(slt(add(offset_4, 0x1f), end_5))
        {
            revert(0, 0)
        }
        length := calldataload(offset_4)
        if gt(length, 0xffffffffffffffff)
        {
            revert(0, 0)
        }
        arrayPos := add(offset_4, 0x20)
        if gt(add(arrayPos, mul(length, 0x20)), end_5)
        {
            revert(0, 0)
        }
    }
    function abi_decode_t_bool_fromMemory(offset_6, end_7) -> value_8
    {
        value_8 := cleanup_revert_t_bool(mload(offset_6))
    }
    function abi_decode_t_bytes32(offset_9, end_10) -> value_11
    {
        value_11 := cleanup_revert_t_bytes32(calldataload(offset_9))
    }
    function abi_decode_t_bytes_calldata_ptr(offset_12, end_13) -> arrayPos_14, length_15
    {
        if iszero(slt(add(offset_12, 0x1f), end_13))
        {
            revert(0, 0)
        }
        length_15 := calldataload(offset_12)
        if gt(length_15, 0xffffffffffffffff)
        {
            revert(0, 0)
        }
        arrayPos_14 := add(offset_12, 0x20)
        if gt(add(arrayPos_14, mul(length_15, 0x1)), end_13)
        {
            revert(0, 0)
        }
    }
    function abi_decode_t_bytes_memory_ptr(offset_16, end_17) -> array
    {
        if iszero(slt(add(offset_16, 0x1f), end_17))
        {
            revert(0, 0)
        }
        let length_18 := calldataload(offset_16)
        array := allocateMemory(array_allocation_size_t_bytes_memory_ptr(length_18))
        mstore(array, length_18)
        let src := add(offset_16, 0x20)
        let dst := add(array, 0x20)
        if gt(add(src, length_18), end_17)
        {
            revert(0, 0)
        }
        copy_calldata_to_memory(src, dst, length_18)
    }
    function abi_decode_t_contract$_Module_$1038(offset_19, end_20) -> value_21
    {
        value_21 := cleanup_revert_t_contract$_Module_$1038(calldataload(offset_19))
    }
    function abi_decode_t_enum$_Operation_$1949(offset_22, end_23) -> value_24
    {
        value_24 := cleanup_revert_t_enum$_Operation_$1949(calldataload(offset_22))
    }
    function abi_decode_t_uint256(offset_25, end_26) -> value_27
    {
        value_27 := cleanup_revert_t_uint256(calldataload(offset_25))
    }
    function abi_decode_tuple_t_address(headStart, dataEnd) -> value0
    {
        if slt(sub(dataEnd, headStart), 32)
        {
            revert(0, 0)
        }
        {
            let offset_28 := 0
            value0 := abi_decode_t_address(add(headStart, offset_28), dataEnd)
        }
    }
    function abi_decode_tuple_t_addresst_addresst_address(headStart_29, dataEnd_30) -> value0_31, value1, value2
    {
        if slt(sub(dataEnd_30, headStart_29), 96)
        {
            revert(0, 0)
        }
        {
            let offset_32 := 0
            value0_31 := abi_decode_t_address(add(headStart_29, offset_32), dataEnd_30)
        }
        {
            let offset_33 := 32
            value1 := abi_decode_t_address(add(headStart_29, offset_33), dataEnd_30)
        }
        {
            let offset_34 := 64
            value2 := abi_decode_t_address(add(headStart_29, offset_34), dataEnd_30)
        }
    }
    function abi_decode_tuple_t_addresst_addresst_uint256(headStart_35, dataEnd_36) -> value0_37, value1_38, value2_39
    {
        if slt(sub(dataEnd_36, headStart_35), 96)
        {
            revert(0, 0)
        }
        {
            let offset_40 := 0
            value0_37 := abi_decode_t_address(add(headStart_35, offset_40), dataEnd_36)
        }
        {
            let offset_41 := 32
            value1_38 := abi_decode_t_address(add(headStart_35, offset_41), dataEnd_36)
        }
        {
            let offset_42 := 64
            value2_39 := abi_decode_t_uint256(add(headStart_35, offset_42), dataEnd_36)
        }
    }
    function abi_decode_tuple_t_addresst_bytes32(headStart_43, dataEnd_44) -> value0_45, value1_46
    {
        if slt(sub(dataEnd_44, headStart_43), 64)
        {
            revert(0, 0)
        }
        {
            let offset_47 := 0
            value0_45 := abi_decode_t_address(add(headStart_43, offset_47), dataEnd_44)
        }
        {
            let offset_48 := 32
            value1_46 := abi_decode_t_bytes32(add(headStart_43, offset_48), dataEnd_44)
        }
    }
    function abi_decode_tuple_t_addresst_uint256(headStart_49, dataEnd_50) -> value0_51, value1_52
    {
        if slt(sub(dataEnd_50, headStart_49), 64)
        {
            revert(0, 0)
        }
        {
            let offset_53 := 0
            value0_51 := abi_decode_t_address(add(headStart_49, offset_53), dataEnd_50)
        }
        {
            let offset_54 := 32
            value1_52 := abi_decode_t_uint256(add(headStart_49, offset_54), dataEnd_50)
        }
    }
    function abi_decode_tuple_t_addresst_uint256t_bytes_calldata_ptrt_enum$_Operation_$1949(headStart_55, dataEnd_56) -> value0_57, value1_58, value2_59, value3, value4
    {
        if slt(sub(dataEnd_56, headStart_55), 128)
        {
            revert(0, 0)
        }
        {
            let offset_60 := 0
            value0_57 := abi_decode_t_address(add(headStart_55, offset_60), dataEnd_56)
        }
        {
            let offset_61 := 32
            value1_58 := abi_decode_t_uint256(add(headStart_55, offset_61), dataEnd_56)
        }
        {
            let offset_62 := calldataload(add(headStart_55, 64))
            if gt(offset_62, 0xffffffffffffffff)
            {
                revert(0, 0)
            }
            value2_59, value3 := abi_decode_t_bytes_calldata_ptr(add(headStart_55, offset_62), dataEnd_56)
        }
        {
            let offset_63 := 96
            value4 := abi_decode_t_enum$_Operation_$1949(add(headStart_55, offset_63), dataEnd_56)
        }
    }
    function abi_decode_tuple_t_addresst_uint256t_bytes_calldata_ptrt_enum$_Operation_$1949t_uint256t_uint256t_uint256t_addresst_address_payablet_bytes_calldata_ptr(headStart_64, dataEnd_65) -> value0_66, value1_67, value2_68, value3_69, value4_70, value5, value6, value7, value8, value9, value10, value11
    {
        if slt(sub(dataEnd_65, headStart_64), 320)
        {
            revert(0, 0)
        }
        {
            let offset_71 := 0
            value0_66 := abi_decode_t_address(add(headStart_64, offset_71), dataEnd_65)
        }
        {
            let offset_72 := 32
            value1_67 := abi_decode_t_uint256(add(headStart_64, offset_72), dataEnd_65)
        }
        {
            let offset_73 := calldataload(add(headStart_64, 64))
            if gt(offset_73, 0xffffffffffffffff)
            {
                revert(0, 0)
            }
            value2_68, value3_69 := abi_decode_t_bytes_calldata_ptr(add(headStart_64, offset_73), dataEnd_65)
        }
        {
            let offset_74 := 96
            value4_70 := abi_decode_t_enum$_Operation_$1949(add(headStart_64, offset_74), dataEnd_65)
        }
        {
            let offset_75 := 128
            value5 := abi_decode_t_uint256(add(headStart_64, offset_75), dataEnd_65)
        }
        {
            let offset_76 := 160
            value6 := abi_decode_t_uint256(add(headStart_64, offset_76), dataEnd_65)
        }
        {
            let offset_77 := 192
            value7 := abi_decode_t_uint256(add(headStart_64, offset_77), dataEnd_65)
        }
        {
            let offset_78 := 224
            value8 := abi_decode_t_address(add(headStart_64, offset_78), dataEnd_65)
        }
        {
            let offset_79 := 256
            value9 := abi_decode_t_address_payable(add(headStart_64, offset_79), dataEnd_65)
        }
        {
            let offset_80 := calldataload(add(headStart_64, 288))
            if gt(offset_80, 0xffffffffffffffff)
            {
                revert(0, 0)
            }
            value10, value11 := abi_decode_t_bytes_calldata_ptr(add(headStart_64, offset_80), dataEnd_65)
        }
    }
    function abi_decode_tuple_t_addresst_uint256t_bytes_memory_ptrt_enum$_Operation_$1949(headStart_81, dataEnd_82) -> value0_83, value1_84, value2_85, value3_86
    {
        if slt(sub(dataEnd_82, headStart_81), 128)
        {
            revert(0, 0)
        }
        {
            let offset_87 := 0
            value0_83 := abi_decode_t_address(add(headStart_81, offset_87), dataEnd_82)
        }
        {
            let offset_88 := 32
            value1_84 := abi_decode_t_uint256(add(headStart_81, offset_88), dataEnd_82)
        }
        {
            let offset_89 := calldataload(add(headStart_81, 64))
            if gt(offset_89, 0xffffffffffffffff)
            {
                revert(0, 0)
            }
            value2_85 := abi_decode_t_bytes_memory_ptr(add(headStart_81, offset_89), dataEnd_82)
        }
        {
            let offset_90 := 96
            value3_86 := abi_decode_t_enum$_Operation_$1949(add(headStart_81, offset_90), dataEnd_82)
        }
    }
    function abi_decode_tuple_t_addresst_uint256t_bytes_memory_ptrt_enum$_Operation_$1949t_uint256t_uint256t_uint256t_addresst_addresst_uint256(headStart_91, dataEnd_92) -> value0_93, value1_94, value2_95, value3_96, value4_97, value5_98, value6_99, value7_100, value8_101, value9_102
    {
        if slt(sub(dataEnd_92, headStart_91), 320)
        {
            revert(0, 0)
        }
        {
            let offset_103 := 0
            value0_93 := abi_decode_t_address(add(headStart_91, offset_103), dataEnd_92)
        }
        {
            let offset_104 := 32
            value1_94 := abi_decode_t_uint256(add(headStart_91, offset_104), dataEnd_92)
        }
        {
            let offset_105 := calldataload(add(headStart_91, 64))
            if gt(offset_105, 0xffffffffffffffff)
            {
                revert(0, 0)
            }
            value2_95 := abi_decode_t_bytes_memory_ptr(add(headStart_91, offset_105), dataEnd_92)
        }
        {
            let offset_106 := 96
            value3_96 := abi_decode_t_enum$_Operation_$1949(add(headStart_91, offset_106), dataEnd_92)
        }
        {
            let offset_107 := 128
            value4_97 := abi_decode_t_uint256(add(headStart_91, offset_107), dataEnd_92)
        }
        {
            let offset_108 := 160
            value5_98 := abi_decode_t_uint256(add(headStart_91, offset_108), dataEnd_92)
        }
        {
            let offset_109 := 192
            value6_99 := abi_decode_t_uint256(add(headStart_91, offset_109), dataEnd_92)
        }
        {
            let offset_110 := 224
            value7_100 := abi_decode_t_address(add(headStart_91, offset_110), dataEnd_92)
        }
        {
            let offset_111 := 256
            value8_101 := abi_decode_t_address(add(headStart_91, offset_111), dataEnd_92)
        }
        {
            let offset_112 := 288
            value9_102 := abi_decode_t_uint256(add(headStart_91, offset_112), dataEnd_92)
        }
    }
    function abi_decode_tuple_t_array$_t_address_$dyn_calldata_ptrt_uint256t_addresst_bytes_calldata_ptr(headStart_113, dataEnd_114) -> value0_115, value1_116, value2_117, value3_118, value4_119, value5_120
    {
        if slt(sub(dataEnd_114, headStart_113), 128)
        {
            revert(0, 0)
        }
        {
            let offset_121 := calldataload(add(headStart_113, 0))
            if gt(offset_121, 0xffffffffffffffff)
            {
                revert(0, 0)
            }
            value0_115, value1_116 := abi_decode_t_array$_t_address_$dyn_calldata_ptr(add(headStart_113, offset_121), dataEnd_114)
        }
        {
            let offset_122 := 32
            value2_117 := abi_decode_t_uint256(add(headStart_113, offset_122), dataEnd_114)
        }
        {
            let offset_123 := 64
            value3_118 := abi_decode_t_address(add(headStart_113, offset_123), dataEnd_114)
        }
        {
            let offset_124 := calldataload(add(headStart_113, 96))
            if gt(offset_124, 0xffffffffffffffff)
            {
                revert(0, 0)
            }
            value4_119, value5_120 := abi_decode_t_bytes_calldata_ptr(add(headStart_113, offset_124), dataEnd_114)
        }
    }
    function abi_decode_tuple_t_bool_fromMemory(headStart_125, dataEnd_126) -> value0_127
    {
        if slt(sub(dataEnd_126, headStart_125), 32)
        {
            revert(0, 0)
        }
        {
            let offset_128 := 0
            value0_127 := abi_decode_t_bool_fromMemory(add(headStart_125, offset_128), dataEnd_126)
        }
    }
    function abi_decode_tuple_t_bytes32(headStart_129, dataEnd_130) -> value0_131
    {
        if slt(sub(dataEnd_130, headStart_129), 32)
        {
            revert(0, 0)
        }
        {
            let offset_132 := 0
            value0_131 := abi_decode_t_bytes32(add(headStart_129, offset_132), dataEnd_130)
        }
    }
    function abi_decode_tuple_t_bytes_calldata_ptr(headStart_133, dataEnd_134) -> value0_135, value1_136
    {
        if slt(sub(dataEnd_134, headStart_133), 32)
        {
            revert(0, 0)
        }
        {
            let offset_137 := calldataload(add(headStart_133, 0))
            if gt(offset_137, 0xffffffffffffffff)
            {
                revert(0, 0)
            }
            value0_135, value1_136 := abi_decode_t_bytes_calldata_ptr(add(headStart_133, offset_137), dataEnd_134)
        }
    }
    function abi_decode_tuple_t_bytes_calldata_ptrt_bytes_calldata_ptr(headStart_138, dataEnd_139) -> value0_140, value1_141, value2_142, value3_143
    {
        if slt(sub(dataEnd_139, headStart_138), 64)
        {
            revert(0, 0)
        }
        {
            let offset_144 := calldataload(add(headStart_138, 0))
            if gt(offset_144, 0xffffffffffffffff)
            {
                revert(0, 0)
            }
            value0_140, value1_141 := abi_decode_t_bytes_calldata_ptr(add(headStart_138, offset_144), dataEnd_139)
        }
        {
            let offset_145 := calldataload(add(headStart_138, 32))
            if gt(offset_145, 0xffffffffffffffff)
            {
                revert(0, 0)
            }
            value2_142, value3_143 := abi_decode_t_bytes_calldata_ptr(add(headStart_138, offset_145), dataEnd_139)
        }
    }
    function abi_decode_tuple_t_bytes_memory_ptr(headStart_146, dataEnd_147) -> value0_148
    {
        if slt(sub(dataEnd_147, headStart_146), 32)
        {
            revert(0, 0)
        }
        {
            let offset_149 := calldataload(add(headStart_146, 0))
            if gt(offset_149, 0xffffffffffffffff)
            {
                revert(0, 0)
            }
            value0_148 := abi_decode_t_bytes_memory_ptr(add(headStart_146, offset_149), dataEnd_147)
        }
    }
    function abi_decode_tuple_t_contract$_Module_$1038(headStart_150, dataEnd_151) -> value0_152
    {
        if slt(sub(dataEnd_151, headStart_150), 32)
        {
            revert(0, 0)
        }
        {
            let offset_153 := 0
            value0_152 := abi_decode_t_contract$_Module_$1038(add(headStart_150, offset_153), dataEnd_151)
        }
    }
    function abi_decode_tuple_t_contract$_Module_$1038t_contract$_Module_$1038(headStart_154, dataEnd_155) -> value0_156, value1_157
    {
        if slt(sub(dataEnd_155, headStart_154), 64)
        {
            revert(0, 0)
        }
        {
            let offset_158 := 0
            value0_156 := abi_decode_t_contract$_Module_$1038(add(headStart_154, offset_158), dataEnd_155)
        }
        {
            let offset_159 := 32
            value1_157 := abi_decode_t_contract$_Module_$1038(add(headStart_154, offset_159), dataEnd_155)
        }
    }
    function abi_decode_tuple_t_uint256(headStart_160, dataEnd_161) -> value0_162
    {
        if slt(sub(dataEnd_161, headStart_160), 32)
        {
            revert(0, 0)
        }
        {
            let offset_163 := 0
            value0_162 := abi_decode_t_uint256(add(headStart_160, offset_163), dataEnd_161)
        }
    }
    function abi_encode_t_address_to_t_address(value_164, pos)
    {
        mstore(pos, cleanup_assert_t_address(value_164))
    }
    function abi_encode_t_array$_t_address_$dyn_memory_ptr_to_t_array$_t_address_$dyn_memory_ptr(value_165, pos_166) -> end_167
    {
        let length_168 := array_length_t_array$_t_address_$dyn_memory_ptr(value_165)
        mstore(pos_166, length_168)
        pos_166 := add(pos_166, 0x20)
        let srcPtr := array_dataslot_t_array$_t_address_$dyn_memory_ptr(value_165)
        for {
            let i := 0
        }
        lt(i, length_168)
        {
            i := add(i, 1)
        }
        {
            abi_encode_t_address_to_t_address(mload(srcPtr), pos_166)
            srcPtr := array_nextElement_t_array$_t_address_$dyn_memory_ptr(srcPtr)
            pos_166 := add(pos_166, 0x20)
        }
        end_167 := pos_166
    }
    function abi_encode_t_bool_to_t_bool(value_169, pos_170)
    {
        mstore(pos_170, cleanup_assert_t_bool(value_169))
    }
    function abi_encode_t_bytes32_to_t_bytes32(value_171, pos_172)
    {
        mstore(pos_172, cleanup_assert_t_bytes32(value_171))
    }
    function abi_encode_t_bytes_memory_ptr_to_t_bytes_memory_ptr(value_173, pos_174) -> end_175
    {
        let length_176 := array_length_t_bytes_memory_ptr(value_173)
        mstore(pos_174, length_176)
        copy_memory_to_memory(add(value_173, 0x20), add(pos_174, 0x20), length_176)
        end_175 := add(add(pos_174, 0x20), round_up_to_mul_of_32(length_176))
    }
    function abi_encode_t_contract$_GnosisSafe_$710_to_t_address_payable(value_177, pos_178)
    {
        mstore(pos_178, convert_t_contract$_GnosisSafe_$710_to_t_address_payable(value_177))
    }
    function abi_encode_t_contract$_Module_$1038_to_t_address(value_179, pos_180)
    {
        mstore(pos_180, convert_t_contract$_Module_$1038_to_t_address(value_179))
    }
    function abi_encode_t_enum$_Operation_$1949_to_t_uint8(value_181, pos_182)
    {
        mstore(pos_182, convert_t_enum$_Operation_$1949_to_t_uint8(value_181))
    }
    function abi_encode_t_string_memory_ptr_to_t_string_memory_ptr(value_183, pos_184) -> end_185
    {
        let length_186 := array_length_t_string_memory_ptr(value_183)
        mstore(pos_184, length_186)
        copy_memory_to_memory(add(value_183, 0x20), add(pos_184, 0x20), length_186)
        end_185 := add(add(pos_184, 0x20), round_up_to_mul_of_32(length_186))
    }
    function abi_encode_t_string_memory_to_t_string_memory_ptr(value_187, pos_188) -> end_189
    {
        let length_190 := array_length_t_string_memory(value_187)
        mstore(pos_188, length_190)
        copy_memory_to_memory(add(value_187, 0x20), add(pos_188, 0x20), length_190)
        end_189 := add(add(pos_188, 0x20), round_up_to_mul_of_32(length_190))
    }
    function abi_encode_t_stringliteral_108d84599042957b954e89d43b52f80be89321dfc114a37800028eba58dafc87_to_t_string_memory_ptr(pos_191) -> end_192
    {
        mstore(pos_191, 36)
        mstore(add(pos_191, 32), 0x496e76616c6964206d617374657220636f707920616464726573732070726f76)
        mstore(add(pos_191, 64), 0x6964656400000000000000000000000000000000000000000000000000000000)
        end_192 := add(pos_191, 96)
    }
    function abi_encode_t_stringliteral_1e0428ffa69bff65645154a36d5017c238f946ddaf89430d30eec813f30bdd77_to_t_string_memory_ptr(pos_193) -> end_194
    {
        mstore(pos_193, 37)
        mstore(add(pos_193, 32), 0x4d6f64756c6573206861766520616c7265616479206265656e20696e69746961)
        mstore(add(pos_193, 64), 0x6c697a6564000000000000000000000000000000000000000000000000000000)
        end_194 := add(pos_193, 96)
    }
    function abi_encode_t_stringliteral_21a1cd38818adb750881fbf07c26ce7223dde608fdd9dadd31a0d41afeca2094_to_t_string_memory_ptr(pos_195) -> end_196
    {
        mstore(pos_195, 30)
        mstore(add(pos_195, 32), 0x496e76616c6964206f776e657220616464726573732070726f76696465640000)
        end_196 := add(pos_195, 64)
    }
    function abi_encode_t_stringliteral_5caa315f9c5cf61be71c182eef2dc9ef7b6ce6b42c320d36694e1d23e09c287e_to_t_string_memory_ptr(pos_197) -> end_198
    {
        mstore(pos_197, 40)
        mstore(add(pos_197, 32), 0x496e76616c696420707265764d6f64756c652c206d6f64756c65207061697220)
        mstore(add(pos_197, 64), 0x70726f7669646564000000000000000000000000000000000000000000000000)
        end_198 := add(pos_197, 96)
    }
    function abi_encode_t_stringliteral_60f21058f4a7689ef29853b3c9c17c9bf69856a949794649bb68878f00552475_to_t_string_memory_ptr(pos_199) -> end_200
    {
        mstore(pos_199, 30)
        mstore(add(pos_199, 32), 0x4f6e6c79206f776e6572732063616e20617070726f7665206120686173680000)
        end_200 := add(pos_199, 64)
    }
    function abi_encode_t_stringliteral_63d26a9feb8568677e5c255c04e4da88e86a25137d5152a9a089790b7e710e86_to_t_string_memory_ptr(pos_201) -> end_202
    {
        mstore(pos_201, 35)
        mstore(add(pos_201, 32), 0x5468726573686f6c642063616e6e6f7420657863656564206f776e657220636f)
        mstore(add(pos_201, 64), 0x756e740000000000000000000000000000000000000000000000000000000000)
        end_202 := add(pos_201, 96)
    }
    function abi_encode_t_stringliteral_7913a3f9168bf3e458e3f42eb08db5c4b33f44228d345660887090b75e24c6aa_to_t_string_memory_ptr(pos_203) -> end_204
    {
        mstore(pos_203, 31)
        mstore(add(pos_203, 32), 0x436f756c64206e6f742066696e69736820696e697469616c697a6174696f6e00)
        end_204 := add(pos_203, 64)
    }
    function abi_encode_t_stringliteral_839b4c4db845de24ec74ef067d85431087d6987a4c904418ee4f6ec699c02482_to_t_string_memory_ptr(pos_205) -> end_206
    {
        mstore(pos_205, 53)
        mstore(add(pos_205, 32), 0x4e6577206f776e657220636f756e74206e6565647320746f206265206c617267)
        mstore(add(pos_205, 64), 0x6572207468616e206e6577207468726573686f6c640000000000000000000000)
        end_206 := add(pos_205, 96)
    }
    function abi_encode_t_stringliteral_8560a13547eca5648355c8db1a9f8653b6f657d31d476c36bca25e47b45b08f4_to_t_string_memory_ptr(pos_207) -> end_208
    {
        mstore(pos_207, 34)
        mstore(add(pos_207, 32), 0x436f756c64206e6f74207061792067617320636f737473207769746820746f6b)
        mstore(add(pos_207, 64), 0x656e000000000000000000000000000000000000000000000000000000000000)
        end_208 := add(pos_207, 96)
    }
    function abi_encode_t_stringliteral_85bcea44c930431ef19052d068cc504a81260341ae6c5ee84bb5a38ec55acf05_to_t_string_memory_ptr(pos_209) -> end_210
    {
        mstore(pos_209, 27)
        mstore(add(pos_209, 32), 0x496e76616c6964207369676e6174757265732070726f76696465640000000000)
        end_210 := add(pos_209, 64)
    }
    function abi_encode_t_stringliteral_8c2199b479423c52a835dfe8b0f2e9eb4c1ec1069ba198ccc38077a4a88a5c00_to_t_string_memory_ptr(pos_211) -> end_212
    {
        mstore(pos_211, 31)
        mstore(add(pos_211, 32), 0x496e76616c6964206d6f64756c6520616464726573732070726f766964656400)
        end_212 := add(pos_211, 64)
    }
    function abi_encode_t_stringliteral_960698caed81fce71c9b7d572ab2e035b6014a5b812b51df8462ea9817fc4ebc_to_t_string_memory_ptr(pos_213) -> end_214
    {
        mstore(pos_213, 38)
        mstore(add(pos_213, 32), 0x496e76616c696420707265764f776e65722c206f776e65722070616972207072)
        mstore(add(pos_213, 64), 0x6f76696465640000000000000000000000000000000000000000000000000000)
        end_214 := add(pos_213, 96)
    }
    function abi_encode_t_stringliteral_9a45ae898fbe2bd07a0b33b5a6c421f76198e9deb66843b8d827b0b9e4a16f86_to_t_string_memory_ptr(pos_215) -> end_216
    {
        mstore(pos_215, 30)
        mstore(add(pos_215, 32), 0x4f776e657273206861766520616c7265616479206265656e2073657475700000)
        end_216 := add(pos_215, 64)
    }
    function abi_encode_t_stringliteral_9d461d71e19b25cd406798d062d7e61f961ad52541d3077a543e857810427d47_to_t_string_memory_ptr(pos_217) -> end_218
    {
        mstore(pos_217, 27)
        mstore(add(pos_217, 32), 0x4164647265737320697320616c726561647920616e206f776e65720000000000)
        end_218 := add(pos_217, 64)
    }
    function abi_encode_t_stringliteral_a2e1f2db9cd32eaa6a2caa3d6caa726a30dc0417d866440bfe13d6a6d030e5e2_to_t_string_memory_ptr(pos_219) -> end_220
    {
        mstore(pos_219, 29)
        mstore(add(pos_219, 32), 0x446f6d61696e20536570617261746f7220616c72656164792073657421000000)
        end_220 := add(pos_219, 64)
    }
    function abi_encode_t_stringliteral_a803fa289679098e38a7f1f6fe43056918c5ab5af07441cb8db77b949c165ca1_to_t_string_memory_ptr(pos_221) -> end_222
    {
        mstore(pos_221, 32)
        mstore(add(pos_221, 32), 0x4475706c6963617465206f776e657220616464726573732070726f7669646564)
        end_222 := add(pos_221, 64)
    }
    function abi_encode_t_stringliteral_ae2b4ea52eaf6de3fb2d8a64b7555be2dfd285b837a62821bf24e7dc6f329450_to_t_string_memory_ptr(pos_223) -> end_224
    {
        mstore(pos_223, 29)
        mstore(add(pos_223, 32), 0x4d6f64756c652068617320616c7265616479206265656e206164646564000000)
        end_224 := add(pos_223, 64)
    }
    function abi_encode_t_stringliteral_b995394ed6031392a784e6dd5e04285cca83077a8dc3873d2fb7fcb090297ab4_to_t_string_memory_ptr(pos_225) -> end_226
    {
        mstore(pos_225, 36)
        mstore(add(pos_225, 32), 0x5468726573686f6c64206e6565647320746f2062652067726561746572207468)
        mstore(add(pos_225, 64), 0x616e203000000000000000000000000000000000000000000000000000000000)
        end_226 := add(pos_225, 96)
    }
    function abi_encode_t_stringliteral_c4780ef0a1d41d59bac8c510cf9ada421bccf2b90f75a8e4ba2e8c09e8d72733_to_t_string_memory_ptr(pos_227) -> end_228
    {
        mstore(pos_227, 44)
        mstore(add(pos_227, 32), 0x4d6574686f642063616e206f6e6c792062652063616c6c65642066726f6d2074)
        mstore(add(pos_227, 64), 0x68697320636f6e74726163740000000000000000000000000000000000000000)
        end_228 := add(pos_227, 96)
    }
    function abi_encode_t_stringliteral_cd36462b17a97c5a3df33333c859d5933a4fb7f5e1a0750f5def8eb51f3272e4_to_t_string_memory_ptr(pos_229) -> end_230
    {
        mstore(pos_229, 48)
        mstore(add(pos_229, 32), 0x4d6574686f642063616e206f6e6c792062652063616c6c65642066726f6d2061)
        mstore(add(pos_229, 64), 0x6e20656e61626c6564206d6f64756c6500000000000000000000000000000000)
        end_230 := add(pos_229, 96)
    }
    function abi_encode_t_stringliteral_e2a11e15f7be1214c1340779ad55027af8aa33aee6cb521776a28a0a44aea377_to_t_string_memory_ptr(pos_231) -> end_232
    {
        mstore(pos_231, 34)
        mstore(add(pos_231, 32), 0x436f756c64206e6f74207061792067617320636f737473207769746820657468)
        mstore(add(pos_231, 64), 0x6572000000000000000000000000000000000000000000000000000000000000)
        end_232 := add(pos_231, 96)
    }
    function abi_encode_t_stringliteral_e7ccb05a0f2c66d12451cdfc6bbab488c38ab704d0f6af9ad18763542e9e5f18_to_t_string_memory_ptr(pos_233) -> end_234
    {
        mstore(pos_233, 42)
        mstore(add(pos_233, 32), 0x4e6f7420656e6f7567682067617320746f206578656375746520736166652074)
        mstore(add(pos_233, 64), 0x72616e73616374696f6e00000000000000000000000000000000000000000000)
        end_234 := add(pos_233, 96)
    }
    function abi_encode_t_uint256_to_t_uint256(value_235, pos_236)
    {
        mstore(pos_236, cleanup_assert_t_uint256(value_235))
    }
    function abi_encode_tuple_t_address__to_t_address_(headStart_237, value0_238) -> tail
    {
        tail := add(headStart_237, 32)
        abi_encode_t_address_to_t_address(value0_238, add(headStart_237, 0))
    }
    function abi_encode_tuple_t_address_t_uint256__to_t_address_t_uint256_(headStart_239, value1_240, value0_241) -> tail_242
    {
        tail_242 := add(headStart_239, 64)
        abi_encode_t_address_to_t_address(value0_241, add(headStart_239, 0))
        abi_encode_t_uint256_to_t_uint256(value1_240, add(headStart_239, 32))
    }
    function abi_encode_tuple_t_array$_t_address_$dyn_memory_ptr__to_t_array$_t_address_$dyn_memory_ptr_(headStart_243, value0_244) -> tail_245
    {
        tail_245 := add(headStart_243, 32)
        mstore(add(headStart_243, 0), sub(tail_245, headStart_243))
        tail_245 := abi_encode_t_array$_t_address_$dyn_memory_ptr_to_t_array$_t_address_$dyn_memory_ptr(value0_244, tail_245)
    }
    function abi_encode_tuple_t_bool__to_t_bool_(headStart_246, value0_247) -> tail_248
    {
        tail_248 := add(headStart_246, 32)
        abi_encode_t_bool_to_t_bool(value0_247, add(headStart_246, 0))
    }
    function abi_encode_tuple_t_bytes32__to_t_bytes32_(headStart_249, value0_250) -> tail_251
    {
        tail_251 := add(headStart_249, 32)
        abi_encode_t_bytes32_to_t_bytes32(value0_250, add(headStart_249, 0))
    }
    function abi_encode_tuple_t_bytes32_t_address_t_uint256_t_bytes32_t_enum$_Operation_$1949_t_uint256_t_uint256_t_uint256_t_address_t_address_t_uint256__to_t_bytes32_t_address_t_uint256_t_bytes32_t_uint8_t_uint256_t_uint256_t_uint256_t_address_t_address_t_uint256_(headStart_252, value10_253, value9_254, value8_255, value7_256, value6_257, value5_258, value4_259, value3_260, value2_261, value1_262, value0_263) -> tail_264
    {
        tail_264 := add(headStart_252, 352)
        abi_encode_t_bytes32_to_t_bytes32(value0_263, add(headStart_252, 0))
        abi_encode_t_address_to_t_address(value1_262, add(headStart_252, 32))
        abi_encode_t_uint256_to_t_uint256(value2_261, add(headStart_252, 64))
        abi_encode_t_bytes32_to_t_bytes32(value3_260, add(headStart_252, 96))
        abi_encode_t_enum$_Operation_$1949_to_t_uint8(value4_259, add(headStart_252, 128))
        abi_encode_t_uint256_to_t_uint256(value5_258, add(headStart_252, 160))
        abi_encode_t_uint256_to_t_uint256(value6_257, add(headStart_252, 192))
        abi_encode_t_uint256_to_t_uint256(value7_256, add(headStart_252, 224))
        abi_encode_t_address_to_t_address(value8_255, add(headStart_252, 256))
        abi_encode_t_address_to_t_address(value9_254, add(headStart_252, 288))
        abi_encode_t_uint256_to_t_uint256(value10_253, add(headStart_252, 320))
    }
    function abi_encode_tuple_t_bytes32_t_bytes32__to_t_bytes32_t_bytes32_(headStart_265, value1_266, value0_267) -> tail_268
    {
        tail_268 := add(headStart_265, 64)
        abi_encode_t_bytes32_to_t_bytes32(value0_267, add(headStart_265, 0))
        abi_encode_t_bytes32_to_t_bytes32(value1_266, add(headStart_265, 32))
    }
    function abi_encode_tuple_t_bytes32_t_contract$_GnosisSafe_$710__to_t_bytes32_t_address_payable_(headStart_269, value1_270, value0_271) -> tail_272
    {
        tail_272 := add(headStart_269, 64)
        abi_encode_t_bytes32_to_t_bytes32(value0_271, add(headStart_269, 0))
        abi_encode_t_contract$_GnosisSafe_$710_to_t_address_payable(value1_270, add(headStart_269, 32))
    }
    function abi_encode_tuple_t_bytes_memory_ptr__to_t_bytes_memory_ptr_(headStart_273, value0_274) -> tail_275
    {
        tail_275 := add(headStart_273, 32)
        mstore(add(headStart_273, 0), sub(tail_275, headStart_273))
        tail_275 := abi_encode_t_bytes_memory_ptr_to_t_bytes_memory_ptr(value0_274, tail_275)
    }
    function abi_encode_tuple_t_bytes_memory_ptr_t_bytes_memory_ptr__to_t_bytes_memory_ptr_t_bytes_memory_ptr_(headStart_276, value1_277, value0_278) -> tail_279
    {
        tail_279 := add(headStart_276, 64)
        mstore(add(headStart_276, 0), sub(tail_279, headStart_276))
        tail_279 := abi_encode_t_bytes_memory_ptr_to_t_bytes_memory_ptr(value0_278, tail_279)
        mstore(add(headStart_276, 32), sub(tail_279, headStart_276))
        tail_279 := abi_encode_t_bytes_memory_ptr_to_t_bytes_memory_ptr(value1_277, tail_279)
    }
    function abi_encode_tuple_t_contract$_Module_$1038__to_t_address_(headStart_280, value0_281) -> tail_282
    {
        tail_282 := add(headStart_280, 32)
        abi_encode_t_contract$_Module_$1038_to_t_address(value0_281, add(headStart_280, 0))
    }
    function abi_encode_tuple_t_string_memory__to_t_string_memory_ptr_(headStart_283, value0_284) -> tail_285
    {
        tail_285 := add(headStart_283, 32)
        mstore(add(headStart_283, 0), sub(tail_285, headStart_283))
        tail_285 := abi_encode_t_string_memory_to_t_string_memory_ptr(value0_284, tail_285)
    }
    function abi_encode_tuple_t_string_memory_ptr__to_t_string_memory_ptr_(headStart_286, value0_287) -> tail_288
    {
        tail_288 := add(headStart_286, 32)
        mstore(add(headStart_286, 0), sub(tail_288, headStart_286))
        tail_288 := abi_encode_t_string_memory_ptr_to_t_string_memory_ptr(value0_287, tail_288)
    }
    function abi_encode_tuple_t_stringliteral_108d84599042957b954e89d43b52f80be89321dfc114a37800028eba58dafc87__to_t_string_memory_ptr_(headStart_289) -> tail_290
    {
        tail_290 := add(headStart_289, 32)
        mstore(add(headStart_289, 0), sub(tail_290, headStart_289))
        tail_290 := abi_encode_t_stringliteral_108d84599042957b954e89d43b52f80be89321dfc114a37800028eba58dafc87_to_t_string_memory_ptr(tail_290)
    }
    function abi_encode_tuple_t_stringliteral_1e0428ffa69bff65645154a36d5017c238f946ddaf89430d30eec813f30bdd77__to_t_string_memory_ptr_(headStart_291) -> tail_292
    {
        tail_292 := add(headStart_291, 32)
        mstore(add(headStart_291, 0), sub(tail_292, headStart_291))
        tail_292 := abi_encode_t_stringliteral_1e0428ffa69bff65645154a36d5017c238f946ddaf89430d30eec813f30bdd77_to_t_string_memory_ptr(tail_292)
    }
    function abi_encode_tuple_t_stringliteral_21a1cd38818adb750881fbf07c26ce7223dde608fdd9dadd31a0d41afeca2094__to_t_string_memory_ptr_(headStart_293) -> tail_294
    {
        tail_294 := add(headStart_293, 32)
        mstore(add(headStart_293, 0), sub(tail_294, headStart_293))
        tail_294 := abi_encode_t_stringliteral_21a1cd38818adb750881fbf07c26ce7223dde608fdd9dadd31a0d41afeca2094_to_t_string_memory_ptr(tail_294)
    }
    function abi_encode_tuple_t_stringliteral_5caa315f9c5cf61be71c182eef2dc9ef7b6ce6b42c320d36694e1d23e09c287e__to_t_string_memory_ptr_(headStart_295) -> tail_296
    {
        tail_296 := add(headStart_295, 32)
        mstore(add(headStart_295, 0), sub(tail_296, headStart_295))
        tail_296 := abi_encode_t_stringliteral_5caa315f9c5cf61be71c182eef2dc9ef7b6ce6b42c320d36694e1d23e09c287e_to_t_string_memory_ptr(tail_296)
    }
    function abi_encode_tuple_t_stringliteral_60f21058f4a7689ef29853b3c9c17c9bf69856a949794649bb68878f00552475__to_t_string_memory_ptr_(headStart_297) -> tail_298
    {
        tail_298 := add(headStart_297, 32)
        mstore(add(headStart_297, 0), sub(tail_298, headStart_297))
        tail_298 := abi_encode_t_stringliteral_60f21058f4a7689ef29853b3c9c17c9bf69856a949794649bb68878f00552475_to_t_string_memory_ptr(tail_298)
    }
    function abi_encode_tuple_t_stringliteral_63d26a9feb8568677e5c255c04e4da88e86a25137d5152a9a089790b7e710e86__to_t_string_memory_ptr_(headStart_299) -> tail_300
    {
        tail_300 := add(headStart_299, 32)
        mstore(add(headStart_299, 0), sub(tail_300, headStart_299))
        tail_300 := abi_encode_t_stringliteral_63d26a9feb8568677e5c255c04e4da88e86a25137d5152a9a089790b7e710e86_to_t_string_memory_ptr(tail_300)
    }
    function abi_encode_tuple_t_stringliteral_7913a3f9168bf3e458e3f42eb08db5c4b33f44228d345660887090b75e24c6aa__to_t_string_memory_ptr_(headStart_301) -> tail_302
    {
        tail_302 := add(headStart_301, 32)
        mstore(add(headStart_301, 0), sub(tail_302, headStart_301))
        tail_302 := abi_encode_t_stringliteral_7913a3f9168bf3e458e3f42eb08db5c4b33f44228d345660887090b75e24c6aa_to_t_string_memory_ptr(tail_302)
    }
    function abi_encode_tuple_t_stringliteral_839b4c4db845de24ec74ef067d85431087d6987a4c904418ee4f6ec699c02482__to_t_string_memory_ptr_(headStart_303) -> tail_304
    {
        tail_304 := add(headStart_303, 32)
        mstore(add(headStart_303, 0), sub(tail_304, headStart_303))
        tail_304 := abi_encode_t_stringliteral_839b4c4db845de24ec74ef067d85431087d6987a4c904418ee4f6ec699c02482_to_t_string_memory_ptr(tail_304)
    }
    function abi_encode_tuple_t_stringliteral_8560a13547eca5648355c8db1a9f8653b6f657d31d476c36bca25e47b45b08f4__to_t_string_memory_ptr_(headStart_305) -> tail_306
    {
        tail_306 := add(headStart_305, 32)
        mstore(add(headStart_305, 0), sub(tail_306, headStart_305))
        tail_306 := abi_encode_t_stringliteral_8560a13547eca5648355c8db1a9f8653b6f657d31d476c36bca25e47b45b08f4_to_t_string_memory_ptr(tail_306)
    }
    function abi_encode_tuple_t_stringliteral_85bcea44c930431ef19052d068cc504a81260341ae6c5ee84bb5a38ec55acf05__to_t_string_memory_ptr_(headStart_307) -> tail_308
    {
        tail_308 := add(headStart_307, 32)
        mstore(add(headStart_307, 0), sub(tail_308, headStart_307))
        tail_308 := abi_encode_t_stringliteral_85bcea44c930431ef19052d068cc504a81260341ae6c5ee84bb5a38ec55acf05_to_t_string_memory_ptr(tail_308)
    }
    function abi_encode_tuple_t_stringliteral_8c2199b479423c52a835dfe8b0f2e9eb4c1ec1069ba198ccc38077a4a88a5c00__to_t_string_memory_ptr_(headStart_309) -> tail_310
    {
        tail_310 := add(headStart_309, 32)
        mstore(add(headStart_309, 0), sub(tail_310, headStart_309))
        tail_310 := abi_encode_t_stringliteral_8c2199b479423c52a835dfe8b0f2e9eb4c1ec1069ba198ccc38077a4a88a5c00_to_t_string_memory_ptr(tail_310)
    }
    function abi_encode_tuple_t_stringliteral_960698caed81fce71c9b7d572ab2e035b6014a5b812b51df8462ea9817fc4ebc__to_t_string_memory_ptr_(headStart_311) -> tail_312
    {
        tail_312 := add(headStart_311, 32)
        mstore(add(headStart_311, 0), sub(tail_312, headStart_311))
        tail_312 := abi_encode_t_stringliteral_960698caed81fce71c9b7d572ab2e035b6014a5b812b51df8462ea9817fc4ebc_to_t_string_memory_ptr(tail_312)
    }
    function abi_encode_tuple_t_stringliteral_9a45ae898fbe2bd07a0b33b5a6c421f76198e9deb66843b8d827b0b9e4a16f86__to_t_string_memory_ptr_(headStart_313) -> tail_314
    {
        tail_314 := add(headStart_313, 32)
        mstore(add(headStart_313, 0), sub(tail_314, headStart_313))
        tail_314 := abi_encode_t_stringliteral_9a45ae898fbe2bd07a0b33b5a6c421f76198e9deb66843b8d827b0b9e4a16f86_to_t_string_memory_ptr(tail_314)
    }
    function abi_encode_tuple_t_stringliteral_9d461d71e19b25cd406798d062d7e61f961ad52541d3077a543e857810427d47__to_t_string_memory_ptr_(headStart_315) -> tail_316
    {
        tail_316 := add(headStart_315, 32)
        mstore(add(headStart_315, 0), sub(tail_316, headStart_315))
        tail_316 := abi_encode_t_stringliteral_9d461d71e19b25cd406798d062d7e61f961ad52541d3077a543e857810427d47_to_t_string_memory_ptr(tail_316)
    }
    function abi_encode_tuple_t_stringliteral_a2e1f2db9cd32eaa6a2caa3d6caa726a30dc0417d866440bfe13d6a6d030e5e2__to_t_string_memory_ptr_(headStart_317) -> tail_318
    {
        tail_318 := add(headStart_317, 32)
        mstore(add(headStart_317, 0), sub(tail_318, headStart_317))
        tail_318 := abi_encode_t_stringliteral_a2e1f2db9cd32eaa6a2caa3d6caa726a30dc0417d866440bfe13d6a6d030e5e2_to_t_string_memory_ptr(tail_318)
    }
    function abi_encode_tuple_t_stringliteral_a803fa289679098e38a7f1f6fe43056918c5ab5af07441cb8db77b949c165ca1__to_t_string_memory_ptr_(headStart_319) -> tail_320
    {
        tail_320 := add(headStart_319, 32)
        mstore(add(headStart_319, 0), sub(tail_320, headStart_319))
        tail_320 := abi_encode_t_stringliteral_a803fa289679098e38a7f1f6fe43056918c5ab5af07441cb8db77b949c165ca1_to_t_string_memory_ptr(tail_320)
    }
    function abi_encode_tuple_t_stringliteral_ae2b4ea52eaf6de3fb2d8a64b7555be2dfd285b837a62821bf24e7dc6f329450__to_t_string_memory_ptr_(headStart_321) -> tail_322
    {
        tail_322 := add(headStart_321, 32)
        mstore(add(headStart_321, 0), sub(tail_322, headStart_321))
        tail_322 := abi_encode_t_stringliteral_ae2b4ea52eaf6de3fb2d8a64b7555be2dfd285b837a62821bf24e7dc6f329450_to_t_string_memory_ptr(tail_322)
    }
    function abi_encode_tuple_t_stringliteral_b995394ed6031392a784e6dd5e04285cca83077a8dc3873d2fb7fcb090297ab4__to_t_string_memory_ptr_(headStart_323) -> tail_324
    {
        tail_324 := add(headStart_323, 32)
        mstore(add(headStart_323, 0), sub(tail_324, headStart_323))
        tail_324 := abi_encode_t_stringliteral_b995394ed6031392a784e6dd5e04285cca83077a8dc3873d2fb7fcb090297ab4_to_t_string_memory_ptr(tail_324)
    }
    function abi_encode_tuple_t_stringliteral_c4780ef0a1d41d59bac8c510cf9ada421bccf2b90f75a8e4ba2e8c09e8d72733__to_t_string_memory_ptr_(headStart_325) -> tail_326
    {
        tail_326 := add(headStart_325, 32)
        mstore(add(headStart_325, 0), sub(tail_326, headStart_325))
        tail_326 := abi_encode_t_stringliteral_c4780ef0a1d41d59bac8c510cf9ada421bccf2b90f75a8e4ba2e8c09e8d72733_to_t_string_memory_ptr(tail_326)
    }
    function abi_encode_tuple_t_stringliteral_cd36462b17a97c5a3df33333c859d5933a4fb7f5e1a0750f5def8eb51f3272e4__to_t_string_memory_ptr_(headStart_327) -> tail_328
    {
        tail_328 := add(headStart_327, 32)
        mstore(add(headStart_327, 0), sub(tail_328, headStart_327))
        tail_328 := abi_encode_t_stringliteral_cd36462b17a97c5a3df33333c859d5933a4fb7f5e1a0750f5def8eb51f3272e4_to_t_string_memory_ptr(tail_328)
    }
    function abi_encode_tuple_t_stringliteral_e2a11e15f7be1214c1340779ad55027af8aa33aee6cb521776a28a0a44aea377__to_t_string_memory_ptr_(headStart_329) -> tail_330
    {
        tail_330 := add(headStart_329, 32)
        mstore(add(headStart_329, 0), sub(tail_330, headStart_329))
        tail_330 := abi_encode_t_stringliteral_e2a11e15f7be1214c1340779ad55027af8aa33aee6cb521776a28a0a44aea377_to_t_string_memory_ptr(tail_330)
    }
    function abi_encode_tuple_t_stringliteral_e7ccb05a0f2c66d12451cdfc6bbab488c38ab704d0f6af9ad18763542e9e5f18__to_t_string_memory_ptr_(headStart_331) -> tail_332
    {
        tail_332 := add(headStart_331, 32)
        mstore(add(headStart_331, 0), sub(tail_332, headStart_331))
        tail_332 := abi_encode_t_stringliteral_e7ccb05a0f2c66d12451cdfc6bbab488c38ab704d0f6af9ad18763542e9e5f18_to_t_string_memory_ptr(tail_332)
    }
    function abi_encode_tuple_t_uint256__to_t_uint256_(headStart_333, value0_334) -> tail_335
    {
        tail_335 := add(headStart_333, 32)
        abi_encode_t_uint256_to_t_uint256(value0_334, add(headStart_333, 0))
    }
    function allocateMemory(size) -> memPtr
    {
        memPtr := mload(64)
        let newFreePtr := add(memPtr, size)
        if or(gt(newFreePtr, 0xffffffffffffffff), lt(newFreePtr, memPtr))
        {
            revert(0, 0)
        }
        mstore(64, newFreePtr)
    }
    function array_allocation_size_t_bytes_memory_ptr(length_336) -> size_337
    {
        if gt(length_336, 0xffffffffffffffff)
        {
            revert(0, 0)
        }
        size_337 := and(add(length_336, 0x1f), not(0x1f))
        size_337 := add(size_337, 0x20)
    }
    function array_dataslot_t_array$_t_address_$dyn_memory_ptr(memPtr_338) -> dataPtr
    {
        dataPtr := add(memPtr_338, 0x20)
    }
    function array_length_t_array$_t_address_$dyn_memory_ptr(value_339) -> length_340
    {
        length_340 := mload(value_339)
    }
    function array_length_t_bytes_memory_ptr(value_341) -> length_342
    {
        length_342 := mload(value_341)
    }
    function array_length_t_string_memory(value_343) -> length_344
    {
        length_344 := mload(value_343)
    }
    function array_length_t_string_memory_ptr(value_345) -> length_346
    {
        length_346 := mload(value_345)
    }
    function array_nextElement_t_array$_t_address_$dyn_memory_ptr(memPtr_347) -> nextPtr
    {
        nextPtr := add(memPtr_347, 0x20)
    }
    function cleanup_assert_t_address(value_348) -> cleaned
    {
        cleaned := cleanup_assert_t_uint160(value_348)
    }
    function cleanup_assert_t_bool(value_349) -> cleaned_350
    {
        cleaned_350 := iszero(iszero(value_349))
    }
    function cleanup_assert_t_bytes32(value_351) -> cleaned_352
    {
        cleaned_352 := value_351
    }
    function cleanup_assert_t_enum$_Operation_$1949(value_353) -> cleaned_354
    {
        if iszero(lt(value_353, 3))
        {
            invalid()
        }
        cleaned_354 := value_353
    }
    function cleanup_assert_t_uint160(value_355) -> cleaned_356
    {
        cleaned_356 := and(value_355, 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF)
    }
    function cleanup_assert_t_uint256(value_357) -> cleaned_358
    {
        cleaned_358 := value_357
    }
    function cleanup_revert_t_address(value_359) -> cleaned_360
    {
        cleaned_360 := cleanup_assert_t_uint160(value_359)
    }
    function cleanup_revert_t_address_payable(value_361) -> cleaned_362
    {
        cleaned_362 := cleanup_assert_t_uint160(value_361)
    }
    function cleanup_revert_t_bool(value_363) -> cleaned_364
    {
        cleaned_364 := iszero(iszero(value_363))
    }
    function cleanup_revert_t_bytes32(value_365) -> cleaned_366
    {
        cleaned_366 := value_365
    }
    function cleanup_revert_t_contract$_Module_$1038(value_367) -> cleaned_368
    {
        cleaned_368 := cleanup_assert_t_address(value_367)
    }
    function cleanup_revert_t_enum$_Operation_$1949(value_369) -> cleaned_370
    {
        if iszero(lt(value_369, 3))
        {
            revert(0, 0)
        }
        cleaned_370 := value_369
    }
    function cleanup_revert_t_uint256(value_371) -> cleaned_372
    {
        cleaned_372 := value_371
    }
    function convert_t_contract$_GnosisSafe_$710_to_t_address_payable(value_373) -> converted
    {
        converted := convert_t_contract$_GnosisSafe_$710_to_t_uint160(value_373)
    }
    function convert_t_contract$_GnosisSafe_$710_to_t_uint160(value_374) -> converted_375
    {
        converted_375 := cleanup_assert_t_uint160(value_374)
    }
    function convert_t_contract$_Module_$1038_to_t_address(value_376) -> converted_377
    {
        converted_377 := convert_t_contract$_Module_$1038_to_t_uint160(value_376)
    }
    function convert_t_contract$_Module_$1038_to_t_uint160(value_378) -> converted_379
    {
        converted_379 := cleanup_assert_t_uint160(value_378)
    }
    function convert_t_enum$_Operation_$1949_to_t_uint8(value_380) -> converted_381
    {
        converted_381 := cleanup_assert_t_enum$_Operation_$1949(value_380)
    }
    function copy_calldata_to_memory(src_382, dst_383, length_384)
    {
        calldatacopy(dst_383, src_382, length_384)
        mstore(add(dst_383, length_384), 0)
    }
    function copy_memory_to_memory(src_385, dst_386, length_387)
    {
        let i_388 := 0
        for {
        }
        lt(i_388, length_387)
        {
            i_388 := add(i_388, 32)
        }
        {
            mstore(add(dst_386, i_388), mload(add(src_385, i_388)))
        }
        if gt(i_388, length_387)
        {
            mstore(add(dst_386, length_387), 0)
        }
    }
    function round_up_to_mul_of_32(value_389) -> result
    {
        result := and(add(value_389, 31), not(31))
    }
}
// ----
// fullSuite
// {
//     let _1 := mload(1)
//     let _2 := mload(0)
//     if slt(sub(_1, _2), 64)
//     {
//         revert(0, 0)
//     }
//     sstore(0, and(calldataload(_2), 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF))
//     let x0, x1, x2, x3, x4 := abi_decode_tuple_t_addresst_uint256t_bytes_calldata_ptrt_enum$_Operation_$1949(mload(7), mload(8))
//     sstore(x1, x0)
//     sstore(x3, x2)
//     sstore(1, x4)
//     pop(abi_encode_tuple_t_bytes32_t_address_t_uint256_t_bytes32_t_enum$_Operation_$1949_t_uint256_t_uint256_t_uint256_t_address_t_address_t_uint256__to_t_bytes32_t_address_t_uint256_t_bytes32_t_uint8_t_uint256_t_uint256_t_uint256_t_address_t_address_t_uint256_(mload(30), mload(31), mload(32), mload(33), mload(34), mload(35), mload(36), mload(37), mload(38), mload(39), mload(40), mload(41)))
//     function abi_decode_tuple_t_addresst_uint256t_bytes_calldata_ptrt_enum$_Operation_$1949(headStart, dataEnd) -> value0, value1, value2, value3, value4
//     {
//         if slt(sub(dataEnd, headStart), 128)
//         {
//             revert(value4, value4)
//         }
//         value0 := and(calldataload(headStart), 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF)
//         value1 := calldataload(add(headStart, 32))
//         let offset := calldataload(add(headStart, 64))
//         let _1 := 0xffffffffffffffff
//         if gt(offset, _1)
//         {
//             revert(value4, value4)
//         }
//         let _2 := add(headStart, offset)
//         if iszero(slt(add(_2, 0x1f), dataEnd))
//         {
//             revert(value4, value4)
//         }
//         let abi_decode_length := calldataload(_2)
//         if gt(abi_decode_length, _1)
//         {
//             revert(value4, value4)
//         }
//         if gt(add(add(_2, abi_decode_length), 32), dataEnd)
//         {
//             revert(value4, value4)
//         }
//         value2 := add(_2, 32)
//         value3 := abi_decode_length
//         let _3 := calldataload(add(headStart, 96))
//         if iszero(lt(_3, 3))
//         {
//             revert(value4, value4)
//         }
//         value4 := _3
//     }
//     function abi_encode_tuple_t_bytes32_t_address_t_uint256_t_bytes32_t_enum$_Operation_$1949_t_uint256_t_uint256_t_uint256_t_address_t_address_t_uint256__to_t_bytes32_t_address_t_uint256_t_bytes32_t_uint8_t_uint256_t_uint256_t_uint256_t_address_t_address_t_uint256_(headStart, value10, value9, value8, value7, value6, value5, value4, value3, value2, value1, value0) -> tail
//     {
//         tail := add(headStart, 352)
//         mstore(headStart, value0)
//         let _1 := 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
//         mstore(add(headStart, 32), and(value1, _1))
//         mstore(add(headStart, 64), value2)
//         mstore(add(headStart, 96), value3)
//         if iszero(lt(value4, 3))
//         {
//             invalid()
//         }
//         mstore(add(headStart, 128), value4)
//         mstore(add(headStart, 160), value5)
//         mstore(add(headStart, 192), value6)
//         mstore(add(headStart, 224), value7)
//         mstore(add(headStart, 256), and(value8, _1))
//         mstore(add(headStart, 288), and(value9, _1))
//         mstore(add(headStart, 320), value10)
//     }
// }
