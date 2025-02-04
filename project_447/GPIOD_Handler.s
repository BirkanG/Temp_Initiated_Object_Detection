    ; GPIOD_Handler.s
    ; Handles interrupts from GPIO Port D (PD0-PD3) using Thumb instruction set
    ; TM4C123 Microcontroller

    ; Define base addresses and offsets
GPIOD_BASE      EQU 0x40007000  ; GPIO Port D base address
GPIOD_MIS       EQU 0x418       ; Masked interrupt status register offset
GPIOD_DATA      EQU 0x3FC       ; Data register offset
GPIOD_ICR       EQU 0x41C       ; Interrupt clear register offset

    AREA    GPIOD_Handler_Code, CODE, READONLY
    THUMB

    ; Export the handler and declare external symbols
    EXPORT GPIOD_Handler
    EXTERN ADC1_Read
    EXTERN digital_temp_thr
    EXTERN min_digital_temp_thr
    EXTERN page
    EXTERN analog_temp_thr_adc_value

    ; Handler starts here
GPIOD_Handler
    PUSH    {R0-R7, LR}        ; Save registers

    ; Debounce delay
    LDR     R0, =500000        ; Set delay count
DebounceLoop
    SUBS    R0, R0, #1         ; Decrement counter
    BNE     DebounceLoop       ; Wait until counter reaches 0

    ; Load base address of GPIOD
    LDR     R1, =GPIOD_BASE

    ; Check if PD0 triggered
    LDR     R2, [R1, #GPIOD_MIS] ; Load GPIOD->MIS
    TST     R2, #1             ; Check bit 0
    BEQ     Check_PD1          ; Skip if not set

    ; Handle PD0 interrupt
    LDR     R3, [R1, #GPIOD_DATA] ; Load GPIOD->DATA
    TST     R3, #1             ; Check PD0 state
    BNE     Clear_PD0          ; Skip if input is high

    ; Decrease digital_temp_thr
    LDR     R4, =digital_temp_thr
    LDR     R5, [R4]           ; Load digital_temp_thr
    LDR     R6, =min_digital_temp_thr
    LDR     R7, [R6]           ; Load min_digital_temp_thr
    CMP     R5, R7             ; Compare thresholds
    BLE     Clear_PD0          ; Skip if not greater
    SUBS    R5, R5, #1         ; Decrease threshold
    STR     R5, [R4]           ; Store updated threshold

Clear_PD0
    MOVS    R3, #1             ; Clear PD0 interrupt
    STR     R3, [R1, #GPIOD_ICR]

Check_PD1
    ; Check if PD1 triggered
    LDR     R2, [R1, #GPIOD_MIS]
    TST     R2, #2             ; Check bit 1
    BEQ     Check_PD2          ; Skip if not set

    ; Handle PD1 interrupt
    LDR     R3, [R1, #GPIOD_DATA]
    TST     R3, #2             ; Check PD1 state
    BNE     Clear_PD1          ; Skip if input is high

    ; Increase digital_temp_thr
    LDR     R4, =digital_temp_thr
    LDR     R5, [R4]
    LDR     R6, =min_digital_temp_thr
    LDR     R7, [R6]
    CMP     R5, R7
    BLT     Clear_PD1          ; Skip if less than
    ADDS    R5, R5, #1         ; Increase threshold
    STR     R5, [R4]

Clear_PD1
    MOVS    R3, #2             ; Clear PD1 interrupt
    STR     R3, [R1, #GPIOD_ICR]

Check_PD2
    ; Check if PD2 triggered
    LDR     R2, [R1, #GPIOD_MIS]
    TST     R2, #4             ; Check bit 2
    BEQ     Check_PD3          ; Skip if not set

    ; Debouncing delay
Debounce_Loop
    LDR     R3, =500000         ; Load delay count value
Debounce_Wait
    SUBS    R3, R3, #1         ; Decrement delay counter
    BNE     Debounce_Wait      ; Wait until counter reaches 0

    ; Re-check button state to confirm stable press
    LDR     R2, [R1, #GPIOD_DATA] ; Read GPIO data
    TST     R2, #4             ; Check if PD2 is still pressed
    BEQ     Clear_PD2          ; If not stable, skip the action

    ; Increment page modulo 100
    LDR     R4, =page          ; Load address of page
    LDR     R5, [R4]           ; Load current page value
    ADDS    R5, R5, #1         ; Increment page
;    MOVS    R6, #100           ; Load divisor (100)
;    UDIV    R7, R5, R6         ; Compute quotient (page / 100)
;    MUL     R7, R7, R6         ; Multiply quotient by 100
;    SUBS    R5, R5, R7         ; Compute remainder (page % 100)
    STR     R5, [R4]           ; Store updated page value

Clear_PD2
    ; Clear PD2 interrupt
    MOVS    R3, #4
    STR     R3, [R1, #GPIOD_ICR]

Check_PD3
    ; Check if PD3 triggered
    LDR     R2, [R1, #GPIOD_MIS]
    TST     R2, #8             ; Check bit 3
    BEQ     End_Handler        ; Skip if not set

    ; Clear PD3 interrupt
    MOVS    R3, #8
    STR     R3, [R1, #GPIOD_ICR]

    ; Refresh analog_temp_thr
    BL      ADC1_Read          ; Call ADC read function
	NOP
	NOP
	NOP
	NOP
    MOV     R4, R0             ; Move ADC result to R4
    LDR     R5, =analog_temp_thr_adc_value
    STR     R4, [R5]           ; Store updated analog_temp_thr value

;    ; Clear PD3 interrupt
;    MOVS    R3, #8
;    STR     R3, [R1, #GPIOD_ICR]

End_Handler
    POP     {R0-R7, PC}        ; Restore registers and return

    ALIGN
    END
