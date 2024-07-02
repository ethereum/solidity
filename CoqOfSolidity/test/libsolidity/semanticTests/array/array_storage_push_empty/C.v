(* Generated by coq-of-solidity *)
Require Import CoqOfSolidity.CoqOfSolidity.

Module C.
  Definition code : Code.t := {|
    Code.name := "C_39";
    Code.hex_name := 0x435f333900000000000000000000000000000000000000000000000000000000;
    Code.functions :=
      [
        
      ];
    Code.body :=
      M.scope (
        do! ltac:(M.monadic (
          M.scope (
            do! ltac:(M.monadic (
              M.declare (|
                ["_1"],
                Some (M.call_function (|
                  "memoryguard",
                  [
                    [Literal.number 0x80]
                  ]
                |))
              |)
            )) in
            do! ltac:(M.monadic (
              M.expr_stmt (|
                M.call_function (|
                  "mstore",
                  [
                    [Literal.number 64];
                    M.get_var (| "_1" |)
                  ]
                |)
              |)
            )) in
            do! ltac:(M.monadic (
              M.if_ (|
                M.call_function (|
                  "callvalue",
                  []
                |),
                M.scope (
                  do! ltac:(M.monadic (
                    M.expr_stmt (|
                      M.call_function (|
                        "revert",
                        [
                          [Literal.number 0];
                          [Literal.number 0]
                        ]
                      |)
                    |)
                  )) in
                  M.pure BlockUnit.Tt
                )
              |)
            )) in
            do! ltac:(M.monadic (
              M.declare (|
                ["_2"],
                Some (M.call_function (|
                  "datasize",
                  [
                    [Literal.string 0x435f33395f6465706c6f79656400000000000000000000000000000000000000]
                  ]
                |))
              |)
            )) in
            do! ltac:(M.monadic (
              M.expr_stmt (|
                M.call_function (|
                  "codecopy",
                  [
                    M.get_var (| "_1" |);
                    M.call_function (|
                      "dataoffset",
                      [
                        [Literal.string 0x435f33395f6465706c6f79656400000000000000000000000000000000000000]
                      ]
                    |);
                    M.get_var (| "_2" |)
                  ]
                |)
              |)
            )) in
            do! ltac:(M.monadic (
              M.expr_stmt (|
                M.call_function (|
                  "return",
                  [
                    M.get_var (| "_1" |);
                    M.get_var (| "_2" |)
                  ]
                |)
              |)
            )) in
            M.pure BlockUnit.Tt
          )
        )) in
        M.pure BlockUnit.Tt
      );
  |}.

  Module deployed.
    Definition code : Code.t := {|
      Code.name := "C_39_deployed";
      Code.hex_name := 0x435f33395f6465706c6f79656400000000000000000000000000000000000000;
      Code.functions :=
        [
          Code.Function.make (
            "storage_array_index_access_uint256_dyn_ptr",
            ["index"],
            ["slot"; "offset"],
            M.scope (
              do! ltac:(M.monadic (
                M.if_ (|
                  M.call_function (|
                    "iszero",
                    [
                      M.call_function (|
                        "lt",
                        [
                          M.get_var (| "index" |);
                          M.call_function (|
                            "sload",
                            [
                              [Literal.number 0]
                            ]
                          |)
                        ]
                      |)
                    ]
                  |),
                  M.scope (
                    do! ltac:(M.monadic (
                      M.expr_stmt (|
                        M.call_function (|
                          "mstore",
                          [
                            [Literal.number 0];
                            M.call_function (|
                              "shl",
                              [
                                [Literal.number 224];
                                [Literal.number 0x4e487b71]
                              ]
                            |)
                          ]
                        |)
                      |)
                    )) in
                    do! ltac:(M.monadic (
                      M.expr_stmt (|
                        M.call_function (|
                          "mstore",
                          [
                            [Literal.number 4];
                            [Literal.number 0x32]
                          ]
                        |)
                      |)
                    )) in
                    do! ltac:(M.monadic (
                      M.expr_stmt (|
                        M.call_function (|
                          "revert",
                          [
                            [Literal.number 0];
                            [Literal.number 0x24]
                          ]
                        |)
                      |)
                    )) in
                    M.pure BlockUnit.Tt
                  )
                |)
              )) in
              do! ltac:(M.monadic (
                M.expr_stmt (|
                  M.call_function (|
                    "mstore",
                    [
                      [Literal.number 0];
                      [Literal.number 0]
                    ]
                  |)
                |)
              )) in
              do! ltac:(M.monadic (
                M.assign (|
                  ["slot"],
                  Some (M.call_function (|
                    "add",
                    [
                      M.call_function (|
                        "keccak256",
                        [
                          [Literal.number 0];
                          [Literal.number 0x20]
                        ]
                      |);
                      M.get_var (| "index" |)
                    ]
                  |))
                |)
              )) in
              do! ltac:(M.monadic (
                M.assign (|
                  ["offset"],
                  Some ([Literal.number 0])
                |)
              )) in
              M.pure BlockUnit.Tt
            )
          )
        ];
      Code.body :=
        M.scope (
          do! ltac:(M.monadic (
            M.scope (
              do! ltac:(M.monadic (
                M.expr_stmt (|
                  M.call_function (|
                    "mstore",
                    [
                      [Literal.number 64];
                      M.call_function (|
                        "memoryguard",
                        [
                          [Literal.number 0x80]
                        ]
                      |)
                    ]
                  |)
                |)
              )) in
              do! ltac:(M.monadic (
                M.if_ (|
                  M.call_function (|
                    "iszero",
                    [
                      M.call_function (|
                        "lt",
                        [
                          M.call_function (|
                            "calldatasize",
                            []
                          |);
                          [Literal.number 4]
                        ]
                      |)
                    ]
                  |),
                  M.scope (
                    do! ltac:(M.monadic (
                      M.if_ (|
                        M.call_function (|
                          "eq",
                          [
                            [Literal.number 0xe931873f];
                            M.call_function (|
                              "shr",
                              [
                                [Literal.number 224];
                                M.call_function (|
                                  "calldataload",
                                  [
                                    [Literal.number 0]
                                  ]
                                |)
                              ]
                            |)
                          ]
                        |),
                        M.scope (
                          do! ltac:(M.monadic (
                            M.if_ (|
                              M.call_function (|
                                "callvalue",
                                []
                              |),
                              M.scope (
                                do! ltac:(M.monadic (
                                  M.expr_stmt (|
                                    M.call_function (|
                                      "revert",
                                      [
                                        [Literal.number 0];
                                        [Literal.number 0]
                                      ]
                                    |)
                                  |)
                                )) in
                                M.pure BlockUnit.Tt
                              )
                            |)
                          )) in
                          do! ltac:(M.monadic (
                            M.if_ (|
                              M.call_function (|
                                "slt",
                                [
                                  M.call_function (|
                                    "add",
                                    [
                                      M.call_function (|
                                        "calldatasize",
                                        []
                                      |);
                                      M.call_function (|
                                        "not",
                                        [
                                          [Literal.number 3]
                                        ]
                                      |)
                                    ]
                                  |);
                                  [Literal.number 32]
                                ]
                              |),
                              M.scope (
                                do! ltac:(M.monadic (
                                  M.expr_stmt (|
                                    M.call_function (|
                                      "revert",
                                      [
                                        [Literal.number 0];
                                        [Literal.number 0]
                                      ]
                                    |)
                                  |)
                                )) in
                                M.pure BlockUnit.Tt
                              )
                            |)
                          )) in
                          do! ltac:(M.monadic (
                            M.declare (|
                              ["value"],
                              Some (M.call_function (|
                                "calldataload",
                                [
                                  [Literal.number 4]
                                ]
                              |))
                            |)
                          )) in
                          do! ltac:(M.monadic (
                            do!
                              M.scope (
                                M.pure BlockUnit.Tt
                              ) in
                            ltac:(M.monadic (
                              M.for_ (|
                                ltac:(M.monadic (
                                  [Literal.number 1]
                                )),
                                M.scope (
                                  M.pure BlockUnit.Tt
                                ),
                                M.scope (
                                  do! ltac:(M.monadic (
                                    M.declare (|
                                      ["expr"],
                                      Some (M.call_function (|
                                        "sload",
                                        [
                                          [Literal.number 0]
                                        ]
                                      |))
                                    |)
                                  )) in
                                  do! ltac:(M.monadic (
                                    M.if_ (|
                                      M.call_function (|
                                        "iszero",
                                        [
                                          M.call_function (|
                                            "lt",
                                            [
                                              M.get_var (| "expr" |);
                                              M.get_var (| "value" |)
                                            ]
                                          |)
                                        ]
                                      |),
                                      M.scope (
                                        do! ltac:(M.monadic (
                                          M.break (||)
                                        )) in
                                        M.pure BlockUnit.Tt
                                      )
                                    |)
                                  )) in
                                  do! ltac:(M.monadic (
                                    M.if_ (|
                                      M.call_function (|
                                        "iszero",
                                        [
                                          M.call_function (|
                                            "lt",
                                            [
                                              M.get_var (| "expr" |);
                                              [Literal.number 18446744073709551616]
                                            ]
                                          |)
                                        ]
                                      |),
                                      M.scope (
                                        do! ltac:(M.monadic (
                                          M.expr_stmt (|
                                            M.call_function (|
                                              "mstore",
                                              [
                                                [Literal.number 0];
                                                M.call_function (|
                                                  "shl",
                                                  [
                                                    [Literal.number 224];
                                                    [Literal.number 0x4e487b71]
                                                  ]
                                                |)
                                              ]
                                            |)
                                          |)
                                        )) in
                                        do! ltac:(M.monadic (
                                          M.expr_stmt (|
                                            M.call_function (|
                                              "mstore",
                                              [
                                                [Literal.number 4];
                                                [Literal.number 0x41]
                                              ]
                                            |)
                                          |)
                                        )) in
                                        do! ltac:(M.monadic (
                                          M.expr_stmt (|
                                            M.call_function (|
                                              "revert",
                                              [
                                                [Literal.number 0];
                                                [Literal.number 0x24]
                                              ]
                                            |)
                                          |)
                                        )) in
                                        M.pure BlockUnit.Tt
                                      )
                                    |)
                                  )) in
                                  do! ltac:(M.monadic (
                                    M.expr_stmt (|
                                      M.call_function (|
                                        "sstore",
                                        [
                                          [Literal.number 0];
                                          M.call_function (|
                                            "add",
                                            [
                                              M.get_var (| "expr" |);
                                              [Literal.number 1]
                                            ]
                                          |)
                                        ]
                                      |)
                                    |)
                                  )) in
                                  do! ltac:(M.monadic (
                                    M.declare (|
                                      ["slot"; "offset"],
                                      Some (M.call_function (|
                                        "storage_array_index_access_uint256_dyn_ptr",
                                        [
                                          M.get_var (| "expr" |)
                                        ]
                                      |))
                                    |)
                                  )) in
                                  M.pure BlockUnit.Tt
                                )
                              |)
                            ))
                          )) in
                          do! ltac:(M.monadic (
                            M.declare (|
                              ["var_i"],
                              Some ([Literal.number 0])
                            |)
                          )) in
                          do! ltac:(M.monadic (
                            do!
                              M.scope (
                                M.pure BlockUnit.Tt
                              ) in
                            ltac:(M.monadic (
                              M.for_ (|
                                ltac:(M.monadic (
                                  M.call_function (|
                                    "lt",
                                    [
                                      M.get_var (| "var_i" |);
                                      M.get_var (| "value" |)
                                    ]
                                  |)
                                )),
                                M.scope (
                                  do! ltac:(M.monadic (
                                    M.assign (|
                                      ["var_i"],
                                      Some (M.call_function (|
                                        "add",
                                        [
                                          M.get_var (| "var_i" |);
                                          [Literal.number 1]
                                        ]
                                      |))
                                    |)
                                  )) in
                                  M.pure BlockUnit.Tt
                                ),
                                M.scope (
                                  do! ltac:(M.monadic (
                                    M.declare (|
                                      ["_1"; "_2"],
                                      Some (M.call_function (|
                                        "storage_array_index_access_uint256_dyn_ptr",
                                        [
                                          M.get_var (| "var_i" |)
                                        ]
                                      |))
                                    |)
                                  )) in
                                  do! ltac:(M.monadic (
                                    M.if_ (|
                                      M.call_function (|
                                        "iszero",
                                        [
                                          M.call_function (|
                                            "iszero",
                                            [
                                              M.call_function (|
                                                "shr",
                                                [
                                                  M.call_function (|
                                                    "shl",
                                                    [
                                                      [Literal.number 3];
                                                      M.get_var (| "_2" |)
                                                    ]
                                                  |);
                                                  M.call_function (|
                                                    "sload",
                                                    [
                                                      M.get_var (| "_1" |)
                                                    ]
                                                  |)
                                                ]
                                              |)
                                            ]
                                          |)
                                        ]
                                      |),
                                      M.scope (
                                        do! ltac:(M.monadic (
                                          M.expr_stmt (|
                                            M.call_function (|
                                              "revert",
                                              [
                                                [Literal.number 0];
                                                [Literal.number 0]
                                              ]
                                            |)
                                          |)
                                        )) in
                                        M.pure BlockUnit.Tt
                                      )
                                    |)
                                  )) in
                                  M.pure BlockUnit.Tt
                                )
                              |)
                            ))
                          )) in
                          do! ltac:(M.monadic (
                            M.expr_stmt (|
                              M.call_function (|
                                "return",
                                [
                                  [Literal.number 0];
                                  [Literal.number 0]
                                ]
                              |)
                            |)
                          )) in
                          M.pure BlockUnit.Tt
                        )
                      |)
                    )) in
                    M.pure BlockUnit.Tt
                  )
                |)
              )) in
              do! ltac:(M.monadic (
                M.expr_stmt (|
                  M.call_function (|
                    "revert",
                    [
                      [Literal.number 0];
                      [Literal.number 0]
                    ]
                  |)
                |)
              )) in
              M.pure BlockUnit.Tt
            )
          )) in
          M.pure BlockUnit.Tt
        );
    |}.
  End deployed.
End C.

Import Ltac2.

Definition codes : list Code.t :=
  ltac2:(
    let codes := Code.get_codes () in
    exact $codes
  ).
