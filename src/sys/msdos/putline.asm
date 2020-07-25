; High speed screen update for Rainbow.
; putline(row, col, buf);
; Row and column are origin 1.
; The buf is a pointer to an array of characters.
; It won't write past the end of the line in
; the video RAM; it looks for the FF at the end
; of the line.
; This routine by Bob McNamara.
; put into large model .ASM format by Rich Ellison

putline_code	segment byte	public	'code'
extrn	tthue_:word
	public	putline_
	assume	cs:putline_code,ds:nothing,es:nothing,ss:nothing

ScrSeg  equ	0ee00h
ScrPtr	equ	3
CMODE	equ	2

putline_	proc	far	; putline(Row, Col, Buf)	/* fast video */
Row	equ	10		; int	Row;			/*   row addr */
Col	equ	12		; int	Col;			/*   col addr */
Buf	equ	14		; char	*Buf;			/*   data     */
				; {
	push	si
	push	di
	push	bp
	mov	bp,sp
	push	ds
	push	es
	mov	ax,ScrSeg		;point extra segment into screen RAM
	mov	es,ax
	mov	di,es:ScrPtr+1		;di <- base line address
	and	di,0fffh
	mov	al,0ffh
	cld

	mov	dx,[bp+Row]		;row number to write (dx)
	lds	si,dword ptr[bp+Buf]	;string to be moved ds:(si)
	mov	bx,[bp+Col]		;column number to start at (bx)
	dec	bx			;column number starts at 1
	dec	dx			;row number starts at 1
	jz	l2
l1:	mov	cx,140
	repnz scasb
	mov	di,es:[di]		;pointer to next line (di)
	and	di,0fffh
	dec	dx
	jnz	l1
l2:	add	di,bx			;di -> offset in row
	push	di

l3:	cmp	al,es:[di]
	jz	l4
	movsb
	cmp	al,es:[di]
	jz	l4
	movsb
	cmp	al,es:[di]
	jz	l4
	movsb
	cmp	al,es:[di]
	jz	l4
	movsb
	cmp	al,es:[di]
	jz	l4
	movsb
	cmp	al,es:[di]
	jz	l4
	movsb
	cmp	al,es:[di]
	jz	l4
	movsb
	cmp	al,es:[di]
	jz	l4
	movsb
	jmp	l3
l4:
	pop	cx
	mov	ax,cx
	sub	di,cx
	mov	cx,di			;cx contains repeat count
	add	ax,1000h		;point into attribute portion of RAM
	mov	di,ax			;di contains pointer into attr. RAM
	mov	ax,0fh			;assume rev. video
	cmp	word ptr tthue_,CMODE	;is this for mode line?
	jz	l5			;yes
	mov	ax,0eh			;no, clear attributes
l5:
	rep stosb

	pop	es
	pop	ds
	pop	bp
	pop	di
	pop	si
	ret
putline_	endp

	putline_code	ends
	end
