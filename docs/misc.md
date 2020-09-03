debugging optimizer

    $ ./iroha a.iroha -d /tmp/a -opt clean



* Folliowing code was from Sythesijer. We used it to prototype iroha.

(MODULE Test001 % クラス名
  (VARIABLES % メンバ変数
    (CONSTANT INT constant_00000 0)
    (VAR INT x :public false :member true :init (REF CONSTANT constant_00000))
    ...
  )
 (BOARD INT acc % メソッドに相当するスケジュール表
  (VARIABLES  % ローカル変数
    ...
  )
    (SEQUENCER acc % スケジュール表の本体
      (SLOT 0 % 状態，各状態に複数のアイテムをいれることが可能
        (METHOD_EXIT :next (1)) % アイテム :nextの後のリストで次状態(分岐なしでも)
      )
      (SLOT 1
        (METHOD_ENTRY :next (2))
      )
      (SLOT 2
        (SET binary_expr_00003 (ADD x acc_y_0002) :next (3)) % この時点では演算子のバインドはしてない
      )
      (SLOT 3
        (SET x (ASSIGN binary_expr_00003) :next (4))
      )
      (SLOT 4
        (RETURN x :next (0))
      )
      (SLOT 5
        (JP :next (0))
      )
...
      (SLOT 100
        (JT binary_expr_00015 :next (9 5)) % 条件分岐の場合:nextの先に複数持つ
      )
…
    )
  )
...
)
