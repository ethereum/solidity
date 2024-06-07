object "ERC20_403" {
    code ltac:(M.monadic (
      [ltac:(M.monadic (
        [M.call (|
          mstore,
          [
          ]
        |), M.if (|
          M.call (|
            callvalue,
            [
            ]
          |),
          ltac:(M.monadic (
            [M.call (|
              revert_error_ca66f745a3ce8ff40e2ccaf1ad45db7774001b90d25810abd9040049be7bf4bb,
              [
              ]
            |), ]
    ))
        |), M.call (|
          constructor_ERC20,
          [
          ]
        |), let _ :=
          M.assign (|
            [_1],
            M.call (|
              allocate_unbounded,
              [
              ]
            |)
          |) in, M.call (|
          codecopy,
          [
          ]
        |), M.call (|
          return,
          [
          ]
        |), ]
    )), M.function (|
        allocate_unbounded,
        [
        ],
        [
          memPtr,
        ],
        ltac:(M.monadic (
          [let _ :=
            M.declare (|
              [memPtr],
              M.call (|
                mload,
                [
                ]
              |)
            |) in, ]
    ))
      |), M.function (|
        revert_error_ca66f745a3ce8ff40e2ccaf1ad45db7774001b90d25810abd9040049be7bf4bb,
        [
        ],
        [
        ],
        ltac:(M.monadic (
          [M.call (|
            revert,
            [
            ]
          |), ]
    ))
      |), M.function (|
        cleanup_rational_by,
        [
          value,
        ],
        [
          cleaned,
        ],
        ltac:(M.monadic (
          [let _ :=
            M.declare (|
              [cleaned],
              value
            |) in, ]
    ))
      |), M.function (|
        cleanup_uint256,
        [
          value,
        ],
        [
          cleaned,
        ],
        ltac:(M.monadic (
          [let _ :=
            M.declare (|
              [cleaned],
              value
            |) in, ]
    ))
      |), M.function (|
        identity,
        [
          value,
        ],
        [
          ret,
        ],
        ltac:(M.monadic (
          [let _ :=
            M.declare (|
              [ret],
              value
            |) in, ]
    ))
      |), M.function (|
        convert_rational_by_to_uint256,
        [
          value,
        ],
        [
          converted,
        ],
        ltac:(M.monadic (
          [let _ :=
            M.declare (|
              [converted],
              M.call (|
                cleanup_uint256,
                [
                ]
              |)
            |) in, ]
    ))
      |), M.function (|
        constructor_ERC20,
        [
        ],
        [
        ],
        ltac:(M.monadic (
          [let _ :=
            M.assign (|
              [expr],
              M.call (|
                caller,
                [
                ]
              |)
            |) in, let _ :=
            M.assign (|
              [expr_1],
              Literal.number 0x14
            |) in, let _ :=
            M.assign (|
              [_1],
              M.call (|
                convert_rational_by_to_uint256,
                [
                ]
              |)
            |) in, M.call (|
            fun_mint,
            [
            ]
          |), ]
    ))
      |), M.function (|
        cleanup_t_rational_by,
        [
          value,
        ],
        [
          cleaned,
        ],
        ltac:(M.monadic (
          [let _ :=
            M.declare (|
              [cleaned],
              value
            |) in, ]
    ))
      |), M.function (|
        cleanup_uint160,
        [
          value,
        ],
        [
          cleaned,
        ],
        ltac:(M.monadic (
          [let _ :=
            M.declare (|
              [cleaned],
              M.call (|
                and,
                [
                ]
              |)
            |) in, ]
    ))
      |), M.function (|
        convert_rational_by_to_uint160,
        [
          value,
        ],
        [
          converted,
        ],
        ltac:(M.monadic (
          [let _ :=
            M.declare (|
              [converted],
              M.call (|
                cleanup_uint160,
                [
                ]
              |)
            |) in, ]
    ))
      |), M.function (|
        convert_rational_by_to_address,
        [
          value,
        ],
        [
          converted,
        ],
        ltac:(M.monadic (
          [let _ :=
            M.declare (|
              [converted],
              M.call (|
                convert_rational_by_to_uint160,
                [
                ]
              |)
            |) in, ]
    ))
      |), M.function (|
        cleanup_address,
        [
          value,
        ],
        [
          cleaned,
        ],
        ltac:(M.monadic (
          [let _ :=
            M.declare (|
              [cleaned],
              M.call (|
                cleanup_uint160,
                [
                ]
              |)
            |) in, ]
    ))
      |), M.function (|
        array_storeLengthForEncoding_string,
        [
          pos,
          length,
        ],
        [
          updated_pos,
        ],
        ltac:(M.monadic (
          [M.call (|
            mstore,
            [
            ]
          |), let _ :=
            M.declare (|
              [updated_pos],
              M.call (|
                add,
                [
                ]
              |)
            |) in, ]
    ))
      |), M.function (|
        store_literal_in_memory_fc0b381caf0a47702017f3c4b358ebe3d3aff6c60ce819a8bf3ef5a95d4f202e,
        [
          memPtr,
        ],
        [
        ],
        ltac:(M.monadic (
          [M.call (|
            mstore,
            [
            ]
          |), ]
    ))
      |), M.function (|
        abi_encode_stringliteral_fc0b381caf0a47702017f3c4b358ebe3d3aff6c60ce819a8bf3ef5a95d4f202e,
        [
          pos,
        ],
        [
          end,
        ],
        ltac:(M.monadic (
          [let _ :=
            M.declare (|
              [pos],
              M.call (|
                array_storeLengthForEncoding_string,
                [
                ]
              |)
            |) in, M.call (|
            store_literal_in_memory_fc0b381caf0a47702017f3c4b358ebe3d3aff6c60ce819a8bf3ef5a95d4f202e,
            [
            ]
          |), let _ :=
            M.declare (|
              [end],
              M.call (|
                add,
                [
                ]
              |)
            |) in, ]
    ))
      |), M.function (|
        abi_encode_stringliteral_fc0b,
        [
          headStart,
        ],
        [
          tail,
        ],
        ltac:(M.monadic (
          [let _ :=
            M.declare (|
              [tail],
              M.call (|
                add,
                [
                ]
              |)
            |) in, M.call (|
            mstore,
            [
            ]
          |), let _ :=
            M.declare (|
              [tail],
              M.call (|
                abi_encode_stringliteral_fc0b381caf0a47702017f3c4b358ebe3d3aff6c60ce819a8bf3ef5a95d4f202e,
                [
                ]
              |)
            |) in, ]
    ))
      |), M.function (|
        require_helper_stringliteral_fc0b,
        [
          condition,
        ],
        [
        ],
        ltac:(M.monadic (
          [M.if (|
            M.call (|
              iszero,
              [
              ]
            |),
            ltac:(M.monadic (
              [let _ :=
                M.assign (|
                  [memPtr],
                  M.call (|
                    allocate_unbounded,
                    [
                    ]
                  |)
                |) in, M.call (|
                mstore,
                [
                ]
              |), let _ :=
                M.assign (|
                  [end],
                  M.call (|
                    abi_encode_stringliteral_fc0b,
                    [
                    ]
                  |)
                |) in, M.call (|
                revert,
                [
                ]
              |), ]
    ))
          |), ]
    ))
      |), M.function (|
        shift_right_unsigned,
        [
          value,
        ],
        [
          newValue,
        ],
        ltac:(M.monadic (
          [let _ :=
            M.declare (|
              [newValue],
              M.call (|
                shr,
                [
                ]
              |)
            |) in, ]
    ))
      |), M.function (|
        cleanup_from_storage_uint256,
        [
          value,
        ],
        [
          cleaned,
        ],
        ltac:(M.monadic (
          [let _ :=
            M.declare (|
              [cleaned],
              value
            |) in, ]
    ))
      |), M.function (|
        extract_from_storage_value_offsett_uint256,
        [
          slot_value,
        ],
        [
          value,
        ],
        ltac:(M.monadic (
          [let _ :=
            M.declare (|
              [value],
              M.call (|
                cleanup_from_storage_uint256,
                [
                ]
              |)
            |) in, ]
    ))
      |), M.function (|
        read_from_storage_split_offset_uint256,
        [
          slot,
        ],
        [
          value,
        ],
        ltac:(M.monadic (
          [let _ :=
            M.declare (|
              [value],
              M.call (|
                extract_from_storage_value_offsett_uint256,
                [
                ]
              |)
            |) in, ]
    ))
      |), M.function (|
        panic_error_0x11,
        [
        ],
        [
        ],
        ltac:(M.monadic (
          [M.call (|
            mstore,
            [
            ]
          |), M.call (|
            mstore,
            [
            ]
          |), M.call (|
            revert,
            [
            ]
          |), ]
    ))
      |), M.function (|
        checked_add_uint256,
        [
          x,
          y,
        ],
        [
          sum,
        ],
        ltac:(M.monadic (
          [let _ :=
            M.declare (|
              [x],
              M.call (|
                cleanup_uint256,
                [
                ]
              |)
            |) in, let _ :=
            M.declare (|
              [y],
              M.call (|
                cleanup_uint256,
                [
                ]
              |)
            |) in, let _ :=
            M.declare (|
              [sum],
              M.call (|
                add,
                [
                ]
              |)
            |) in, M.if (|
            M.call (|
              gt,
              [
              ]
            |),
            ltac:(M.monadic (
              [M.call (|
                panic_error_0x11,
                [
                ]
              |), ]
    ))
          |), ]
    ))
      |), M.function (|
        shift_left,
        [
          value,
        ],
        [
          newValue,
        ],
        ltac:(M.monadic (
          [let _ :=
            M.declare (|
              [newValue],
              M.call (|
                shl,
                [
                ]
              |)
            |) in, ]
    ))
      |), M.function (|
        update_byte_slice_shift,
        [
          value,
          toInsert,
        ],
        [
          result,
        ],
        ltac:(M.monadic (
          [let _ :=
            M.assign (|
              [mask],
              M.call (|
                not,
                [
                ]
              |)
            |) in, let _ :=
            M.declare (|
              [toInsert],
              M.call (|
                shift_left,
                [
                ]
              |)
            |) in, let _ :=
            M.declare (|
              [value],
              M.call (|
                and,
                [
                ]
              |)
            |) in, let _ :=
            M.declare (|
              [result],
              M.call (|
                or,
                [
                ]
              |)
            |) in, ]
    ))
      |), M.function (|
        convert_uint256_to_uint256,
        [
          value,
        ],
        [
          converted,
        ],
        ltac:(M.monadic (
          [let _ :=
            M.declare (|
              [converted],
              M.call (|
                cleanup_uint256,
                [
                ]
              |)
            |) in, ]
    ))
      |), M.function (|
        prepare_store_uint256,
        [
          value,
        ],
        [
          ret,
        ],
        ltac:(M.monadic (
          [let _ :=
            M.declare (|
              [ret],
              value
            |) in, ]
    ))
      |), M.function (|
        update_storage_value_offsett_uint256_to_uint256,
        [
          slot,
          value,
        ],
        [
        ],
        ltac:(M.monadic (
          [let _ :=
            M.assign (|
              [convertedValue],
              M.call (|
                convert_uint256_to_uint256,
                [
                ]
              |)
            |) in, M.call (|
            sstore,
            [
            ]
          |), ]
    ))
      |), M.function (|
        convert_uint160_to_uint160,
        [
          value,
        ],
        [
          converted,
        ],
        ltac:(M.monadic (
          [let _ :=
            M.declare (|
              [converted],
              M.call (|
                cleanup_uint160,
                [
                ]
              |)
            |) in, ]
    ))
      |), M.function (|
        convert_uint160_to_address,
        [
          value,
        ],
        [
          converted,
        ],
        ltac:(M.monadic (
          [let _ :=
            M.declare (|
              [converted],
              M.call (|
                convert_uint160_to_uint160,
                [
                ]
              |)
            |) in, ]
    ))
      |), M.function (|
        convert_address_to_address,
        [
          value,
        ],
        [
          converted,
        ],
        ltac:(M.monadic (
          [let _ :=
            M.declare (|
              [converted],
              M.call (|
                convert_uint160_to_address,
                [
                ]
              |)
            |) in, ]
    ))
      |), M.function (|
        mapping_index_access_mapping_address_uint256_of_address,
        [
          slot,
          key,
        ],
        [
          dataSlot,
        ],
        ltac:(M.monadic (
          [M.call (|
            mstore,
            [
            ]
          |), M.call (|
            mstore,
            [
            ]
          |), let _ :=
            M.declare (|
              [dataSlot],
              M.call (|
                keccak256,
                [
                ]
              |)
            |) in, ]
    ))
      |), M.function (|
        abi_encode_uint256_to_uint256,
        [
          value,
          pos,
        ],
        [
        ],
        ltac:(M.monadic (
          [M.call (|
            mstore,
            [
            ]
          |), ]
    ))
      |), M.function (|
        abi_encode_uint256,
        [
          headStart,
          value0,
        ],
        [
          tail,
        ],
        ltac:(M.monadic (
          [let _ :=
            M.declare (|
              [tail],
              M.call (|
                add,
                [
                ]
              |)
            |) in, M.call (|
            abi_encode_uint256_to_uint256,
            [
            ]
          |), ]
    ))
      |), M.function (|
        fun_mint,
        [
          var_account,
          var_value,
        ],
        [
        ],
        ltac:(M.monadic (
          [let _ :=
            M.assign (|
              [_1],
              var_account
            |) in, let _ :=
            M.assign (|
              [expr],
              _1
            |) in, let _ :=
            M.assign (|
              [expr_1],
              Literal.number 0x00
            |) in, let _ :=
            M.assign (|
              [expr_2],
              M.call (|
                convert_rational_by_to_address,
                [
                ]
              |)
            |) in, let _ :=
            M.assign (|
              [expr_3],
              M.call (|
                iszero,
                [
                ]
              |)
            |) in, M.call (|
            require_helper_stringliteral_fc0b,
            [
            ]
          |), let _ :=
            M.assign (|
              [_2],
              M.call (|
                read_from_storage_split_offset_uint256,
                [
                ]
              |)
            |) in, let _ :=
            M.assign (|
              [expr_4],
              _2
            |) in, let _ :=
            M.assign (|
              [_3],
              var_value
            |) in, let _ :=
            M.assign (|
              [expr_5],
              _3
            |) in, let _ :=
            M.assign (|
              [expr_6],
              M.call (|
                checked_add_uint256,
                [
                ]
              |)
            |) in, M.call (|
            update_storage_value_offsett_uint256_to_uint256,
            [
            ]
          |), let _ :=
            M.assign (|
              [_6_slot],
              Literal.number 0x00
            |) in, let _ :=
            M.assign (|
              [expr_271_slot],
              _6_slot
            |) in, let _ :=
            M.assign (|
              [_4],
              var_account
            |) in, let _ :=
            M.assign (|
              [expr_7],
              _4
            |) in, let _ :=
            M.assign (|
              [_5],
              M.call (|
                mapping_index_access_mapping_address_uint256_of_address,
                [
                ]
              |)
            |) in, let _ :=
            M.assign (|
              [_6],
              M.call (|
                read_from_storage_split_offset_uint256,
                [
                ]
              |)
            |) in, let _ :=
            M.assign (|
              [expr_8],
              _6
            |) in, let _ :=
            M.assign (|
              [_7],
              var_value
            |) in, let _ :=
            M.assign (|
              [expr_9],
              _7
            |) in, let _ :=
            M.assign (|
              [expr_10],
              M.call (|
                checked_add_uint256,
                [
                ]
              |)
            |) in, let _ :=
            M.assign (|
              [_slot],
              Literal.number 0x00
            |) in, let _ :=
            M.assign (|
              [expr_slot],
              _slot
            |) in, let _ :=
            M.assign (|
              [_8],
              var_account
            |) in, let _ :=
            M.assign (|
              [expr_11],
              _8
            |) in, let _ :=
            M.assign (|
              [_9],
              M.call (|
                mapping_index_access_mapping_address_uint256_of_address,
                [
                ]
              |)
            |) in, M.call (|
            update_storage_value_offsett_uint256_to_uint256,
            [
            ]
          |), let _ :=
            M.assign (|
              [expr_12],
              Literal.number 0x00
            |) in, let _ :=
            M.assign (|
              [expr_13],
              M.call (|
                convert_rational_by_to_address,
                [
                ]
              |)
            |) in, let _ :=
            M.assign (|
              [_10],
              var_account
            |) in, let _ :=
            M.assign (|
              [expr_14],
              _10
            |) in, let _ :=
            M.assign (|
              [_11],
              var_value
            |) in, let _ :=
            M.assign (|
              [expr_15],
              _11
            |) in, let _ :=
            M.assign (|
              [_12],
              Literal.number 0xddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef
            |) in, let _ :=
            M.assign (|
              [_13],
              M.call (|
                convert_address_to_address,
                [
                ]
              |)
            |) in, let _ :=
            M.assign (|
              [_14],
              M.call (|
                convert_address_to_address,
                [
                ]
              |)
            |) in, let _ :=
            M.assign (|
              [_15],
              M.call (|
                allocate_unbounded,
                [
                ]
              |)
            |) in, let _ :=
            M.assign (|
              [_16],
              M.call (|
                abi_encode_uint256,
                [
                ]
              |)
            |) in, M.call (|
            log3,
            [
            ]
          |), ]
    ))
      |), ]
    ))
    object "ERC20_403_deployed" {
        code ltac:(M.monadic (
          [ltac:(M.monadic (
            [M.call (|
              mstore,
              [
              ]
            |), M.if (|
              M.call (|
                iszero,
                [
                ]
              |),
              ltac:(M.monadic (
                [let _ :=
                  M.assign (|
                    [selector],
                    M.call (|
                      shift_right_unsigned,
                      [
                      ]
                    |)
                  |) in, M.switch (|
                  selector,
                  [
                    M.case (|
                      Literal.number 0x095ea7b3,
                      ltac:(M.monadic (
                        [M.call (|
                          external_fun_approve,
                          [
                          ]
                        |), ]
        ))
                    |),
                    M.case (|
                      Literal.number 0x18160ddd,
                      ltac:(M.monadic (
                        [M.call (|
                          external_fun_totalSupply,
                          [
                          ]
                        |), ]
        ))
                    |),
                    M.case (|
                      Literal.number 0x23b872dd,
                      ltac:(M.monadic (
                        [M.call (|
                          external_fun_transferFrom,
                          [
                          ]
                        |), ]
        ))
                    |),
                    M.case (|
                      Literal.number 0x39509351,
                      ltac:(M.monadic (
                        [M.call (|
                          external_fun_increaseAllowance,
                          [
                          ]
                        |), ]
        ))
                    |),
                    M.case (|
                      Literal.number 0x70a08231,
                      ltac:(M.monadic (
                        [M.call (|
                          external_fun_balanceOf,
                          [
                          ]
                        |), ]
        ))
                    |),
                    M.case (|
                      Literal.number 0xa457c2d7,
                      ltac:(M.monadic (
                        [M.call (|
                          external_fun_decreaseAllowance,
                          [
                          ]
                        |), ]
        ))
                    |),
                    M.case (|
                      Literal.number 0xa9059cbb,
                      ltac:(M.monadic (
                        [M.call (|
                          external_fun_transfer,
                          [
                          ]
                        |), ]
        ))
                    |),
                    M.case (|
                      Literal.number 0xdd62ed3e,
                      ltac:(M.monadic (
                        [M.call (|
                          external_fun_allowance,
                          [
                          ]
                        |), ]
        ))
                    |),
                    M.case (|
                      default,
                      ltac:(M.monadic (
                        []
        ))
                    |),
                  ]
                |), ]
        ))
            |), M.call (|
              revert_error_42b3090547df1d2001c96683413b8cf91c1b902ef5e3cb8d9f6f304cf7446f74,
              [
              ]
            |), ]
        )), M.function (|
            shift_right_unsigned,
            [
              value,
            ],
            [
              newValue,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [newValue],
                  M.call (|
                    shr,
                    [
                    ]
                  |)
                |) in, ]
        ))
          |), M.function (|
            allocate_unbounded,
            [
            ],
            [
              memPtr,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [memPtr],
                  M.call (|
                    mload,
                    [
                    ]
                  |)
                |) in, ]
        ))
          |), M.function (|
            revert_error_ca66f745a3ce8ff40e2ccaf1ad45db7774001b90d25810abd9040049be7bf4bb,
            [
            ],
            [
            ],
            ltac:(M.monadic (
              [M.call (|
                revert,
                [
                ]
              |), ]
        ))
          |), M.function (|
            revert_error_dbdddcbe895c83990c08b3492a0e83918d802a52331272ac6fdb6a7c4aea3b1b,
            [
            ],
            [
            ],
            ltac:(M.monadic (
              [M.call (|
                revert,
                [
                ]
              |), ]
        ))
          |), M.function (|
            cleanup_uint160,
            [
              value,
            ],
            [
              cleaned,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [cleaned],
                  M.call (|
                    and,
                    [
                    ]
                  |)
                |) in, ]
        ))
          |), M.function (|
            cleanup_address,
            [
              value,
            ],
            [
              cleaned,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [cleaned],
                  M.call (|
                    cleanup_uint160,
                    [
                    ]
                  |)
                |) in, ]
        ))
          |), M.function (|
            validator_revert_address,
            [
              value,
            ],
            [
            ],
            ltac:(M.monadic (
              [M.if (|
                M.call (|
                  iszero,
                  [
                  ]
                |),
                ltac:(M.monadic (
                  [M.call (|
                    revert,
                    [
                    ]
                  |), ]
        ))
              |), ]
        ))
          |), M.function (|
            abi_decode_address,
            [
              offset,
              end,
            ],
            [
              value,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [value],
                  M.call (|
                    calldataload,
                    [
                    ]
                  |)
                |) in, M.call (|
                validator_revert_address,
                [
                ]
              |), ]
        ))
          |), M.function (|
            cleanup_uint256,
            [
              value,
            ],
            [
              cleaned,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [cleaned],
                  value
                |) in, ]
        ))
          |), M.function (|
            validator_revert_uint256,
            [
              value,
            ],
            [
            ],
            ltac:(M.monadic (
              [M.if (|
                M.call (|
                  iszero,
                  [
                  ]
                |),
                ltac:(M.monadic (
                  [M.call (|
                    revert,
                    [
                    ]
                  |), ]
        ))
              |), ]
        ))
          |), M.function (|
            abi_decode_uint256,
            [
              offset,
              end,
            ],
            [
              value,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [value],
                  M.call (|
                    calldataload,
                    [
                    ]
                  |)
                |) in, M.call (|
                validator_revert_uint256,
                [
                ]
              |), ]
        ))
          |), M.function (|
            abi_decode_addresst_uint256,
            [
              headStart,
              dataEnd,
            ],
            [
              value0,
              value1,
            ],
            ltac:(M.monadic (
              [M.if (|
                M.call (|
                  slt,
                  [
                  ]
                |),
                ltac:(M.monadic (
                  [M.call (|
                    revert_error_dbdddcbe895c83990c08b3492a0e83918d802a52331272ac6fdb6a7c4aea3b1b,
                    [
                    ]
                  |), ]
        ))
              |), let _ :=
                M.assign (|
                  [offset],
                  Literal.number 0
                |) in, let _ :=
                M.declare (|
                  [value0],
                  M.call (|
                    abi_decode_address,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [offset_1],
                  Literal.number 32
                |) in, let _ :=
                M.declare (|
                  [value1],
                  M.call (|
                    abi_decode_uint256,
                    [
                    ]
                  |)
                |) in, ]
        ))
          |), M.function (|
            cleanup_bool,
            [
              value,
            ],
            [
              cleaned,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [cleaned],
                  M.call (|
                    iszero,
                    [
                    ]
                  |)
                |) in, ]
        ))
          |), M.function (|
            abi_encode_bool_to_bool,
            [
              value,
              pos,
            ],
            [
            ],
            ltac:(M.monadic (
              [M.call (|
                mstore,
                [
                ]
              |), ]
        ))
          |), M.function (|
            abi_encode_bool,
            [
              headStart,
              value0,
            ],
            [
              tail,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [tail],
                  M.call (|
                    add,
                    [
                    ]
                  |)
                |) in, M.call (|
                abi_encode_bool_to_bool,
                [
                ]
              |), ]
        ))
          |), M.function (|
            external_fun_approve,
            [
            ],
            [
            ],
            ltac:(M.monadic (
              [M.if (|
                M.call (|
                  callvalue,
                  [
                  ]
                |),
                ltac:(M.monadic (
                  [M.call (|
                    revert_error_ca66f745a3ce8ff40e2ccaf1ad45db7774001b90d25810abd9040049be7bf4bb,
                    [
                    ]
                  |), ]
        ))
              |), let _ :=
                M.assign (|
                  [paramparam_1],
                  M.call (|
                    abi_decode_addresst_uint256,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [ret],
                  M.call (|
                    fun_approve,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [memPos],
                  M.call (|
                    allocate_unbounded,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [memEnd],
                  M.call (|
                    abi_encode_bool,
                    [
                    ]
                  |)
                |) in, M.call (|
                return,
                [
                ]
              |), ]
        ))
          |), M.function (|
            abi_decode,
            [
              headStart,
              dataEnd,
            ],
            [
            ],
            ltac:(M.monadic (
              [M.if (|
                M.call (|
                  slt,
                  [
                  ]
                |),
                ltac:(M.monadic (
                  [M.call (|
                    revert_error_dbdddcbe895c83990c08b3492a0e83918d802a52331272ac6fdb6a7c4aea3b1b,
                    [
                    ]
                  |), ]
        ))
              |), ]
        ))
          |), M.function (|
            abi_encode_uint256_to_uint256,
            [
              value,
              pos,
            ],
            [
            ],
            ltac:(M.monadic (
              [M.call (|
                mstore,
                [
                ]
              |), ]
        ))
          |), M.function (|
            abi_encode_uint256,
            [
              headStart,
              value0,
            ],
            [
              tail,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [tail],
                  M.call (|
                    add,
                    [
                    ]
                  |)
                |) in, M.call (|
                abi_encode_uint256_to_uint256,
                [
                ]
              |), ]
        ))
          |), M.function (|
            external_fun_totalSupply,
            [
            ],
            [
            ],
            ltac:(M.monadic (
              [M.if (|
                M.call (|
                  callvalue,
                  [
                  ]
                |),
                ltac:(M.monadic (
                  [M.call (|
                    revert_error_ca66f745a3ce8ff40e2ccaf1ad45db7774001b90d25810abd9040049be7bf4bb,
                    [
                    ]
                  |), ]
        ))
              |), M.call (|
                abi_decode,
                [
                ]
              |), let _ :=
                M.assign (|
                  [ret],
                  M.call (|
                    fun_totalSupply,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [memPos],
                  M.call (|
                    allocate_unbounded,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [memEnd],
                  M.call (|
                    abi_encode_uint256,
                    [
                    ]
                  |)
                |) in, M.call (|
                return,
                [
                ]
              |), ]
        ))
          |), M.function (|
            abi_decode_addresst_addresst_uint256,
            [
              headStart,
              dataEnd,
            ],
            [
              value0,
              value1,
              value2,
            ],
            ltac:(M.monadic (
              [M.if (|
                M.call (|
                  slt,
                  [
                  ]
                |),
                ltac:(M.monadic (
                  [M.call (|
                    revert_error_dbdddcbe895c83990c08b3492a0e83918d802a52331272ac6fdb6a7c4aea3b1b,
                    [
                    ]
                  |), ]
        ))
              |), let _ :=
                M.assign (|
                  [offset],
                  Literal.number 0
                |) in, let _ :=
                M.declare (|
                  [value0],
                  M.call (|
                    abi_decode_address,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [offset_1],
                  Literal.number 32
                |) in, let _ :=
                M.declare (|
                  [value1],
                  M.call (|
                    abi_decode_address,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [offset_2],
                  Literal.number 64
                |) in, let _ :=
                M.declare (|
                  [value2],
                  M.call (|
                    abi_decode_uint256,
                    [
                    ]
                  |)
                |) in, ]
        ))
          |), M.function (|
            external_fun_transferFrom,
            [
            ],
            [
            ],
            ltac:(M.monadic (
              [M.if (|
                M.call (|
                  callvalue,
                  [
                  ]
                |),
                ltac:(M.monadic (
                  [M.call (|
                    revert_error_ca66f745a3ce8ff40e2ccaf1ad45db7774001b90d25810abd9040049be7bf4bb,
                    [
                    ]
                  |), ]
        ))
              |), let _ :=
                M.assign (|
                  [paramparam_1param_2],
                  M.call (|
                    abi_decode_addresst_addresst_uint256,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [ret],
                  M.call (|
                    fun_transferFrom,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [memPos],
                  M.call (|
                    allocate_unbounded,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [memEnd],
                  M.call (|
                    abi_encode_bool,
                    [
                    ]
                  |)
                |) in, M.call (|
                return,
                [
                ]
              |), ]
        ))
          |), M.function (|
            external_fun_increaseAllowance,
            [
            ],
            [
            ],
            ltac:(M.monadic (
              [M.if (|
                M.call (|
                  callvalue,
                  [
                  ]
                |),
                ltac:(M.monadic (
                  [M.call (|
                    revert_error_ca66f745a3ce8ff40e2ccaf1ad45db7774001b90d25810abd9040049be7bf4bb,
                    [
                    ]
                  |), ]
        ))
              |), let _ :=
                M.assign (|
                  [paramparam_1],
                  M.call (|
                    abi_decode_addresst_uint256,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [ret],
                  M.call (|
                    fun_increaseAllowance,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [memPos],
                  M.call (|
                    allocate_unbounded,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [memEnd],
                  M.call (|
                    abi_encode_bool,
                    [
                    ]
                  |)
                |) in, M.call (|
                return,
                [
                ]
              |), ]
        ))
          |), M.function (|
            abi_decode_tuple_address,
            [
              headStart,
              dataEnd,
            ],
            [
              value0,
            ],
            ltac:(M.monadic (
              [M.if (|
                M.call (|
                  slt,
                  [
                  ]
                |),
                ltac:(M.monadic (
                  [M.call (|
                    revert_error_dbdddcbe895c83990c08b3492a0e83918d802a52331272ac6fdb6a7c4aea3b1b,
                    [
                    ]
                  |), ]
        ))
              |), let _ :=
                M.assign (|
                  [offset],
                  Literal.number 0
                |) in, let _ :=
                M.declare (|
                  [value0],
                  M.call (|
                    abi_decode_address,
                    [
                    ]
                  |)
                |) in, ]
        ))
          |), M.function (|
            external_fun_balanceOf,
            [
            ],
            [
            ],
            ltac:(M.monadic (
              [M.if (|
                M.call (|
                  callvalue,
                  [
                  ]
                |),
                ltac:(M.monadic (
                  [M.call (|
                    revert_error_ca66f745a3ce8ff40e2ccaf1ad45db7774001b90d25810abd9040049be7bf4bb,
                    [
                    ]
                  |), ]
        ))
              |), let _ :=
                M.assign (|
                  [param],
                  M.call (|
                    abi_decode_tuple_address,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [ret],
                  M.call (|
                    fun_balanceOf,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [memPos],
                  M.call (|
                    allocate_unbounded,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [memEnd],
                  M.call (|
                    abi_encode_uint256,
                    [
                    ]
                  |)
                |) in, M.call (|
                return,
                [
                ]
              |), ]
        ))
          |), M.function (|
            external_fun_decreaseAllowance,
            [
            ],
            [
            ],
            ltac:(M.monadic (
              [M.if (|
                M.call (|
                  callvalue,
                  [
                  ]
                |),
                ltac:(M.monadic (
                  [M.call (|
                    revert_error_ca66f745a3ce8ff40e2ccaf1ad45db7774001b90d25810abd9040049be7bf4bb,
                    [
                    ]
                  |), ]
        ))
              |), let _ :=
                M.assign (|
                  [paramparam_1],
                  M.call (|
                    abi_decode_addresst_uint256,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [ret],
                  M.call (|
                    fun_decreaseAllowance,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [memPos],
                  M.call (|
                    allocate_unbounded,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [memEnd],
                  M.call (|
                    abi_encode_bool,
                    [
                    ]
                  |)
                |) in, M.call (|
                return,
                [
                ]
              |), ]
        ))
          |), M.function (|
            external_fun_transfer,
            [
            ],
            [
            ],
            ltac:(M.monadic (
              [M.if (|
                M.call (|
                  callvalue,
                  [
                  ]
                |),
                ltac:(M.monadic (
                  [M.call (|
                    revert_error_ca66f745a3ce8ff40e2ccaf1ad45db7774001b90d25810abd9040049be7bf4bb,
                    [
                    ]
                  |), ]
        ))
              |), let _ :=
                M.assign (|
                  [paramparam_1],
                  M.call (|
                    abi_decode_addresst_uint256,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [ret],
                  M.call (|
                    fun_transfer_94,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [memPos],
                  M.call (|
                    allocate_unbounded,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [memEnd],
                  M.call (|
                    abi_encode_bool,
                    [
                    ]
                  |)
                |) in, M.call (|
                return,
                [
                ]
              |), ]
        ))
          |), M.function (|
            abi_decode_addresst_address,
            [
              headStart,
              dataEnd,
            ],
            [
              value0,
              value1,
            ],
            ltac:(M.monadic (
              [M.if (|
                M.call (|
                  slt,
                  [
                  ]
                |),
                ltac:(M.monadic (
                  [M.call (|
                    revert_error_dbdddcbe895c83990c08b3492a0e83918d802a52331272ac6fdb6a7c4aea3b1b,
                    [
                    ]
                  |), ]
        ))
              |), let _ :=
                M.assign (|
                  [offset],
                  Literal.number 0
                |) in, let _ :=
                M.declare (|
                  [value0],
                  M.call (|
                    abi_decode_address,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [offset_1],
                  Literal.number 32
                |) in, let _ :=
                M.declare (|
                  [value1],
                  M.call (|
                    abi_decode_address,
                    [
                    ]
                  |)
                |) in, ]
        ))
          |), M.function (|
            external_fun_allowance,
            [
            ],
            [
            ],
            ltac:(M.monadic (
              [M.if (|
                M.call (|
                  callvalue,
                  [
                  ]
                |),
                ltac:(M.monadic (
                  [M.call (|
                    revert_error_ca66f745a3ce8ff40e2ccaf1ad45db7774001b90d25810abd9040049be7bf4bb,
                    [
                    ]
                  |), ]
        ))
              |), let _ :=
                M.assign (|
                  [paramparam_1],
                  M.call (|
                    abi_decode_addresst_address,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [ret],
                  M.call (|
                    fun_allowance,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [memPos],
                  M.call (|
                    allocate_unbounded,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [memEnd],
                  M.call (|
                    abi_encode_uint256,
                    [
                    ]
                  |)
                |) in, M.call (|
                return,
                [
                ]
              |), ]
        ))
          |), M.function (|
            revert_error_42b3090547df1d2001c96683413b8cf91c1b902ef5e3cb8d9f6f304cf7446f74,
            [
            ],
            [
            ],
            ltac:(M.monadic (
              [M.call (|
                revert,
                [
                ]
              |), ]
        ))
          |), M.function (|
            zero_value_for_split_bool,
            [
            ],
            [
              ret,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [ret],
                  Literal.number 0
                |) in, ]
        ))
          |), M.function (|
            fun_approve,
            [
              var_spender,
              var_value,
            ],
            [
              var,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.assign (|
                  [zero_bool],
                  M.call (|
                    zero_value_for_split_bool,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.declare (|
                  [var],
                  zero_bool
                |) in, let _ :=
                M.assign (|
                  [expr],
                  M.call (|
                    caller,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [_1],
                  var_spender
                |) in, let _ :=
                M.assign (|
                  [expr_1],
                  _1
                |) in, let _ :=
                M.assign (|
                  [_2],
                  var_value
                |) in, let _ :=
                M.assign (|
                  [expr_2],
                  _2
                |) in, M.call (|
                fun__approve,
                [
                ]
              |), let _ :=
                M.assign (|
                  [expr_3],
                  Literal.number 0x01
                |) in, let _ :=
                M.declare (|
                  [var],
                  expr_3
                |) in, M.leave, ]
        ))
          |), M.function (|
            zero_value_for_split_uint256,
            [
            ],
            [
              ret,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [ret],
                  Literal.number 0
                |) in, ]
        ))
          |), M.function (|
            shift_right_0_unsigned,
            [
              value,
            ],
            [
              newValue,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [newValue],
                  M.call (|
                    shr,
                    [
                    ]
                  |)
                |) in, ]
        ))
          |), M.function (|
            cleanup_from_storage_uint256,
            [
              value,
            ],
            [
              cleaned,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [cleaned],
                  value
                |) in, ]
        ))
          |), M.function (|
            extract_from_storage_value_offsett_uint256,
            [
              slot_value,
            ],
            [
              value,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [value],
                  M.call (|
                    cleanup_from_storage_uint256,
                    [
                    ]
                  |)
                |) in, ]
        ))
          |), M.function (|
            read_from_storage_split_offset_uint256,
            [
              slot,
            ],
            [
              value,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [value],
                  M.call (|
                    extract_from_storage_value_offsett_uint256,
                    [
                    ]
                  |)
                |) in, ]
        ))
          |), M.function (|
            fun_totalSupply,
            [
            ],
            [
              var,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.assign (|
                  [zero_uint256],
                  M.call (|
                    zero_value_for_split_uint256,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.declare (|
                  [var],
                  zero_uint256
                |) in, let _ :=
                M.assign (|
                  [_1],
                  M.call (|
                    read_from_storage_split_offset_uint256,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [expr],
                  _1
                |) in, let _ :=
                M.declare (|
                  [var],
                  expr
                |) in, M.leave, ]
        ))
          |), M.function (|
            identity,
            [
              value,
            ],
            [
              ret,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [ret],
                  value
                |) in, ]
        ))
          |), M.function (|
            convert_uint160_to_uint160,
            [
              value,
            ],
            [
              converted,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [converted],
                  M.call (|
                    cleanup_uint160,
                    [
                    ]
                  |)
                |) in, ]
        ))
          |), M.function (|
            convert_uint160_to_address,
            [
              value,
            ],
            [
              converted,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [converted],
                  M.call (|
                    convert_uint160_to_uint160,
                    [
                    ]
                  |)
                |) in, ]
        ))
          |), M.function (|
            convert_address_to_address,
            [
              value,
            ],
            [
              converted,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [converted],
                  M.call (|
                    convert_uint160_to_address,
                    [
                    ]
                  |)
                |) in, ]
        ))
          |), M.function (|
            mapping_index_access_mapping_address_mapping_address_uint256_of_address,
            [
              slot,
              key,
            ],
            [
              dataSlot,
            ],
            ltac:(M.monadic (
              [M.call (|
                mstore,
                [
                ]
              |), M.call (|
                mstore,
                [
                ]
              |), let _ :=
                M.declare (|
                  [dataSlot],
                  M.call (|
                    keccak256,
                    [
                    ]
                  |)
                |) in, ]
        ))
          |), M.function (|
            mapping_index_access_mapping_address_uint256_of_address,
            [
              slot,
              key,
            ],
            [
              dataSlot,
            ],
            ltac:(M.monadic (
              [M.call (|
                mstore,
                [
                ]
              |), M.call (|
                mstore,
                [
                ]
              |), let _ :=
                M.declare (|
                  [dataSlot],
                  M.call (|
                    keccak256,
                    [
                    ]
                  |)
                |) in, ]
        ))
          |), M.function (|
            panic_error_0x11,
            [
            ],
            [
            ],
            ltac:(M.monadic (
              [M.call (|
                mstore,
                [
                ]
              |), M.call (|
                mstore,
                [
                ]
              |), M.call (|
                revert,
                [
                ]
              |), ]
        ))
          |), M.function (|
            checked_sub_uint256,
            [
              x,
              y,
            ],
            [
              diff,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [x],
                  M.call (|
                    cleanup_uint256,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.declare (|
                  [y],
                  M.call (|
                    cleanup_uint256,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.declare (|
                  [diff],
                  M.call (|
                    sub,
                    [
                    ]
                  |)
                |) in, M.if (|
                M.call (|
                  gt,
                  [
                  ]
                |),
                ltac:(M.monadic (
                  [M.call (|
                    panic_error_0x11,
                    [
                    ]
                  |), ]
        ))
              |), ]
        ))
          |), M.function (|
            fun_transferFrom,
            [
              var_from,
              var_to,
              var_value,
            ],
            [
              var,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.assign (|
                  [zero_bool],
                  M.call (|
                    zero_value_for_split_bool,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.declare (|
                  [var],
                  zero_bool
                |) in, let _ :=
                M.assign (|
                  [_1],
                  var_from
                |) in, let _ :=
                M.assign (|
                  [expr],
                  _1
                |) in, let _ :=
                M.assign (|
                  [_2],
                  var_to
                |) in, let _ :=
                M.assign (|
                  [expr_1],
                  _2
                |) in, let _ :=
                M.assign (|
                  [_3],
                  var_value
                |) in, let _ :=
                M.assign (|
                  [expr_2],
                  _3
                |) in, M.call (|
                fun_transfer,
                [
                ]
              |), let _ :=
                M.assign (|
                  [_4],
                  var_from
                |) in, let _ :=
                M.assign (|
                  [expr_3],
                  _4
                |) in, let _ :=
                M.assign (|
                  [expr_4],
                  M.call (|
                    caller,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [_11_slot],
                  Literal.number 0x01
                |) in, let _ :=
                M.assign (|
                  [expr_134_slot],
                  _11_slot
                |) in, let _ :=
                M.assign (|
                  [_5],
                  var_from
                |) in, let _ :=
                M.assign (|
                  [expr_5],
                  _5
                |) in, let _ :=
                M.assign (|
                  [_6],
                  M.call (|
                    mapping_index_access_mapping_address_mapping_address_uint256_of_address,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [_slot],
                  _6
                |) in, let _ :=
                M.assign (|
                  [expr_136_slot],
                  _slot
                |) in, let _ :=
                M.assign (|
                  [expr_6],
                  M.call (|
                    caller,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [_7],
                  M.call (|
                    mapping_index_access_mapping_address_uint256_of_address,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [_8],
                  M.call (|
                    read_from_storage_split_offset_uint256,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [expr_7],
                  _8
                |) in, let _ :=
                M.assign (|
                  [_9],
                  var_value
                |) in, let _ :=
                M.assign (|
                  [expr_8],
                  _9
                |) in, let _ :=
                M.assign (|
                  [expr_9],
                  M.call (|
                    checked_sub_uint256,
                    [
                    ]
                  |)
                |) in, M.call (|
                fun__approve,
                [
                ]
              |), let _ :=
                M.assign (|
                  [expr_10],
                  Literal.number 0x01
                |) in, let _ :=
                M.declare (|
                  [var],
                  expr_10
                |) in, M.leave, ]
        ))
          |), M.function (|
            checked_add_uint256,
            [
              x,
              y,
            ],
            [
              sum,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [x],
                  M.call (|
                    cleanup_uint256,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.declare (|
                  [y],
                  M.call (|
                    cleanup_uint256,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.declare (|
                  [sum],
                  M.call (|
                    add,
                    [
                    ]
                  |)
                |) in, M.if (|
                M.call (|
                  gt,
                  [
                  ]
                |),
                ltac:(M.monadic (
                  [M.call (|
                    panic_error_0x11,
                    [
                    ]
                  |), ]
        ))
              |), ]
        ))
          |), M.function (|
            fun_increaseAllowance,
            [
              var_spender,
              var_addedValue,
            ],
            [
              var,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.assign (|
                  [zero_t_bool],
                  M.call (|
                    zero_value_for_split_bool,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.declare (|
                  [var],
                  zero_t_bool
                |) in, let _ :=
                M.assign (|
                  [expr],
                  M.call (|
                    caller,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [_1],
                  var_spender
                |) in, let _ :=
                M.assign (|
                  [expr_1],
                  _1
                |) in, let _ :=
                M.assign (|
                  [_20_slot],
                  Literal.number 0x01
                |) in, let _ :=
                M.assign (|
                  [expr_160_slot],
                  _20_slot
                |) in, let _ :=
                M.assign (|
                  [expr_2],
                  M.call (|
                    caller,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [_2],
                  M.call (|
                    mapping_index_access_mapping_address_mapping_address_uint256_of_address,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [_22_slot],
                  _2
                |) in, let _ :=
                M.assign (|
                  [expr_163_slot],
                  _22_slot
                |) in, let _ :=
                M.assign (|
                  [_3],
                  var_spender
                |) in, let _ :=
                M.assign (|
                  [expr_3],
                  _3
                |) in, let _ :=
                M.assign (|
                  [_4],
                  M.call (|
                    mapping_index_access_mapping_address_uint256_of_address,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [_5],
                  M.call (|
                    read_from_storage_split_offset_uint256,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [expr_4],
                  _5
                |) in, let _ :=
                M.assign (|
                  [_6],
                  var_addedValue
                |) in, let _ :=
                M.assign (|
                  [expr_5],
                  _6
                |) in, let _ :=
                M.assign (|
                  [expr_6],
                  M.call (|
                    checked_add_uint256,
                    [
                    ]
                  |)
                |) in, M.call (|
                fun__approve,
                [
                ]
              |), let _ :=
                M.assign (|
                  [expr_7],
                  Literal.number 0x01
                |) in, let _ :=
                M.declare (|
                  [var],
                  expr_7
                |) in, M.leave, ]
        ))
          |), M.function (|
            fun_balanceOf,
            [
              var_owner,
            ],
            [
              var,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.assign (|
                  [zero_t_uint256],
                  M.call (|
                    zero_value_for_split_uint256,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.declare (|
                  [var],
                  zero_t_uint256
                |) in, let _ :=
                M.assign (|
                  [_28_slot],
                  Literal.number 0x00
                |) in, let _ :=
                M.assign (|
                  [expr_54_slot],
                  _28_slot
                |) in, let _ :=
                M.assign (|
                  [_1],
                  var_owner
                |) in, let _ :=
                M.assign (|
                  [expr],
                  _1
                |) in, let _ :=
                M.assign (|
                  [_2],
                  M.call (|
                    mapping_index_access_mapping_address_uint256_of_address,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [_3],
                  M.call (|
                    read_from_storage_split_offset_uint256,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [expr_1],
                  _3
                |) in, let _ :=
                M.declare (|
                  [var],
                  expr_1
                |) in, M.leave, ]
        ))
          |), M.function (|
            fun_decreaseAllowance,
            [
              var_spender,
              var_subtractedValue,
            ],
            [
              var,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.assign (|
                  [zero_bool],
                  M.call (|
                    zero_value_for_split_bool,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.declare (|
                  [var],
                  zero_bool
                |) in, let _ :=
                M.assign (|
                  [expr],
                  M.call (|
                    caller,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [_1],
                  var_spender
                |) in, let _ :=
                M.assign (|
                  [expr_1],
                  _1
                |) in, let _ :=
                M.assign (|
                  [_34_slot],
                  Literal.number 0x01
                |) in, let _ :=
                M.assign (|
                  [expr_slot],
                  _34_slot
                |) in, let _ :=
                M.assign (|
                  [expr_2],
                  M.call (|
                    caller,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [_2],
                  M.call (|
                    mapping_index_access_mapping_address_mapping_address_uint256_of_address,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [_36_slot],
                  _2
                |) in, let _ :=
                M.assign (|
                  [expr_189_slot],
                  _36_slot
                |) in, let _ :=
                M.assign (|
                  [_3],
                  var_spender
                |) in, let _ :=
                M.assign (|
                  [expr_3],
                  _3
                |) in, let _ :=
                M.assign (|
                  [_4],
                  M.call (|
                    mapping_index_access_mapping_address_uint256_of_address,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [_5],
                  M.call (|
                    read_from_storage_split_offset_uint256,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [expr_4],
                  _5
                |) in, let _ :=
                M.assign (|
                  [_6],
                  var_subtractedValue
                |) in, let _ :=
                M.assign (|
                  [expr_5],
                  _6
                |) in, let _ :=
                M.assign (|
                  [expr_6],
                  M.call (|
                    checked_sub_uint256,
                    [
                    ]
                  |)
                |) in, M.call (|
                fun__approve,
                [
                ]
              |), let _ :=
                M.assign (|
                  [expr_7],
                  Literal.number 0x01
                |) in, let _ :=
                M.declare (|
                  [var],
                  expr_7
                |) in, M.leave, ]
        ))
          |), M.function (|
            fun_transfer_94,
            [
              var_to,
              var_value,
            ],
            [
              var,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.assign (|
                  [zero_bool],
                  M.call (|
                    zero_value_for_split_bool,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.declare (|
                  [var],
                  zero_bool
                |) in, let _ :=
                M.assign (|
                  [expr],
                  M.call (|
                    caller,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [_1],
                  var_to
                |) in, let _ :=
                M.assign (|
                  [expr_1],
                  _1
                |) in, let _ :=
                M.assign (|
                  [_2],
                  var_value
                |) in, let _ :=
                M.assign (|
                  [expr_2],
                  _2
                |) in, M.call (|
                fun_transfer,
                [
                ]
              |), let _ :=
                M.assign (|
                  [expr_3],
                  Literal.number 0x01
                |) in, let _ :=
                M.declare (|
                  [var],
                  expr_3
                |) in, M.leave, ]
        ))
          |), M.function (|
            fun_allowance,
            [
              var_owner,
              var_spender,
            ],
            [
              var_,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.assign (|
                  [zero_uint256],
                  M.call (|
                    zero_value_for_split_uint256,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.declare (|
                  [var_],
                  zero_uint256
                |) in, let _ :=
                M.assign (|
                  [_45_slot],
                  Literal.number 0x01
                |) in, let _ :=
                M.assign (|
                  [expr_68_slot],
                  _45_slot
                |) in, let _ :=
                M.assign (|
                  [_1],
                  var_owner
                |) in, let _ :=
                M.assign (|
                  [expr],
                  _1
                |) in, let _ :=
                M.assign (|
                  [_2],
                  M.call (|
                    mapping_index_access_mapping_address_mapping_address_uint256_of_address,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [_48_slot],
                  _2
                |) in, let _ :=
                M.assign (|
                  [expr_70_slot],
                  _48_slot
                |) in, let _ :=
                M.assign (|
                  [_3],
                  var_spender
                |) in, let _ :=
                M.assign (|
                  [expr_1],
                  _3
                |) in, let _ :=
                M.assign (|
                  [_4],
                  M.call (|
                    mapping_index_access_mapping_address_uint256_of_address,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [_5],
                  M.call (|
                    read_from_storage_split_offset_uint256,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [expr_2],
                  _5
                |) in, let _ :=
                M.declare (|
                  [var_],
                  expr_2
                |) in, M.leave, ]
        ))
          |), M.function (|
            cleanup_rational_by,
            [
              value,
            ],
            [
              cleaned,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [cleaned],
                  value
                |) in, ]
        ))
          |), M.function (|
            convert_rational_by_to_uint160,
            [
              value,
            ],
            [
              converted,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [converted],
                  M.call (|
                    cleanup_uint160,
                    [
                    ]
                  |)
                |) in, ]
        ))
          |), M.function (|
            convert_rational_by_to_address,
            [
              value,
            ],
            [
              converted,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [converted],
                  M.call (|
                    convert_rational_by_to_uint160,
                    [
                    ]
                  |)
                |) in, ]
        ))
          |), M.function (|
            array_storeLengthForEncoding_string,
            [
              pos,
              length,
            ],
            [
              updated_pos,
            ],
            ltac:(M.monadic (
              [M.call (|
                mstore,
                [
                ]
              |), let _ :=
                M.declare (|
                  [updated_pos],
                  M.call (|
                    add,
                    [
                    ]
                  |)
                |) in, ]
        ))
          |), M.function (|
            store_literal_in_memory_c953f4879035ed60e766b34720f656aab5c697b141d924c283124ecedb91c208,
            [
              memPtr,
            ],
            [
            ],
            ltac:(M.monadic (
              [M.call (|
                mstore,
                [
                ]
              |), M.call (|
                mstore,
                [
                ]
              |), ]
        ))
          |), M.function (|
            abi_encode_stringliteral_c953f4879035ed60e766b34720f656aab5c697b141d924c283124ecedb91c208,
            [
              pos,
            ],
            [
              end,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [pos],
                  M.call (|
                    array_storeLengthForEncoding_string,
                    [
                    ]
                  |)
                |) in, M.call (|
                store_literal_in_memory_c953f4879035ed60e766b34720f656aab5c697b141d924c283124ecedb91c208,
                [
                ]
              |), let _ :=
                M.declare (|
                  [end],
                  M.call (|
                    add,
                    [
                    ]
                  |)
                |) in, ]
        ))
          |), M.function (|
            abi_encode_stringliteral_c953,
            [
              headStart,
            ],
            [
              tail,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [tail],
                  M.call (|
                    add,
                    [
                    ]
                  |)
                |) in, M.call (|
                mstore,
                [
                ]
              |), let _ :=
                M.declare (|
                  [tail],
                  M.call (|
                    abi_encode_stringliteral_c953f4879035ed60e766b34720f656aab5c697b141d924c283124ecedb91c208,
                    [
                    ]
                  |)
                |) in, ]
        ))
          |), M.function (|
            require_helper_stringliteral_c953,
            [
              condition,
            ],
            [
            ],
            ltac:(M.monadic (
              [M.if (|
                M.call (|
                  iszero,
                  [
                  ]
                |),
                ltac:(M.monadic (
                  [let _ :=
                    M.assign (|
                      [memPtr],
                      M.call (|
                        allocate_unbounded,
                        [
                        ]
                      |)
                    |) in, M.call (|
                    mstore,
                    [
                    ]
                  |), let _ :=
                    M.assign (|
                      [end],
                      M.call (|
                        abi_encode_stringliteral_c953,
                        [
                        ]
                      |)
                    |) in, M.call (|
                    revert,
                    [
                    ]
                  |), ]
        ))
              |), ]
        ))
          |), M.function (|
            store_literal_in_memory_24883cc5fe64ace9d0df1893501ecb93c77180f0ff69cca79affb3c316dc8029,
            [
              memPtr,
            ],
            [
            ],
            ltac:(M.monadic (
              [M.call (|
                mstore,
                [
                ]
              |), M.call (|
                mstore,
                [
                ]
              |), ]
        ))
          |), M.function (|
            abi_encode_stringliteral_24883cc5fe64ace9d0df1893501ecb93c77180f0ff69cca79affb3c316dc8029,
            [
              pos,
            ],
            [
              end,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [pos],
                  M.call (|
                    array_storeLengthForEncoding_string,
                    [
                    ]
                  |)
                |) in, M.call (|
                store_literal_in_memory_24883cc5fe64ace9d0df1893501ecb93c77180f0ff69cca79affb3c316dc8029,
                [
                ]
              |), let _ :=
                M.declare (|
                  [end],
                  M.call (|
                    add,
                    [
                    ]
                  |)
                |) in, ]
        ))
          |), M.function (|
            abi_encode_stringliteral,
            [
              headStart,
            ],
            [
              tail,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [tail],
                  M.call (|
                    add,
                    [
                    ]
                  |)
                |) in, M.call (|
                mstore,
                [
                ]
              |), let _ :=
                M.declare (|
                  [tail],
                  M.call (|
                    abi_encode_stringliteral_24883cc5fe64ace9d0df1893501ecb93c77180f0ff69cca79affb3c316dc8029,
                    [
                    ]
                  |)
                |) in, ]
        ))
          |), M.function (|
            require_helper_stringliteral_2488,
            [
              condition,
            ],
            [
            ],
            ltac:(M.monadic (
              [M.if (|
                M.call (|
                  iszero,
                  [
                  ]
                |),
                ltac:(M.monadic (
                  [let _ :=
                    M.assign (|
                      [memPtr],
                      M.call (|
                        allocate_unbounded,
                        [
                        ]
                      |)
                    |) in, M.call (|
                    mstore,
                    [
                    ]
                  |), let _ :=
                    M.assign (|
                      [end],
                      M.call (|
                        abi_encode_stringliteral,
                        [
                        ]
                      |)
                    |) in, M.call (|
                    revert,
                    [
                    ]
                  |), ]
        ))
              |), ]
        ))
          |), M.function (|
            shift_left,
            [
              value,
            ],
            [
              newValue,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [newValue],
                  M.call (|
                    shl,
                    [
                    ]
                  |)
                |) in, ]
        ))
          |), M.function (|
            update_byte_slice_shift,
            [
              value,
              toInsert,
            ],
            [
              result,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.assign (|
                  [mask],
                  M.call (|
                    not,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.declare (|
                  [toInsert],
                  M.call (|
                    shift_left,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.declare (|
                  [value],
                  M.call (|
                    and,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.declare (|
                  [result],
                  M.call (|
                    or,
                    [
                    ]
                  |)
                |) in, ]
        ))
          |), M.function (|
            convert_uint256_to_uint256,
            [
              value,
            ],
            [
              converted,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [converted],
                  M.call (|
                    cleanup_uint256,
                    [
                    ]
                  |)
                |) in, ]
        ))
          |), M.function (|
            prepare_store_uint256,
            [
              value,
            ],
            [
              ret,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [ret],
                  value
                |) in, ]
        ))
          |), M.function (|
            update_storage_value_offsett_uint256_to_uint256,
            [
              slot,
              value,
            ],
            [
            ],
            ltac:(M.monadic (
              [let _ :=
                M.assign (|
                  [convertedValue],
                  M.call (|
                    convert_uint256_to_uint256,
                    [
                    ]
                  |)
                |) in, M.call (|
                sstore,
                [
                ]
              |), ]
        ))
          |), M.function (|
            fun__approve,
            [
              var_owner,
              var_spender,
              var_value,
            ],
            [
            ],
            ltac:(M.monadic (
              [let _ :=
                M.assign (|
                  [_1],
                  var_owner
                |) in, let _ :=
                M.assign (|
                  [expr],
                  _1
                |) in, let _ :=
                M.assign (|
                  [expr_1],
                  Literal.number 0x00
                |) in, let _ :=
                M.assign (|
                  [expr_2],
                  M.call (|
                    convert_rational_by_to_address,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [expr_3],
                  M.call (|
                    iszero,
                    [
                    ]
                  |)
                |) in, M.call (|
                require_helper_stringliteral_c953,
                [
                ]
              |), let _ :=
                M.assign (|
                  [_2],
                  var_spender
                |) in, let _ :=
                M.assign (|
                  [expr_4],
                  _2
                |) in, let _ :=
                M.assign (|
                  [expr_5],
                  Literal.number 0x00
                |) in, let _ :=
                M.assign (|
                  [expr_6],
                  M.call (|
                    convert_rational_by_to_address,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [expr_7],
                  M.call (|
                    iszero,
                    [
                    ]
                  |)
                |) in, M.call (|
                require_helper_stringliteral_2488,
                [
                ]
              |), let _ :=
                M.assign (|
                  [_3],
                  var_value
                |) in, let _ :=
                M.assign (|
                  [expr_8],
                  _3
                |) in, let _ :=
                M.assign (|
                  [_55_slot],
                  Literal.number 0x01
                |) in, let _ :=
                M.assign (|
                  [expr_360_slot],
                  _55_slot
                |) in, let _ :=
                M.assign (|
                  [_4],
                  var_owner
                |) in, let _ :=
                M.assign (|
                  [expr_9],
                  _4
                |) in, let _ :=
                M.assign (|
                  [_5],
                  M.call (|
                    mapping_index_access_mapping_address_mapping_address_uint256_of_address,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [_58_slot],
                  _5
                |) in, let _ :=
                M.assign (|
                  [expr_363_slot],
                  _58_slot
                |) in, let _ :=
                M.assign (|
                  [_6],
                  var_spender
                |) in, let _ :=
                M.assign (|
                  [expr_10],
                  _6
                |) in, let _ :=
                M.assign (|
                  [_7],
                  M.call (|
                    mapping_index_access_mapping_address_uint256_of_address,
                    [
                    ]
                  |)
                |) in, M.call (|
                update_storage_value_offsett_uint256_to_uint256,
                [
                ]
              |), let _ :=
                M.assign (|
                  [_8],
                  var_owner
                |) in, let _ :=
                M.assign (|
                  [expr_11],
                  _8
                |) in, let _ :=
                M.assign (|
                  [_9],
                  var_spender
                |) in, let _ :=
                M.assign (|
                  [expr_12],
                  _9
                |) in, let _ :=
                M.assign (|
                  [_10],
                  var_value
                |) in, let _ :=
                M.assign (|
                  [expr_13],
                  _10
                |) in, let _ :=
                M.assign (|
                  [_11],
                  Literal.number 0x8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b925
                |) in, let _ :=
                M.assign (|
                  [_12],
                  M.call (|
                    convert_address_to_address,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [_13],
                  M.call (|
                    convert_address_to_address,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [_14],
                  M.call (|
                    allocate_unbounded,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [_15],
                  M.call (|
                    abi_encode_uint256,
                    [
                    ]
                  |)
                |) in, M.call (|
                log3,
                [
                ]
              |), ]
        ))
          |), M.function (|
            store_literal_in_memory_0557e210f7a69a685100a7e4e3d0a7024c546085cee28910fd17d0b081d9516f,
            [
              memPtr,
            ],
            [
            ],
            ltac:(M.monadic (
              [M.call (|
                mstore,
                [
                ]
              |), M.call (|
                mstore,
                [
                ]
              |), ]
        ))
          |), M.function (|
            abi_encode_stringliteral_0557,
            [
              pos,
            ],
            [
              end,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [pos],
                  M.call (|
                    array_storeLengthForEncoding_string,
                    [
                    ]
                  |)
                |) in, M.call (|
                store_literal_in_memory_0557e210f7a69a685100a7e4e3d0a7024c546085cee28910fd17d0b081d9516f,
                [
                ]
              |), let _ :=
                M.declare (|
                  [end],
                  M.call (|
                    add,
                    [
                    ]
                  |)
                |) in, ]
        ))
          |), M.function (|
            abi_encode_tuple_stringliteral,
            [
              headStart,
            ],
            [
              tail,
            ],
            ltac:(M.monadic (
              [let _ :=
                M.declare (|
                  [tail],
                  M.call (|
                    add,
                    [
                    ]
                  |)
                |) in, M.call (|
                mstore,
                [
                ]
              |), let _ :=
                M.declare (|
                  [tail],
                  M.call (|
                    abi_encode_stringliteral_0557,
                    [
                    ]
                  |)
                |) in, ]
        ))
          |), M.function (|
            require_helper_stringliteral,
            [
              condition,
            ],
            [
            ],
            ltac:(M.monadic (
              [M.if (|
                M.call (|
                  iszero,
                  [
                  ]
                |),
                ltac:(M.monadic (
                  [let _ :=
                    M.assign (|
                      [memPtr],
                      M.call (|
                        allocate_unbounded,
                        [
                        ]
                      |)
                    |) in, M.call (|
                    mstore,
                    [
                    ]
                  |), let _ :=
                    M.assign (|
                      [end],
                      M.call (|
                        abi_encode_tuple_stringliteral,
                        [
                        ]
                      |)
                    |) in, M.call (|
                    revert,
                    [
                    ]
                  |), ]
        ))
              |), ]
        ))
          |), M.function (|
            fun_transfer,
            [
              var_from,
              var_to,
              var_value,
            ],
            [
            ],
            ltac:(M.monadic (
              [let _ :=
                M.assign (|
                  [_1],
                  var_to
                |) in, let _ :=
                M.assign (|
                  [expr],
                  _1
                |) in, let _ :=
                M.assign (|
                  [expr_1],
                  Literal.number 0x00
                |) in, let _ :=
                M.assign (|
                  [expr_2],
                  M.call (|
                    convert_rational_by_to_address,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [expr_3],
                  M.call (|
                    iszero,
                    [
                    ]
                  |)
                |) in, M.call (|
                require_helper_stringliteral,
                [
                ]
              |), let _ :=
                M.assign (|
                  [_70_slot],
                  Literal.number 0x00
                |) in, let _ :=
                M.assign (|
                  [expr_221_slot],
                  _70_slot
                |) in, let _ :=
                M.assign (|
                  [_2],
                  var_from
                |) in, let _ :=
                M.assign (|
                  [expr_4],
                  _2
                |) in, let _ :=
                M.assign (|
                  [_3],
                  M.call (|
                    mapping_index_access_mapping_address_uint256_of_address,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [_4],
                  M.call (|
                    read_from_storage_split_offset_uint256,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [expr_5],
                  _4
                |) in, let _ :=
                M.assign (|
                  [_5],
                  var_value
                |) in, let _ :=
                M.assign (|
                  [expr_6],
                  _5
                |) in, let _ :=
                M.assign (|
                  [expr_7],
                  M.call (|
                    checked_sub_uint256,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [_75_slot],
                  Literal.number 0x00
                |) in, let _ :=
                M.assign (|
                  [expr_218_slot],
                  _75_slot
                |) in, let _ :=
                M.assign (|
                  [_6],
                  var_from
                |) in, let _ :=
                M.assign (|
                  [expr_8],
                  _6
                |) in, let _ :=
                M.assign (|
                  [_7],
                  M.call (|
                    mapping_index_access_mapping_address_uint256_of_address,
                    [
                    ]
                  |)
                |) in, M.call (|
                update_storage_value_offsett_uint256_to_uint256,
                [
                ]
              |), let _ :=
                M.assign (|
                  [_78_slot],
                  Literal.number 0x00
                |) in, let _ :=
                M.assign (|
                  [expr_231_slot],
                  _78_slot
                |) in, let _ :=
                M.assign (|
                  [_8],
                  var_to
                |) in, let _ :=
                M.assign (|
                  [expr_9],
                  _8
                |) in, let _ :=
                M.assign (|
                  [_9],
                  M.call (|
                    mapping_index_access_mapping_address_uint256_of_address,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [_10],
                  M.call (|
                    read_from_storage_split_offset_uint256,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [expr_10],
                  _10
                |) in, let _ :=
                M.assign (|
                  [_11],
                  var_value
                |) in, let _ :=
                M.assign (|
                  [expr_11],
                  _11
                |) in, let _ :=
                M.assign (|
                  [expr_12],
                  M.call (|
                    checked_add_uint256,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [_83_slot],
                  Literal.number 0x00
                |) in, let _ :=
                M.assign (|
                  [expr_228_slot],
                  _83_slot
                |) in, let _ :=
                M.assign (|
                  [_12],
                  var_to
                |) in, let _ :=
                M.assign (|
                  [expr_13],
                  _12
                |) in, let _ :=
                M.assign (|
                  [_13],
                  M.call (|
                    mapping_index_access_mapping_address_uint256_of_address,
                    [
                    ]
                  |)
                |) in, M.call (|
                update_storage_value_offsett_uint256_to_uint256,
                [
                ]
              |), let _ :=
                M.assign (|
                  [_14],
                  var_from
                |) in, let _ :=
                M.assign (|
                  [expr_14],
                  _14
                |) in, let _ :=
                M.assign (|
                  [_15],
                  var_to
                |) in, let _ :=
                M.assign (|
                  [expr_15],
                  _15
                |) in, let _ :=
                M.assign (|
                  [_16],
                  var_value
                |) in, let _ :=
                M.assign (|
                  [expr_16],
                  _16
                |) in, let _ :=
                M.assign (|
                  [_17],
                  Literal.number 0xddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef
                |) in, let _ :=
                M.assign (|
                  [_18],
                  M.call (|
                    convert_address_to_address,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [_19],
                  M.call (|
                    convert_address_to_address,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [_20],
                  M.call (|
                    allocate_unbounded,
                    [
                    ]
                  |)
                |) in, let _ :=
                M.assign (|
                  [_21],
                  M.call (|
                    abi_encode_uint256,
                    [
                    ]
                  |)
                |) in, M.call (|
                log3,
                [
                ]
              |), ]
        ))
          |), ]
        ))
        data: a2646970667358221220e6dc5aa403489ac0f98c340cb1499801c6d3bd818b41486a061fdadb03d3f4b464736f6c634300081b0033
    }
}