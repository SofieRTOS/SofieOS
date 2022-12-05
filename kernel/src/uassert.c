void uassert(int c) {
	sofie_set_protected_stack();
	if (!c) {
		uprintf("ASSERT!");
		while(1);
	}
	sofie_restore_protected_stack();
}

