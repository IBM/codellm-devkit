# Source file used to test the jal macro with -KPIC code.
	
text_label:	
	.set	noreorder
	.cpload	$25
	.set	reorder
	.cprestore	0
	jal	$25
	jal	$4,$25
	jal	text_label
	jal	external_text_label
	
# Test j as well	
	j	text_label
