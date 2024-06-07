Require Import CoqOfSolidity.CoqOfSolidity.
Require EVM.Crypto.Keccak.

(** We should probably use an existing library for the [Dict.t] definition. We have mainly two kinds
    of keys in our code:
    - [string] for variable/function names
    - [U256.t] for addresses coming from the code
*)
Module Dict.
  Definition t (K V : Set) : Set :=
    list (K * V).

  Fixpoint get {K V : Set} (eqb : K -> K -> bool) (dict : t K V) (key : K) : option V :=
    match dict with
    | [] => None
    | (current_key, current_value) :: dict =>
      if eqb key current_key then
        Some current_value
      else
        get eqb dict key
    end.

  Definition declare {K V : Set} (dict : t K V) (key : K) (value : V) : t K V :=
    (key, value) :: dict.

  Fixpoint assign_function {K V : Set}
      (eqb : K -> K -> bool) (dict : t K V) (key : K) (f : V -> V) :
      option (t K V) :=
    match dict with
    | [] => None
    | ((current_key, current_value) as entry) :: dict =>
      if eqb current_key key then
        Some ((current_key, f current_value) :: dict)
      else
        match assign_function eqb dict key f with
        | None => None
        | Some dict => Some (entry :: dict)
        end
    end.

  Definition assign {K V : Set} (eqb : K -> K -> bool) (dict : t K V) (key : K) (value : V) :
      option (t K V) :=
    assign_function eqb dict key (fun _ => value).
End Dict.

Module Locals.
  Record t : Set := {
    functions : list (string * (list U256.t -> M.t (list U256.t)));
    variables : list (string * U256.t)
  }.

  Definition empty : t := {|
    functions := [];
    variables := [];
  |}.
End Locals.

Module Stack.
  Definition t : Set :=
    list Locals.t.

  Definition open_scope (stack : t) : t :=
    Locals.empty :: stack.

  Definition close_scope (stack : t) : t + string :=
    match stack with
    | [] => inr "cannot close the last scope as there are none left"
    | _ :: stack => inl stack
    end.

  Fixpoint get_var (stack : t) (name : string) : U256.t + string :=
    match stack with
    | [] => inr ("variable '" ++ name ++ "' not found")%string
    | locals :: stack =>
      match Dict.get String.eqb locals.(Locals.variables) name with
      | None => get_var stack name
      | Some value => inl value
      end
    end.

  Definition declare_var (stack : t) (name : string) (value : U256.t) : t + string :=
    match stack with
    | [] => inr "no scope to declare the variable"
    | locals :: stack =>
      inl (
        locals <| Locals.variables := Dict.declare locals.(Locals.variables) name value |> ::
        stack
      )
    end.

  Fixpoint declare_vars_aux (stack : t) (names : list string) (values : list U256.t) :
      option (t + string) :=
    match names, values with
    | [], [] => Some (inl stack)
    | name :: names, value :: values =>
      match declare_var stack name value with
      | inl stack => declare_vars_aux stack names values
      | inr error => Some (inr error)
      end
    | _, _ => None
    end.

  Definition declare_vars (stack : t) (names : list string) (values : list U256.t) : t + string :=
    match declare_vars_aux stack names values with
    | Some result => result
    | None =>
      inr (
        "declare: names and values have different lengths for names: " ++
        String.concat ", " names
      )%string
    end.

  Fixpoint assign_var (stack : t) (name : string) (value : U256.t) : t + string :=
    match stack with
    | [] => inr ("variable '" ++ name ++ "' not found")%string
    | locals :: stack =>
      match Dict.assign String.eqb locals.(Locals.variables) name value with
      | None =>
        match assign_var stack name value with
        | inl stack => inl (locals :: stack)
        | inr error => inr error
        end
      | Some variables => inl (locals <| Locals.variables := variables |> :: stack)
      end
    end.

  Fixpoint assign_vars_aux (stack : t) (names : list string) (values : list U256.t) :
      option (t + string) :=
    match names, values with
    | [], [] => Some (inl stack)
    | name :: names, value :: values =>
      match assign_var stack name value with
      | inl stack => assign_vars_aux stack names values
      | inr error => Some (inr error)
      end
    | _, _ => None
    end.

  Definition assign_vars (stack : t) (names : list string) (values : list U256.t) : t + string :=
    match assign_vars_aux stack names values with
    | Some result => result
    | None =>
      inr (
        "assign: names and values have different lengths for names: " ++
        String.concat ", " names
      )%string
    end.

  Fixpoint get_function (stack : t) (name : string) : list U256.t -> M.t (list U256.t) :=
    match stack with
    | [] => fun _ => LowM.Impossible ("function '" ++ name ++ "' not found")
    | locals :: stack =>
      match Dict.get String.eqb locals.(Locals.functions) name with
      | None => get_function stack name
      | Some function => function
      end
    end.

  Definition declare_function
      (stack : t)
      (name : string)
      (body : list U256.t -> M.t (list U256.t)) :
      t + string :=
    match stack with
    | [] => inr "no scope to declare the function"
    | locals :: stack =>
      inl (
        locals <| Locals.functions := Dict.declare locals.(Locals.functions) name body |> ::
        stack
      )
    end.
End Stack.

Module Memory.
  (** We define the memory as a function instead of an explicit list as there can be holes in it. It
      goes from addresses in [U256.t] to bytes represented as [Z]. *)
  Definition t : Set :=
    U256.t -> Z.

  Definition init : t :=
    fun _ => 0.

  (** Get the bytes from some memory from a start adress and for a certain length. *)
  Definition get_bytes (memory : Memory.t) (start length : U256.t) : list Z :=
    List.map
      (fun (i : nat) =>
        let address : U256.t := start + Z.of_nat i in
        memory address
      )
      (List.seq 0 (Z.to_nat length)).

  Definition update_bytes (memory : Memory.t) (start : U256.t) (bytes : list Z) : Memory.t :=
    fun address =>
      let i : Z := address - start in
      if andb (0 <=? i) (i <? Z.of_nat (List.length bytes)) then
        List.nth_default 0 bytes (Z.to_nat i)
      else
        memory address.

  Definition u256_as_bytes (value : U256.t) : list Z :=
    List.map
      (fun (i : nat) => Z.shiftr value (8 * (31 - Z.of_nat i)) mod 256)
      (List.seq 0 32).

  Definition bytes_as_u256 (bytes : list Z) : U256.t :=
    List.fold_left
      (fun (acc : U256.t) (byte : Z) =>
        (acc * 256) + byte
      )
      bytes
      0.

  Definition bytes_as_bytes (bytes : list Z) : list Nibble.byte :=
    List.map
      (fun (byte : Z) => Nibble.byte_of_N (Z.to_N byte))
      bytes.

  Fixpoint hex_string_as_bytes (hex_string : string) : list Z :=
    match hex_string with
    | "" => []
    | String.String a "" => [] (* this case is unexpected *)
    | String.String a (String.String b rest) =>
      match HexString.ascii_to_digit a, HexString.ascii_to_digit b with
      | Some a, Some b =>
        let byte := 16 * Z.of_N a + Z.of_N b in
        byte :: hex_string_as_bytes rest
      | _, _ => [] (* this case is unexpected *)
      end
    end.
End Memory.

Module Storage.
  (** Each slot in the storage is a word. This is different from [Memory.t] where it is only
      bytes. *)
  Definition t : Set :=
    U256.t -> U256.t.

  Definition update (storage : Storage.t) (address value : U256.t) : Storage.t :=
    fun current_address =>
      if address =? current_address then
        value
      else
        storage current_address.
End Storage.

Module CallStack.
  (** The list of functions that were called with their corresponding parameters. This is for
      debugging purpose only, and does not exist in the semantics of Yul. *)
  Definition t : Set :=
    list (string * list (string * U256.t)).
End CallStack.

Module Account.
  Record t : Set := {
    balance : U256.t;
    nonce: Z;
    code : U256.t;
    (** When calling a constructor the parameters are concatenated to the code. We represent them
        here. *)
    codedata : list Z;
    storage : Storage.t;
    immutables : Dict.t U256.t U256.t;
  }.
End Account.

Module State.
  (** The state contains the various kinds of memory that we use in a smart contract. *)
  Record t : Set := {
    stack : Stack.t;
    memory : Memory.t;
    return_data : list Z;
    transient_storage : Storage.t;
    accounts : list (U256.t * Account.t);
    codes : list (U256.t * M.t BlockUnit.t);
    logs : list (list U256.t * list Z);
    (** This is only for debugging *)
    call_stack : CallStack.t;
  }.
End State.

Module Stdlib.
  Definition get_signed_value (value : U256.t) : Z :=
    ((value + (2 ^ 255)) mod (2 ^ 256)) - (2 ^ 255).

  Goal List.map get_signed_value [0; 10; (2 ^ 256) - 3] = [0; 10; -3].
  Proof. reflexivity. Qed.

  Definition stop : M.t unit :=
    LowM.Pure (Result.Return 0 0).

  Definition add (x y : U256.t) : U256.t :=
    (x + y) mod (2 ^ 256).

  Definition sub (x y : U256.t) : U256.t :=
    (x - y) mod (2 ^ 256).

  Definition mul (x y : U256.t) : U256.t :=
    (x * y) mod (2 ^ 256).

  Definition div (x y : U256.t) : U256.t :=
    if y =? 0 then 0 else x / y.

  Definition sdiv (x y : U256.t) : U256.t :=
    if y =? 0 then
      0
    else
      let x := get_signed_value x in
      let y := get_signed_value y in
      let result := x / y in
      result mod (2 ^ 256).

  Definition mod_ (x y : U256.t) : U256.t :=
    if y =? 0 then 0 else x mod y.

  Definition smod (a b : Z) : Z :=
    let a := get_signed_value a in
    let b := get_signed_value b in
    if b =? 0 then
      0
    else
      (Z.rem a b) mod (2 ^ 256).

  Goal List.map (fun '(x, y) => smod x y)
    [(10, 3); (10, 0); (10, 10); (2^256 -8, 2^256 -3); (7, 5); (7, 2^256 -5)] =
    [1; 0; 0; 2^256 -2; 2; 2].
  Proof. vm_compute. reflexivity. Qed.

  Definition exp (x y : U256.t) : U256.t :=
    x ^ y.

  Definition not (x : U256.t) : U256.t :=
    2^256 - x - 1.

  Definition lt (x y : U256.t) : U256.t :=
    if x <? y then 1 else 0.

  Definition gt (x y : U256.t) : U256.t :=
    if x >? y then 1 else 0.

  (* Signed version of [lt] *)
  Definition slt (x y : U256.t) : U256.t :=
    let x := (x + 2 ^ 255) mod (2 ^ 256) in
    let y := (y + 2 ^ 255) mod (2 ^ 256) in
    lt x y.

  Definition sgt (x y : U256.t) : U256.t :=
    let x := (x + 2 ^ 255) mod (2 ^ 256) in
    let y := (y + 2 ^ 255) mod (2 ^ 256) in
    gt x y.

  Definition eq (x y : U256.t) : U256.t :=
    if x =? y then 1 else 0.

  Definition iszero (x : U256.t) : U256.t :=
    if x =? 0 then 1 else 0.

  Definition and (x y : U256.t) : U256.t :=
    Z.land x y.

  Definition or (x y : U256.t) : U256.t :=
    Z.lor x y.

  Definition xor (x y : U256.t) : U256.t :=
    Z.lxor x y.

  Definition byte (n x : U256.t) : U256.t :=
    (x / (256 ^ (31 - n))) mod 256.

  Definition shl (x y : U256.t) : U256.t :=
    (y * (2 ^ x)) mod (2 ^ 256).

  Definition shr (x y : U256.t) : U256.t :=
    y / (2 ^ x).

  Definition sar (x y : U256.t) : M.t U256.t :=
    LowM.Impossible "sar".

  Definition addmod (x y m : U256.t) : U256.t :=
    (x + y) mod m.

  Definition mulmod (x y m : U256.t) : U256.t :=
    (x * y) mod m.

  Definition signextend (i x : Z) : Z :=
    if i >=? 31 then
      x
    else
      let size := 8 * (i + 1) in
      let byte := (x / 2 ^ (8 * i)) mod 256 in
      let sign_bit := byte / 128 in
      let extend_bit (bit size : Z) : Z :=
        if bit =? 1 then
          (2 ^ 256) - (2 ^ size)
        else
          0 in
      (x mod (2 ^ size)) + extend_bit sign_bit size.

  Definition keccak256 (p n : U256.t) : M.t U256.t :=
    let* bytes := LowM.Primitive (Primitive.MLoad p n) M.pure in
    let bytes := Memory.bytes_as_bytes bytes in
    let hash : list Nibble.byte := EVM.Crypto.Keccak.keccak_256 bytes in
    let hash : list Z := (List.map (fun byte => Z.of_N (Nibble.N_of_byte byte))) hash in
    M.pure (Memory.bytes_as_u256 hash).

  Definition pc : M.t U256.t :=
    LowM.Impossible "pc".

  Definition pop (x : U256.t) : M.t unit :=
    M.pure tt.

  Definition mload (address : U256.t) : M.t U256.t :=
    let* bytes := LowM.Primitive (Primitive.MLoad address 32) M.pure in
    M.pure (Memory.bytes_as_u256 bytes).

  Definition mstore (address value : U256.t) : M.t unit :=
    let bytes := Memory.u256_as_bytes value in
    LowM.Primitive (Primitive.MStore address bytes) M.pure.

  Definition mstore8 (address value : U256.t) : M.t unit :=
    let bytes := [value mod 256] in
    LowM.Primitive (Primitive.MStore address bytes) M.pure.

  Definition sload (address : U256.t) : M.t U256.t :=
    LowM.Primitive (Primitive.SLoad address) M.pure.

  Definition sstore (address value : U256.t) : M.t unit :=
    LowM.Primitive (Primitive.SStore address value) M.pure.

  Definition tload (address : U256.t) : M.t U256.t :=
    LowM.Primitive (Primitive.TLoad address) M.pure.

  Definition tstore (address value : U256.t) : M.t unit :=
    LowM.Primitive (Primitive.TStore address value) M.pure.

  Definition msize : M.t U256.t :=
    LowM.Impossible "msize".

  Definition gas : M.t U256.t :=
    M.pure 1000.

  Definition address : M.t U256.t :=
    let* environemnt := LowM.Primitive Primitive.GetEnvironment M.pure in
    M.pure environemnt.(Environment.address).

  Definition balance (a : U256.t) : M.t U256.t :=
    LowM.Impossible "balance".

  Definition selfbalance : M.t U256.t :=
    LowM.Impossible "selfbalance".

  Definition caller : M.t U256.t :=
    LowM.Primitive Primitive.GetEnvironment (fun env => M.pure env.(Environment.caller)).

  Definition callvalue : M.t U256.t :=
    LowM.Primitive Primitive.GetEnvironment (fun env => M.pure env.(Environment.callvalue)).

  Definition calldataload (p : U256.t) : M.t U256.t :=
    let* environment := LowM.Primitive Primitive.GetEnvironment M.pure in
    let calldata := environment.(Environment.calldata) in
    let bytes : list Z :=
      List.map
        (fun (i : nat) =>
          let address : nat := (Z.to_nat p + i)%nat in
          List.nth_default 0 calldata address
        )
        (List.seq 0 32) in
    M.pure (Memory.bytes_as_u256 bytes).

  Definition calldatasize : M.t U256.t :=
    let* environment := LowM.Primitive Primitive.GetEnvironment M.pure in
    M.pure (Z.of_nat (List.length environment.(Environment.calldata))).

  Definition calldatacopy (t f s : U256.t) : M.t unit :=
    let* environment := LowM.Primitive Primitive.GetEnvironment M.pure in
    let calldata := environment.(Environment.calldata) in
    let bytes : list Z :=
      List.map
        (fun (i : nat) =>
          let address : nat := (Z.to_nat f + i)%nat in
          List.nth_default 0 calldata address
        )
        (List.seq 0 (Z.to_nat s)) in
    LowM.Primitive (Primitive.MStore t bytes) M.pure.

  Definition codesize : M.t U256.t :=
    let* environment := LowM.Primitive Primitive.GetEnvironment M.pure in
    let address := environment.(Environment.address) in
    let* codedata := LowM.Primitive (Primitive.GetCodedata address) M.pure in
    M.pure (32 + Z.of_nat (List.length codedata)).

  (** There are two kinds of code copy that we handle: either to copy actual code, or
      to copy the constructor's parameters that are stored just after the code of the
      constructor. *)
  Definition codecopy (t f s : U256.t) : M.t unit :=
    (* code case *)
    if f mod (2^256) =? 0 then
      if s =? 32 then
        let name : U256.t := f / (2^256) in
        let bytes := Memory.u256_as_bytes name in
        LowM.Primitive (Primitive.MStore t bytes) M.pure
      else
        LowM.Impossible "codecopy: s must be 32 for the code case"
    (* codedata case *)
    else if f mod (2^256) =? 32 then
      let* environment := LowM.Primitive Primitive.GetEnvironment M.pure in
      let* codedata :=
        LowM.Primitive (Primitive.GetCodedata environment.(Environment.address)) M.pure in
      if s =? Z.of_nat (List.length codedata) then
        LowM.Primitive (Primitive.MStore t codedata) M.pure
      else
        LowM.Impossible "codecopy: s must be the length of the codedata for the codedata case"
    else
      LowM.Impossible "codecopy: f mod (2^256) must be 0 or 32".

  Definition extcodesize (a : U256.t) : M.t U256.t :=
    let* codedata := LowM.Primitive (Primitive.GetCodedata a) M.pure in
    M.pure (32 + Z.of_nat (List.length codedata)).

  Definition extcodecopy (a t f s : U256.t) : M.t unit :=
    LowM.Impossible "extcodecopy".

  Definition returndatasize : M.t U256.t :=
    let* return_data := LowM.Primitive Primitive.RLoad M.pure in
    M.pure (Z.of_nat (List.length return_data)).

  Definition returndatacopy (t f s : U256.t) : M.t unit :=
    let* return_data := LowM.Primitive Primitive.RLoad M.pure in
    let bytes : list Z :=
      List.map
        (fun (i : nat) =>
          let index : nat := (Z.to_nat f + i)%nat in
          List.nth_default 0 return_data index
        )
        (List.seq 0 (Z.to_nat s)) in
    LowM.Primitive (Primitive.MStore t bytes) M.pure.

  Definition mcopy (t f s : U256.t) : M.t unit :=
    let* bytes := LowM.Primitive (Primitive.MLoad f s) M.pure in
    LowM.Primitive (Primitive.MStore t bytes) M.pure.

  Definition extcodehash (a : U256.t) : M.t U256.t :=
    LowM.Impossible "extcodehash".

  Definition create (v p n : U256.t) : M.t U256.t :=
    (* TODO: have the exact calculation of the address with RLP *)
    let* created_address :=
      let* environment := LowM.Primitive Primitive.GetEnvironment M.pure in
      let address := environment.(Environment.address) in
      let* nonce := LowM.Primitive Primitive.GetNonce M.pure in
      let bytes : list Z :=
        Memory.u256_as_bytes address ++ Memory.u256_as_bytes nonce in
      let bytes : list Nibble.byte := Memory.bytes_as_bytes bytes in
      let hash : list Nibble.byte := EVM.Crypto.Keccak.keccak_256 bytes in
      let hash : list Z := (List.map (fun byte => Z.of_N (Nibble.N_of_byte byte))) hash in
      M.pure (Z.land ((2 ^ 160) - 1) (Memory.bytes_as_u256 hash)) in
    if n <? 32 then
      LowM.Impossible "create with code of size lesser than a word"
    else
      let* code := mload p in
      let* codedata := LowM.Primitive (Primitive.MLoad (p + 32) (n - 32)) M.pure in
      let* tt := LowM.Primitive (Primitive.CreateAccount created_address code codedata) M.pure in
      (* The input during the call is empty as it is in the [codedata]. *)
      let* call_contract_status := LowM.CallContract created_address v [] M.pure in
      (* Failure case *)
      if call_contract_status =? 0 then
        M.pure 0
      (* Success case *)
      else
        let* constructor_output := LowM.Primitive Primitive.RLoad M.pure in
        if negb (Z.of_nat (List.length constructor_output) =? 32) then
          LowM.Impossible "create: constructor_output must be a word"
        else
          let deployed_code := Memory.bytes_as_u256 constructor_output in
          LowM.Primitive (Primitive.UpdateCodeForDeploy created_address deployed_code) (fun _ =>
          M.pure created_address).

  Definition create2 (v p n s : U256.t) : M.t U256.t :=
    LowM.Impossible "create2".

  Definition call (g a v in_ insize out outsize : U256.t) : M.t U256.t :=
    let* input := LowM.Primitive (Primitive.MLoad in_ insize) M.pure in
    let* result := LowM.CallContract a v input M.pure in
    let* output := LowM.Primitive Primitive.RLoad M.pure in
    LowM.Primitive (Primitive.MStore out (List.firstn (Z.to_nat outsize) output)) (fun _ =>
    M.pure result).

  Definition callcode (g a v in_ insize out outsize : U256.t) : M.t U256.t :=
    LowM.Impossible "callcode".

  Definition delegatecall (g a in_ insize out outsize : U256.t) : M.t U256.t :=
    LowM.Impossible "delegatecall".

  (* TODO: have a flag such that the operations that change the state fail. *)
  Definition staticcall (g a in_ insize out outsize : U256.t) : M.t U256.t :=
    call g a 0 in_ insize out outsize.

  Definition return_ (p s : U256.t) : M.t unit :=
    LowM.Pure (Result.Return p s).

  Definition revert (p s : U256.t) : M.t unit :=
    LowM.Pure (Result.Revert p s).

  Definition selfdestruct (a : U256.t) : M.t unit :=
    LowM.Impossible "selfdestruct".

  Definition invalid : M.t unit :=
    LowM.Impossible "invalid".

  Definition log0 (p s : U256.t) : M.t unit :=
    let* payload := LowM.Primitive (Primitive.MLoad p s) M.pure in
    LowM.Primitive (Primitive.Log [] payload) M.pure.

  Definition log1 (p s t1 : U256.t) : M.t unit :=
    let* payload := LowM.Primitive (Primitive.MLoad p s) M.pure in
    LowM.Primitive (Primitive.Log [t1] payload) M.pure.

  Definition log2 (p s t1 t2 : U256.t) : M.t unit :=
    let* payload := LowM.Primitive (Primitive.MLoad p s) M.pure in
    LowM.Primitive (Primitive.Log [t1; t2] payload) M.pure.

  Definition log3 (p s t1 t2 t3 : U256.t) : M.t unit :=
    let* payload := LowM.Primitive (Primitive.MLoad p s) M.pure in
    LowM.Primitive (Primitive.Log [t1; t2; t3] payload) M.pure.

  Definition log4 (p s t1 t2 t3 t4 : U256.t) : M.t unit :=
    let* payload := LowM.Primitive (Primitive.MLoad p s) M.pure in
    LowM.Primitive (Primitive.Log [t1; t2; t3; t4] payload) M.pure.

  Definition chainid : M.t U256.t :=
    LowM.Impossible "chainid".

  Definition basefee : M.t U256.t :=
    LowM.Impossible "basefee".

  Definition blobbasefee : M.t U256.t :=
    LowM.Impossible "blobbasefee".

  Definition origin : M.t U256.t :=
    LowM.Impossible "origin".

  Definition gasprice : M.t U256.t :=
    LowM.Impossible "gasprice".

  Definition blockhash (b : U256.t) : M.t U256.t :=
    LowM.Impossible "blockhash".

  Definition blobhash (i : U256.t) : M.t U256.t :=
    LowM.Impossible "blobhash".

  Definition coinbase : M.t U256.t :=
    LowM.Impossible "coinbase".

  Definition timestamp : M.t U256.t :=
    LowM.Impossible "timestamp".

  Definition number : M.t U256.t :=
    LowM.Impossible "number".

  Definition difficulty : M.t U256.t :=
    LowM.Impossible "difficulty".

  Definition prevrandao : M.t U256.t :=
    LowM.Impossible "prevrandao".

  Definition gaslimit : M.t U256.t :=
    LowM.Impossible "gaslimit".

  Definition loadimmutable (name : U256.t) : M.t U256.t :=
    LowM.Primitive (Primitive.LoadImmutable name) M.pure.

  Definition setimmutable (_offset name value : U256.t) : M.t unit :=
    LowM.Primitive (Primitive.SetImmutable name value) M.pure.

  (** Additional functions for the object mode of Yul. *)
  Module Object.
    (** We assume that the optimizer does not use any additional memory. *)
    Definition memoryguard (size : U256.t) : M.t U256.t :=
      M.pure size.

    (** For the [dataoffset] function we use the [name] of the code. We shift this
        address by 256 bits in order to be able to complete it with the address constructor call's
        parameters that are concatenated to the constructor's code at startup.

        Having an address that is more than 256 bits long is not realistic but not an issue for us
        as we store the addresses in [Z].
    *)
    Definition dataoffset (name : U256.t) : M.t U256.t :=
      M.pure (name * (2 ^ 256)).

    (** We suppose that the size of the code is a word's size, and is one word that is the name. *)
    Definition datasize (name : U256.t) : M.t U256.t :=
      M.pure 32.

    Definition datacopy (t f s : U256.t) : M.t unit :=
      codecopy t f s.
  End Object.

  Notation "'fn' p '=>' body" :=
    (fun arguments =>
      match arguments with
      | p => body
      | _ => LowM.Impossible "wrong number of arguments"
      end)
    (at level 200, p pattern).

  Definition return_unit (body : M.t unit) : M.t (list U256.t) :=
    M.let_ body (fun _ => M.pure []).

  Definition return_u256 (body : M.t U256.t) : M.t (list U256.t) :=
    M.let_ body (fun result => M.pure [result]).

  Definition functions : list (string * (list U256.t -> M.t (list U256.t))) := [
    ("stop", fn [] => return_unit stop);
    ("add", fn [x; y] => return_u256 (M.pure (add x y)));
    ("sub", fn [x; y] => return_u256 (M.pure (sub x y)));
    ("mul", fn [x; y] => return_u256 (M.pure (mul x y)));
    ("div", fn [x; y] => return_u256 (M.pure (div x y)));
    ("sdiv", fn [x; y] => return_u256 (M.pure (sdiv x y)));
    ("mod", fn [x; y] => return_u256 (M.pure (mod_ x y)));
    ("smod", fn [x; y] => return_u256 (M.pure (smod x y)));
    ("exp", fn [x; y] => return_u256 (M.pure (exp x y)));
    ("not", fn [x] => return_u256 (M.pure (not x)));
    ("lt", fn [x; y] => return_u256 (M.pure (lt x y)));
    ("gt", fn [x; y] => return_u256 (M.pure (gt x y)));
    ("slt", fn [x; y] => return_u256 (M.pure (slt x y)));
    ("sgt", fn [x; y] => return_u256 (M.pure (sgt x y)));
    ("eq", fn [x; y] => return_u256 (M.pure (eq x y)));
    ("iszero", fn [x] => return_u256 (M.pure (iszero x)));
    ("and", fn [x; y] => return_u256 (M.pure (and x y)));
    ("or", fn [x; y] => return_u256 (M.pure (or x y)));
    ("xor", fn [x; y] => return_u256 (M.pure (xor x y)));
    ("byte", fn [n; x] => return_u256 (M.pure (byte n x)));
    ("shl", fn [x; y] => return_u256 (M.pure (shl x y)));
    ("shr", fn [x; y] => return_u256 (M.pure (shr x y)));
    ("sar", fn [x; y] => return_u256 (sar x y));
    ("addmod", fn [x; y; m] => return_u256 (M.pure (addmod x y m)));
    ("mulmod", fn [x; y; m] => return_u256 (M.pure (mulmod x y m)));
    ("signextend", fn [i; x] => return_u256 (M.pure (signextend i x)));
    ("keccak256", fn [p; n] => return_u256 (keccak256 p n));
    ("pc", fn [] => return_u256 pc);
    ("pop", fn [x] => return_unit (pop x));
    ("mload", fn [p] => return_u256 (mload p));
    ("mstore", fn [p; v] => return_unit (mstore p v));
    ("mstore8", fn [p; v] => return_unit (mstore8 p v));
    ("sload", fn [p] => return_u256 (sload p));
    ("sstore", fn [p; v] => return_unit (sstore p v));
    ("tload", fn [p] => return_u256 (tload p));
    ("tstore", fn [p; v] => return_unit (tstore p v));
    ("msize", fn [] => return_u256 msize);
    ("gas", fn [] => return_u256 gas);
    ("address", fn [] => return_u256 address);
    ("balance", fn [a] => return_u256 (balance a));
    ("selfbalance", fn [] => return_u256 selfbalance);
    ("caller", fn [] => return_u256 caller);
    ("callvalue", fn [] => return_u256 callvalue);
    ("calldataload", fn [p] => return_u256 (calldataload p));
    ("calldatasize", fn [] => return_u256 calldatasize);
    ("calldatacopy", fn [p; s; n] => return_unit (calldatacopy p s n));
    ("codesize", fn [] => return_u256 codesize);
    ("codecopy", fn [p; s; n] => return_unit (codecopy p s n));
    ("extcodesize", fn [a] => return_u256 (extcodesize a));
    ("extcodecopy", fn [a; p; s; n] => return_unit (extcodecopy a p s n));
    ("returndatasize", fn [] => return_u256 returndatasize);
    ("returndatacopy", fn [p; s; n] => return_unit (returndatacopy p s n));
    ("mcopy", fn [t; f; s] => return_unit (mcopy t f s));
    ("extcodehash", fn [a] => return_u256 (extcodehash a));
    ("create", fn [v; p; n] => return_u256 (create v p n));
    ("create2", fn [v; p; n; s] => return_u256 (create2 v p n s));
    ("call", fn [g; a; v; in_; insize; out; outsize] =>
      return_u256 (call g a v in_ insize out outsize));
    ("callcode", fn [g; a; v; in_; insize; out; outsize] =>
      return_u256 (callcode g a v in_ insize out outsize));
    ("delegatecall", fn [g; a; in_; insize; out; outsize] =>
      return_u256 (delegatecall g a in_ insize out outsize));
    ("staticcall", fn [g; a; in_; insize; out; outsize] =>
      return_u256 (staticcall g a in_ insize out outsize));
    ("return", fn [p; s] => return_unit (return_ p s));
    ("revert", fn [p; s] => return_unit (revert p s));
    ("selfdestruct", fn [a] => return_unit (selfdestruct a));
    ("invalid", fn [] => return_unit invalid);
    ("log0", fn [p; s] => return_unit (log0 p s));
    ("log1", fn [p; s; t1] => return_unit (log1 p s t1));
    ("log2", fn [p; s; t1; t2] => return_unit (log2 p s t1 t2));
    ("log3", fn [p; s; t1; t2; t3] => return_unit (log3 p s t1 t2 t3));
    ("log4", fn [p; s; t1; t2; t3; t4] => return_unit (log4 p s t1 t2 t3 t4));
    ("chainid", fn [] => return_u256 chainid);
    ("basefee", fn [] => return_u256 basefee);
    ("blobbasefee", fn [] => return_u256 blobbasefee);
    ("origin", fn [] => return_u256 origin);
    ("gasprice", fn [] => return_u256 gasprice);
    ("blockhash", fn [b] => return_u256 (blockhash b));
    ("blobhash", fn [i] => return_u256 (blobhash i));
    ("coinbase", fn [] => return_u256 coinbase);
    ("timestamp", fn [] => return_u256 timestamp);
    ("number", fn [] => return_u256 number);
    ("difficulty", fn [] => return_u256 difficulty);
    ("prevrandao", fn [] => return_u256 prevrandao);
    ("gaslimit", fn [] => return_u256 gaslimit);
    ("loadimmutable", fn [name] => return_u256 (loadimmutable name));
    ("setimmutable", fn [offset; name; value] => return_unit (setimmutable offset name value));
    ("memoryguard", fn [p] => return_u256 (Object.memoryguard p));
    ("dataoffset", fn [name] => return_u256 (Object.dataoffset name));
    ("datasize", fn [name] => return_u256 (Object.datasize name));
    ("datacopy", fn [p; s; n] => return_unit (Object.datacopy p s n))
  ].

  Definition init_stack : Stack.t :=
    [
      {|
        Locals.functions := Stdlib.functions;
        Locals.variables := [];
      |}
    ].

  Definition initial_state : State.t :=
    {|
      State.stack := init_stack;
      State.memory := Memory.init;
      State.return_data := [];
      State.transient_storage := Memory.init;
      State.accounts := [];
      State.codes := [];
      State.logs := [];
      State.call_stack := [];
    |}.
End Stdlib.

(** We consider that all the primitives can be defined as a function over the state. *)
Definition eval_primitive {A : Set}
    (environment : Environment.t)
    (primitive : Primitive.t A) :
    State.t -> (A * State.t) + string :=
  fun state =>
  match primitive with
  | Primitive.OpenScope =>
    inl (
      tt,
      state <| State.stack := Stack.open_scope state.(State.stack) |>
    )
  | Primitive.CloseScope =>
    let stack := Stack.close_scope state.(State.stack) in
    match stack with
    | inr error => inr error
    | inl stack =>
      inl (
        tt,
        state <| State.stack := stack |>
      )
    end
  | Primitive.GetVar name =>
    let value := Stack.get_var state.(State.stack) name in
    match value with
    | inr error => inr error
    | inl value =>
      inl (
        value,
        state
      )
    end
  | Primitive.DeclareVars names values =>
    let stack := Stack.declare_vars state.(State.stack) names values in
    match stack with
    | inr error => inr error
    | inl stack =>
      inl (
        tt,
        state <| State.stack := stack |>
      )
    end
  | Primitive.AssignVars names values =>
    let stack := Stack.assign_vars state.(State.stack) names values in
    match stack with
    | inr error => inr error
    | inl stack =>
      inl (
        tt,
        state <| State.stack := stack |>
      )
    end
  | Primitive.MLoad address length =>
    inl (
      Memory.get_bytes state.(State.memory) address length,
      state
    )
  | Primitive.MStore address bytes =>
    inl (
      tt,
      state <| State.memory := Memory.update_bytes state.(State.memory) address bytes |>
    )
  | Primitive.SLoad slot =>
    let address := environment.(Environment.address) in
    let storage := Dict.get Z.eqb state.(State.accounts) address in
    match storage with
    | None => inr ("storage not found for the address " ++ HexString.of_Z address)%string
    | Some account =>
      inl (
        account.(Account.storage) slot,
        state
      )
    end
  | Primitive.SStore slot value =>
    let address := environment.(Environment.address) in
    let accounts :=
      Dict.assign_function Z.eqb state.(State.accounts) address (fun account =>
        account <| Account.storage := Storage.update account.(Account.storage) slot value |>
      ) in
    match accounts with
    | None => inr ("storage not found for the address " ++ HexString.of_Z address)%string
    | Some accounts =>
      inl (
        tt,
        state <| State.accounts := accounts |>
      )
    end
  | Primitive.RLoad =>
    inl (
      state.(State.return_data),
      state
    )
  | Primitive.TLoad address =>
    inl (
      state.(State.transient_storage) address,
      state
    )
  | Primitive.TStore address value =>
    inl (
      tt,
      state <| State.transient_storage :=
        Storage.update state.(State.transient_storage) address value
      |>
    )
  | Primitive.Log topics payload =>
    inl (
      tt,
      state <| State.logs := (topics, payload) :: state.(State.logs) |>
    )
  | Primitive.GetEnvironment =>
    inl (
      environment,
      state
    )
  | Primitive.GetNonce =>
    let address := environment.(Environment.address) in
    let accounts := state.(State.accounts) in
    match Dict.get Z.eqb accounts address with
    | None => inr ("nonce not found for the address " ++ HexString.of_Z address)%string
    | Some account =>
      inl (
        account.(Account.nonce),
        state
      )
    end
  | Primitive.GetCodedata address =>
    let accounts := state.(State.accounts) in
    match Dict.get Z.eqb accounts address with
    | None => inr ("codedata not found for the address " ++ HexString.of_Z address)%string
    | Some account =>
      inl (
        account.(Account.codedata),
        state
      )
    end
  | Primitive.CreateAccount address code codedata =>
    let account := {|
      Account.balance := 0;
      Account.nonce := 1;
      Account.code := code;
      Account.codedata := codedata;
      Account.storage := Memory.init;
      Account.immutables := [];
    |} in
    inl (
      tt,
      state <| State.accounts := Dict.declare state.(State.accounts) address account |>
    )
  | Primitive.UpdateCodeForDeploy address code =>
    let accounts := state.(State.accounts) in
    match Dict.assign_function Z.eqb accounts address (fun account =>
      account <| Account.code := code |>
    ) with
    | None => inr ("code not found for the address " ++ HexString.of_Z address)%string
    | Some accounts =>
      inl (
        tt,
        state <| State.accounts := accounts |>
      )
    end
  | Primitive.LoadImmutable name =>
    let address := environment.(Environment.address) in
    let accounts := state.(State.accounts) in
    match Dict.get Z.eqb accounts address with
    | None => inr ("immutables not found for the address " ++ HexString.of_Z address)%string
    | Some account =>
      match Dict.get Z.eqb account.(Account.immutables) name with
      | None => inr ("immutable not found for the name " ++ HexString.of_Z name)%string
      | Some value =>
        inl (
          value,
          state
        )
      end
    end
  | Primitive.SetImmutable name value =>
    let address := environment.(Environment.address) in
    let accounts := state.(State.accounts) in
    match Dict.assign_function Z.eqb accounts address (fun account =>
      account <| Account.immutables := Dict.declare account.(Account.immutables) name value |>
    ) with
    | None => inr ("immutables not found for the address " ++ HexString.of_Z address)%string
    | Some accounts =>
      inl (
        tt,
        state <| State.accounts := accounts |>
      )
    end
  | Primitive.CallStackPush name arguments =>
    inl (
      tt,
      state <| State.call_stack := (name, arguments) :: state.(State.call_stack) |>
    )
  | Primitive.CallStackPop =>
    match state.(State.call_stack) with
    | [] => inr "cannot pop the last element of the call stack"
    | _ :: call_stack =>
      inl (
        tt,
        state <| State.call_stack := call_stack |>
      )
    end
  end.

Definition state_error_bind {S E A B : Set} (e1 : S -> (A + E) * S) (e2 : A -> S -> (B + E) * S) :
    S -> (B + E) * S :=
  fun state =>
  match e1 state with
  | (inl a, state) => e2 a state
  | (inr error, state) => (inr error, state)
  end.

Notation "'letS?' x ':=' e 'in' body" :=
  (state_error_bind e (fun x => body))
  (at level 200, x ident, e at level 200, body at level 200).

(** We assume that there is engouh value in the contract we are transfering from. We also use this
    opportunity to increase the nonce of the account. *)
Definition decrease_value_of_current_contract
    (environemnt : Environment.t) (state : State.t) (transferred_value : U256.t) :
    State.t + string :=
  let address := environemnt.(Environment.address) in
  let accounts :=
    Dict.assign_function Z.eqb state.(State.accounts) address (fun account =>
      account
        <| Account.balance := account.(Account.balance) - transferred_value |>
        <| Account.nonce := account.(Account.nonce) + 1 |>
    ) in
  match accounts with
  | None => inr ("balance not found for the address " ++ HexString.of_Z address)%string
  | Some accounts =>
    inl (
      state <| State.accounts := accounts |>
    )
  end.

(** A function to evaluate an expression assuming that we have enough [fuel]. *)
Fixpoint eval {A : Set}
    (fuel : nat)
    (environment : Environment.t)
    (e : LowM.t A) :
    State.t -> (A + string) * State.t :=
  match fuel with
  | O => fun state => (inr "out of fuel", state)
  | S fuel =>
    match e with
    | LowM.Pure output => fun state => (inl output, state)
    | LowM.Primitive primitive k =>
      fun state =>
      let value_state := eval_primitive environment primitive state in
      match value_state with
      | inl (value, state) => eval fuel environment (k value) state
      | inr error => (inr error, state)
      end
    | LowM.DeclareFunction name body k =>
      fun state =>
      let stack := Stack.declare_function state.(State.stack) name body in
      match stack with
      | inr error => (inr error, state)
      | inl stack =>
        eval fuel environment k (state <| State.stack := stack |>)
      end
    | LowM.CallFunction name arguments k =>
      fun state =>
      let function := Stack.get_function state.(State.stack) name in
      (letS? results := eval fuel environment (function arguments) in
      eval fuel environment (k results)) state
    | LowM.Loop body break_with k =>
      letS? output := eval fuel environment body in
      match break_with output with
      | None => eval fuel environment (LowM.Loop body break_with k)
      | Some output => eval fuel environment (k output)
      end
    | LowM.CallContract address value input k => fun state =>
      if value >? environment.(Environment.callvalue) then
        (* When there is not enough balance, the call fails but we do not revert *)
        eval fuel environment (k 0) state
      else
        match decrease_value_of_current_contract environment state value with
        | inr error => (inr error, state)
        | inl state =>
          let callee_environment := {|
            Environment.caller := environment.(Environment.address);
            Environment.callvalue := value;
            Environment.calldata := input;
            Environment.address := address;
          |} in
          let callee_account := Dict.get Z.eqb state.(State.accounts) address in
          match callee_account with
          | None =>
            (* If the contract only contains code, we transfer the [value] and succeed immediately *)
            eval fuel environment (k 1) state
          | Some callee_account =>
            let callee_state :=
              Stdlib.initial_state
                <| State.accounts := state.(State.accounts) |>
                <| State.codes := state.(State.codes) |> in
            let callee_contract :=
              let code := callee_account.(Account.code) in
              match Dict.get Z.eqb state.(State.codes) code with
              | None => LowM.Impossible "code not found"
              | Some contract => contract
              end in
            let '(result, callee_state) :=
              eval fuel callee_environment callee_contract callee_state in
            match result with
            | inr error => (inr error, state)
            | inl (Result.Ok _) => (inr "call: expected a return or a revert", state)
            | inl (Result.Return p s) =>
              let state :=
                state
                  <| State.accounts := callee_state.(State.accounts) |>
                  <| State.return_data := Memory.get_bytes callee_state.(State.memory) p s |> in
              eval fuel environment (k 1) state
            | inl (Result.Revert p s) =>
              let state :=
                state
                  <| State.accounts := callee_state.(State.accounts) |>
                  <| State.return_data := Memory.get_bytes callee_state.(State.memory) p s |> in
              eval fuel environment (k 0) state
            end
          end
        end
    | LowM.Let e1 k =>
      letS? value := eval fuel environment e1 in
      eval fuel environment (k value)
    | LowM.Impossible message => fun state => (inr ("Impossible: " ++ message)%string, state)
    end
  end.

Module Run.
  Reserved Notation "{{ environment , state | e ⇓ output | state' }}"
    (at level 70, no associativity).

  Inductive t {A : Set} (environment : Environment.t) (state : State.t) (output : A) :
      LowM.t A -> State.t -> Prop :=
  | Pure : {{ environment, state | LowM.Pure output ⇓ output | state }}
  | Primitive {B : Set} (primitive : Primitive.t B) (k : B -> LowM.t A) value state_inter state' :
    inl (value, state_inter) = eval_primitive environment primitive state ->
    {{ environment, state_inter | k value ⇓ output | state' }} ->
    {{ environment, state | LowM.Primitive primitive k ⇓ output | state' }}
  | DeclareFunction name body k stack_inter state' :
    inl stack_inter = Stack.declare_function state.(State.stack) name body ->
    let state_inter := state <| State.stack := stack_inter |> in
    {{ environment, state_inter | k ⇓ output | state' }} ->
    {{ environment, state | LowM.DeclareFunction name body k ⇓ output | state' }}
  | CallFunction name arguments k results state_inter state' :
    let function := Stack.get_function state.(State.stack) name in
    {{ environment, state | function arguments ⇓ results | state_inter }} ->
    {{ environment, state_inter | k results ⇓ output | state' }} ->
    {{ environment, state | LowM.CallFunction name arguments k ⇓ output | state' }}
  | Let {B : Set} (e1 : LowM.t B) k state_inter output_inter state' :
    {{ environment, state | e1 ⇓ output_inter | state_inter }} ->
    {{ environment, state_inter | k output_inter ⇓ output | state' }} ->
    {{ environment, state | LowM.Let e1 k ⇓ output | state' }}

  where "{{ environment , state | e ⇓ output | state' }}" :=
    (t environment state output e state').
End Run.

Import Run.

(** The [eval] function follows the semantics given by [Run.t]. *)
(* Fixpoint eval_is_run {A : Set}
    (fuel : nat)
    (environment : Environment.t)
    (state : State.t)
    (e : LowM.t A)
    (output : A)
    (state' : State.t) :
  eval fuel environment state e = (inl output, state') ->
  {{ environment, state | e ⇓ output | state' }}.
Proof.
  destruct fuel as [|fuel]; [discriminate|].
  destruct e; cbn; intros H_eval.
  { (* Pure *)
    inversion H_eval; constructor.
  }
  { (* Primitive *)
    destruct primitive;
      cbn in H_eval;
      constructor;
      eapply eval_is_run;
      eassumption.
  }
  { (* DeclareFunction *)
    constructor.
    eapply eval_is_run.
    eassumption.
  }
  { (* CallFunction *)
    destruct eval as [[results | message] locals_inter] eqn:H_eval_inter in H_eval.
    { econstructor;
        eapply eval_is_run;
        eassumption.
    }
    { discriminate. }
  }
  { (* Let *)
    destruct eval as [[results | message] locals_inter] eqn:H_eval_inter in H_eval.
    { econstructor;
        eapply eval_is_run;
        eassumption.
    }
    { discriminate. }
  }
  { (* Impossible *)
    discriminate.
  }
Qed. *)

Definition eval_with_revert
    (fuel : nat)
    (environment : Environment.t)
    (e : M.t BlockUnit.t)
    (state : State.t) :
    (Result.t BlockUnit.t + string) * State.t :=
  let '(output, state') := eval fuel environment e state in
  match output with
  | inl (Result.Revert _ _) => (output, state' <| State.accounts := state.(State.accounts) |>)
  | _ => (output, state')
  end.

Definition update_current_code_for_deploy (hex_name : Z) : M.t BlockUnit.t :=
  let* environment := LowM.Primitive Primitive.GetEnvironment M.pure in
  let address := environment.(Environment.address) in
  let* tt := LowM.Primitive (Primitive.UpdateCodeForDeploy address hex_name) M.pure in
  M.pure BlockUnit.Tt.

(** We design the testing primitives so that, in case of error, we can see what was wrong. *)
Module Test.
  (** Supposed to be equal to [inl]. *)
  Definition is_return (result : Result.t BlockUnit.t + string) (state : State.t) :
      list Z + (Result.t BlockUnit.t + string) :=
    match result with
    | inl (Result.Return start length) =>
      let output := Memory.get_bytes state.(State.memory) start length in
      inl output
    | _ => inr result
    end.

  Module Status.
    Inductive t : Set :=
    | Success
    | Failure
    | OutOfGas.
  End Status.

  (** Supposed to be equal to [inl]. *)
  Definition extract_output
      (result : Result.t BlockUnit.t + string)
      (state : State.t)
      (status : Status.t) :
      list Z + (Result.t BlockUnit.t + string) :=
    match status with
    | Status.Success =>
      match result with
      | inl (Result.Return start length) =>
        let output := Memory.get_bytes state.(State.memory) start length in
        inl output
      | _ => inr result
      end
    | Status.Failure =>
      match result with
      | inl (Result.Revert start length) =>
        let output := Memory.get_bytes state.(State.memory) start length in
        inl output
      | _ => inr result
      end
    | Status.OutOfGas =>
      (* We rely on the fact that most tests that end with an "out of gas" error also make
         an "out of fuel" error, even if these two measures of the execution steps are different. *)
      match result with
      | inr "out of fuel" => inl []
      | _ => inr result
      end
    end.
End Test.

Definition declared_vars (state : State.t) : list (list (string * U256.t)) :=
  List.map (fun locals => locals.(Locals.variables)) state.(State.stack).
