import           Data.Bits

data UInst
  = IncPc
  | JmpEn
  | NextInst
  | Halt
  | WrA
  | WrB
  | WrC
  | WrMar
  | WrDmem
  | RdA
  | RdB
  | RdC
  | RdTmp
  | RdDmem
  | RdPmem
  | AluEn
  | AluDisableA
  | AluCin
  | AluInvB
  | AluSelAdder
  | AluS0
  | AluS1
  | AluShr
  | DontUpdateFlags
  deriving (Eq, Ord, Enum)

activeLow :: [UInst]
activeLow =
  [ JmpEn,
    NextInst,
    Halt,
    WrA,
    WrB,
    WrC,
    WrMar,
    WrDmem,
    RdA,
    RdB,
    RdC,
    RdTmp,
    RdDmem,
    RdPmem,
    AluEn,
    AluDisableA,
    DontUpdateFlags
  ]

slice :: Int -> Int -> Int -> Int
slice val high low =
  let width = high - low + 1
      mask = shiftL 1 width - 1
   in (.&.) (shiftR val low) mask

match :: Int -> String -> Bool
match val pattern = go 0 (reverse pattern)
  where
    go _ "" = True
    go i (c : cs)
      | c == '0' =
          ((.&.) (shiftR val i) 1 /= 1)
            && go (i + 1) cs
      | c == '1' =
          ((.&.) (shiftR val i) 1 /= 0)
            && go (i + 1) cs
      | otherwise = go (i + 1) cs


ucode :: Int -> Int -> Int -> Maybe [UInst]
ucode cycle flags inst
  -- range safety
  | not $ 0 <= cycle && cycle < 4 &&
          0 <= flags && flags < 8 &&
          0 <= inst && inst < 256
  = Nothing

  -- fetch
  | cycle == 0
  = Just [IncPc]

  -- mov d, s
  | match inst "000__0__"
  , dst /= 3
  = Just $
    case cycle of
      1 -> wr src ++ rd dst ++ [NextInst]

  -- mov d, [s]
  | match inst "000__1__"
  , dst /= 3
  = Just $
    case cycle of
      1 -> WrMar : rd src
      2 -> wr src ++ [RdDmem, NextInst]

  -- mov [d], s
  | match inst "001__0__"
  = Just $
    case cycle of
      1 -> WrMar : rd dst
      2 -> [WrDmem] ++ rd src ++ [NextInst]

  -- xchg d, s
  | match inst "010__0__"
  , dst /= 3
  , src /= 3
  = Just $
    case cycle of
      1 -> [AluEn, AluDisableA, AluSelAdder, DontUpdateFlags] ++ rd dst
      2 -> wr dst ++ rd src
      3 -> wr src ++ [RdTmp, NextInst]

  -- jcc dest
  | match inst "01100___"
  = Just $
    case cycle of
      1 -> [JmpEn | condOk] ++ [IncPc, NextInst]

  -- jmp dest
  | match inst "011010__"
  = Just $
    case cycle of
      1 -> [JmpEn, IncPc, NextInst]

  -- djnz s, dest
  | match inst "011011__"
  , src `elem` [0, 1, 2]
  = case cycle of
      1 -> [AluEn, AluSelAdder, AluInvB]

  = case (src, cycle) of
        (0, 1) -> [AluEn, AluSelAdder, DontUpdateFlags, RdA, AluCin]
        (1, 1) -> [AluEn, AluSelAdder, DontUpdateFlags, RdB, AluCin]
        (2, 1) -> [AluEn, AluSelAdder, DontUpdateFlags, RdC, AluCin]

        (0, 2) -> [WrA, RdTmp]
        (1, 2) -> [WrB, RdTmp]
        (2, 2) -> [WrC, RdTmp]

        (_, 3) -> if match flags "__0"
                  then [JmpEn, NextInst]
                  else [NextInst]

  -- halt
  | match inst "011100__" = [Halt, NextInst]

  -- nop2, nop3, nop4
  | match inst "0111____" =
      case (slice inst 3 2, cycle) of
        (1, 1) -> [NextInst]

        (2, 1) -> []
        (2, 2) -> [NextInst]

        (3, 1) -> []
        (3, 2) -> []
        (3, 3) -> [NextInst]

  -- add s
  | match inst "10000___" =
      if aluSrc `elem` [0, 1, 2, 3, 4]
      then
        case (aluSrc, cycle) of
          (0, 1) -> [AluEn, AluSelAdder, RdA]
          (1, 1) -> [AluEn, AluSelAdder, RdB]
          (2, 1) -> [AluEn, AluSelAdder, RdC]
          (3, 1) -> [AluEn, AluSelAdder, RdPmem, IncPc]
          (4, 1) -> [AluEn, AluSelAdder]

          (_, 2) -> [WrA, RdTmp, NextInst]
      else
        case (aluSrc, cycle) of
          (5, 1) -> [WrMar, RdB]
          (6, 1) -> [WrMar, RdC]
          (7, 1) -> [WrMar, RdPmem, IncPc]

          (_, 2) -> [AluEn, AluSelAdder, RdDmem]

          (_, 3) -> [WrA, RdTmp, NextInst]

  -- adc s
  | match inst "10001___" =
    -- if z, add AluCin
      if aluSrc `elem` [0, 1, 2, 3, 4]
      then
        case (aluSrc, cycle) of
          (0, 1) -> [AluEn, AluSelAdder, RdA]
          (1, 1) -> [AluEn, AluSelAdder, RdB]
          (2, 1) -> [AluEn, AluSelAdder, RdC]
          (3, 1) -> [AluEn, AluSelAdder, RdPmem, IncPc]
          (4, 1) -> [AluEn, AluSelAdder]

          (_, 2) -> [WrA, RdTmp, NextInst]
      else
        case (aluSrc, cycle) of
          (5, 1) -> [WrMar, RdB]
          (6, 1) -> [WrMar, RdC]
          (7, 1) -> [WrMar, RdPmem, IncPc]

          (_, 2) -> [AluEn, AluSelAdder, RdDmem]

          (_, 3) -> [WrA, RdTmp, NextInst]



  | otherwise = []

  where
    match = match

    src = slice inst 1 0
    dst = slice inst 4 3

    wr r = case r of
      0 -> [WrA]
      1 -> [WrB]
      2 -> [WrC]
      _ -> []

    rd r = case r of
      0 -> [RdA]
      1 -> [RdB]
      2 -> [RdC]
      _ -> [RdPmem, IncPc]

    cond = slice inst 2 0
    z = testBit flags 0
    n = testBit flags 1
    c = testBit flags 2
    condOk = case cond of
      0 -> z                -- jz, je
      1 -> n                -- jn, jl
      2 -> c                -- jc
      3 -> z || n           -- jle
      4 -> not z            -- jnz, jne
      5 -> not n            -- jnn, jge
      6 -> not c            -- jnc
      7 -> not z && not n   -- jg

    aluSrc = slice inst 2 0

uCodes :: [Int]
uCodes = [0..8192]
