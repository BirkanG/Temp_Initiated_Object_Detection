    AREA    |.text|, CODE, READONLY
    EXPORT  a_delay

a_delay
    SUBS    R0, R0, #1   ; Decrement ulCount
    BNE     a_delay        ; If ulCount != 0, branch back to Delay
    BX      LR           ; Return to caller
    END
