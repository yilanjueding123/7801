 	AREA XXSCopy_180, CODE, READONLY
	
	EXPORT Display_Data_Conv
	EXPORT Display_Data_Conv_180
	
	
	
	
	
	
Display_Data_Conv
	STMDA r13!,{r4-r11}
	MOV r6,r0
	ADD r7,r0,r2
data_conv_frame	
	ADD r5,r3,r1
data_conv_line
	LDRH r4,[r0],r2
	LDRH r8,[r0],r2
	ORR r9,r4,r8,LSL #16
	LDRH r4,[r0],r2
	LDRH r8,[r0],r2
	ORR r10,r4,r8,LSL #16
	LDRH r4,[r0],r2
	LDRH r8,[r0],r2
	ORR r11,r4,r8,LSL #16
	LDRH r4,[r0],r2
	LDRH r8,[r0],r2
	ORR r12,r4,r8,LSL #16
	
	STMIA r1,{r9-r12}
	ADD r1,r1,#16
	CMP r1,r5
	BNE data_conv_line
	ADD r6,r6,#2
	MOV r0,r6
	CMP r6,r7
	BNE data_conv_frame
	LDMIB r13!,{r4-r11}
	NOP
	NOP
	NOP
	BX r14



	
Display_Data_Conv_180
	STMDA r13!,{r4-r11}
	MOV r6,r0
	ADD r7,r0,r2
	MOV R5,R3,LSR #1
	MUL r5,r2,r5
	ADD r1,r5,r1
	MOV r5,r1
data_conv_frame_180	
	SUB r5,r5,r3
data_conv_line_180
	LDRH r8,[r0],r2
	LDRH r4,[r0],r2
	ORR r12,r4,r8,LSL #16
	LDRH r8,[r0],r2
	LDRH r4,[r0],r2
	ORR r11,r4,r8,LSL #16
	LDRH r8,[r0],r2
	LDRH r4,[r0],r2
	ORR r10,r4,r8,LSL #16
	LDRH r8,[r0],r2
	LDRH r4,[r0],r2
	ORR r9,r4,r8,LSL #16
	
	STMDB r1,{r9-r12}
	SUB r1,r1,#16
	CMP r1,r5
	BNE data_conv_line_180
	ADD r6,r6,#2
	MOV r0,r6
	CMP r6,r7
	BNE data_conv_frame_180
	LDMIB r13!,{r4-r11}
	NOP
	NOP
	NOP
	BX r14
	
	
 	END
 


